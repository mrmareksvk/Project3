from flask import Flask, render_template
import psycopg2
import psycopg2.sql
import datetime
import pytz
import json
from meteocalc import Temp, heat_index


app = Flask(__name__)


def DBconnector():  # <------------------- CHANGE ZIS
    return psycopg2.connect(
        host="127.0.0.1",
        port="5432",
        database="test",
        user="postgres",
        password="test",
    )


def readDB():
    con = DBconnector()
    cur = con.cursor()
    cur.execute(
        "SELECT table_name FROM information_schema.tables WHERE table_schema='public'"
    )

    tables = cur.fetchall()

    latest_values = []

    for table in tables:
        cur.execute(
            psycopg2.sql.SQL("SELECT * FROM {} ORDER BY dt DESC LIMIT 1").format(
                psycopg2.sql.Identifier(table[0])
            )
        )
        values = cur.fetchone()

        status = ""

        if datetime.datetime.now(datetime.timezone.utc) < pytz.utc.localize(
            values[6]
        ) + datetime.timedelta(minutes=20):
            status = "ONLINE"
        else:
            status = "OFFLINE"

        latest_values.append(
            {
                "name": table[0],
                "status": status,
                "temp": str(values[1]),
                "hum": str(values[2]),
                "light": str(values[3]),
                "datetime": values[6].strftime("%d.%m.%Y %H:%M"),
                "lat": str(values[4]),
                "lon": str(values[5]),
            }
        )
    con.close()
    cur.close()
    return latest_values


def data_graph(tablename, limit):
    con = DBconnector()
    cur = con.cursor()
    cur.execute(psycopg2.sql.SQL("SELECT temperature, humidity, lux, dt FROM {} ORDER BY dt DESC LIMIT %s").format(
        psycopg2.sql.Identifier(tablename)), (limit,))
    dates = []
    temperatures = []
    humidities = []
    lights = []

    for row in cur.fetchall():
        temperatures.append(row[0])
        humidities.append(row[1])
        lights.append(row[2])
        dates.append(row[3])

    con.close()
    cur.close()

    return dates, temperatures, humidities, lights


@app.route("/")
def homePage():
    data_a = readDB()
    return render_template("index.html", data_a=data_a)


@app.route("/sensor<sensorID>")
@app.route("/sensor<sensorID>/<graph>")
def sensorPage(sensorID=None, graph=None):
    tablename = 'sensor%s' % (sensorID)
    data_a = readDB()
    for data in data_a:
        if data['name'] == tablename:
            data_display = data
            t = Temp(data['temp'], 'c')
            hi = float(heat_index(temperature=t, humidity=int(data['hum'])).c)

    con = DBconnector()
    cur = con.cursor()

    cur.execute(
        psycopg2.sql.SQL("SELECT * FROM {}").format(
            psycopg2.sql.Identifier(tablename)
        )
    )

    archive = cur.fetchall()
    archive_list = list(map(list, archive))

    cur.close()
    con.close()

    for row in archive_list:
        row[6] = row[6].strftime("%d.%m.%Y %H:%M") + "\n"
        row[6].replace(',', '')

    # empty list added to beginning in order to fix CSV spacing issue
    archive_list.insert(0, [])

    filename = 'sensor%s' % (sensorID) + "_" + str(datetime.datetime.now(datetime.timezone.utc))

    datestamps = []
    if graph is not None:
        if '10' in graph:
            dates, temperatures, humidities, lights = data_graph(tablename, 10)
            for date in dates:
                datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
        elif '20' in graph:
            dates, temperatures, humidities, lights = data_graph(tablename, 20)
            for date in dates:
                datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
        elif '30' in graph:
            dates, temperatures, humidities, lights = data_graph(tablename, 30)
            for date in dates:
                datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
        return render_template("sensor.html", hi=hi, data=data_display, dates=json.dumps(datestamps), temperatures=temperatures, humidities=humidities, lights=lights, archive=json.dumps(archive_list), download=filename, option=graph)
    else:
        return render_template("sensor.html", hi=hi, data=data_display, archive=json.dumps(archive_list), download=filename)


if __name__ == "__main__":
    app.debug = True
    app.run(port=5000)
