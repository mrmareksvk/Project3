from flask import Flask, render_template
import psycopg2
import psycopg2.sql
import datetime
import pytz
from matplotlib.figure import Figure


app = Flask(__name__)


def DBconnector():  # <------------------- CHANGE ZIS
    return psycopg2.connect(
        host="127.0.0.1", port="5432", database="test", user="test", password="test"
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
    cur.execute(
        psycopg2.sql.SQL("SELECT {}, dt FROM {} ORDER BY dt DESC LIMIT %s").format(
            psycopg2.sql.Identifier(data), psycopg2.sql.Identifier(tablename)
        ),
        (limit,),
    )
    dates = []
    temperatures = []
    humidities = []
    lights = []
    path = "./static/"

    for row in cur.fetchall():
        if data == "temperature, humidity":
            temperatures.append(row[0])
            humidities.append(row[1])
            dates.append(row[2])
        else:
            dates.append(row[1])
            if data == "temperature":
                temperatures.append(row[0])
            elif data == "humidity":
                humidities.append(row[0])
            else:
                lights.append(row[0])
    fig = Figure()
    ax = fig.subplots()
    if data == "temperature":
        ax.plot(dates, temperatures)
        filename = path + "temperature{}{}.png".format(limit, tablename)
        print(filename)
        fig.savefig(filename)
    elif data == "humidity":
        ax.plot(dates, humidities, "-")
        filename = path + "humidity{}{}.png".format(limit, tablename)
        fig.savefig(filename)
    elif data == "lux":
        ax.plot(dates, lights, "-")
        filename = path + "light{}{}.png".format(limit, tablename)
        fig.savefig(filename)
    elif data == "temperature, humidity":
        filename = path + "temperature,humidity{}{}.png".format(limit, tablename)
        ax.plot(dates, temperatures, humidities, "-")
        fig.savefig(filename)
    fig.clear()
    cur.close()
    con.close()
    return filename


@app.route("/")
def homePage():
    data_a = readDB()
    return render_template("index.html", data_a=data_a)


@app.route("/sensor<sensorID>")
@app.route("/sensor<sensorID>/<something>")
def sensorPage(sensorID=None, something=None):
    tablename = 'sensor%s' % (sensorID)
    data_a = readDB()
    for data in data_a:
        if data['name'] == tablename:
            data_display = data

    if something is not None:
        if '10' in something:
            if 'temp' in something:
                filename = data_graph('temperature', tablename, 10)
            elif 'hum' in something:
                filename = data_graph('humidity', tablename, 10)
            elif 'light' in something:
                filename = data_graph('lux', tablename, 10)
            elif 'temp&hum' in something:
                filename = data_graph('temperature, humidity', tablename, 10)
        elif '20' in something:
            if 'temp' in something:
                filename = data_graph('temperature', tablename, 20)
            elif 'hum' in something:
                filename = data_graph('humidity', tablename, 20)
            elif 'light' in something:
                filename = data_graph('lux', tablename, 20)
            elif 'temp&hum' in something:
                filename = data_graph('temperature, humidity', tablename, 20)
        elif '30' in something:
            if 'temp' in something:
                filename = data_graph('temperature', tablename, 30)
            elif 'hum' in something:
                filename = data_graph('humidity', tablename, 30)
            elif 'light' in something:
                filename = data_graph('lux', tablename, 30)
            elif 'temp&hum' in something:
                filename = data_graph('temperature, humidity', tablename, 30)
        return render_template("sensor.html", data=data_display, filename=filename)
    else:
        return render_template("sensor.html", data=data_display)


if __name__ == "__main__":
    app.debug = True
    app.run(port=5000)
