from flask import Flask, render_template
import psycopg2
import psycopg2.sql
import datetime
import pytz
import json


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


def data_graph(data, tablename, limit):
    con = DBconnector()
    cur = con.cursor()
    if data == 'temperature, humidity':
        split_data = data.split(", ")
        cur.execute(psycopg2.sql.SQL("SELECT {}, {}, dt FROM {} ORDER BY dt DESC LIMIT %s").format(
            psycopg2.sql.Identifier(split_data[0]), psycopg2.sql.Identifier(split_data[1]), psycopg2.sql.Identifier(tablename)), (limit,))
    else:
        cur.execute(psycopg2.sql.SQL("SELECT {}, dt FROM {} ORDER BY dt DESC LIMIT %s").format(
            psycopg2.sql.Identifier(data), psycopg2.sql.Identifier(tablename)), (limit,))
    dates = []
    temperatures = []
    humidities = []
    lights = []

    for row in cur.fetchall():
        if data == 'temperature, humidity':
            temperatures.append(row[0])
            humidities.append(row[1])
            dates.append(row[2])
        else:
            dates.append(row[1])
            if data == 'temperature':
                temperatures.append(row[0])
            elif data == 'humidity':
                humidities.append(row[0])
            else:
                lights.append(row[0])
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

    datestamps = []
    if graph is not None:
        if '10' in graph:
            if 'light' in graph:
                dates, temperatures, humidities, lights = data_graph('lux', tablename, 10)
                for date in dates:
                    datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
                text = 'Light intensity (last 10 records)'
            elif 'temp&hum' in graph:
                dates, temperatures, humidities, lights = data_graph('temperature, humidity', tablename, 10)
                for date in dates:
                    datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
                text = 'Temperature&Humidity (last 10 records)'
        elif '20' in graph:
            if 'light' in graph:
                dates, temperatures, humidities, lights = data_graph('lux', tablename, 20)
                for date in dates:
                    datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
                text = 'Light intensity (last 10 records)'
            elif 'temp&hum' in graph:
                dates, temperatures, humidities, lights = data_graph('temperature, humidity', tablename, 20)
                for date in dates:
                    datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
                text = 'Temperature&Humidity (last 20 records)'
        elif '30' in graph:
            if 'light' in graph:
                dates, temperatures, humidities, lights = data_graph('lux', tablename, 30)
                for date in dates:
                    datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
                text = 'Light intensity (last 30 records)'
            elif 'temp&hum' in graph:
                dates, temperatures, humidities, lights = data_graph('temperature, humidity', tablename, 30)
                for date in dates:
                    datestamps.append(date.strftime("%d-%m-%Y %H:%M"))
                text = 'Temperature&Humidity (last 30 records)'
        return render_template("sensor.html", data=data_display, dates=json.dumps(datestamps), temperatures=temperatures, humidities=humidities, lights=lights, text=text)
    else:
        return render_template("sensor.html", data=data_display)


if __name__ == "__main__":
    app.debug = True
    app.run(port=5000)
