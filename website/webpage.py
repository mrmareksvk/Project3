from flask import Flask, render_template
import psycopg2
import psycopg2.sql
import datetime
import pytz

app = Flask(__name__)


def readDB():
    con = psycopg2.connect(
        host="127.0.0.1",
        port="5432",
        database="test",
        user="test",
        password="test"
    )
    cur = con.cursor()
    cur.execute("SELECT table_name FROM information_schema.tables WHERE table_schema='public'")

    tables = cur.fetchall()

    latest_values = []

    for table in tables:
        cur.execute(
            psycopg2.sql.SQL("SELECT * FROM {} ORDER BY dt DESC LIMIT 1").format(psycopg2.sql.Identifier(table[0]))
        )
        values = cur.fetchone()

        status = ""

        if datetime.datetime.now(datetime.timezone.utc) < pytz.utc.localize(values[6]) + datetime.timedelta(minutes=20):
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
                "lon": str(values[5])
            }
        )
    con.close()
    cur.close()
    return latest_values


@app.route("/")
def homePage():
    data_a = readDB()
    return render_template("index.html", data_a=data_a)


@app.route("/sensor<sensorID>")
def sensorPage(sensorID=None):
    data_a = readDB()
    for data in data_a:
        if data['name'] == 'sensor%s' % (sensorID):
            data_display = data
    return render_template("sensor.html", data=data_display)


if __name__ == "__main__":
    app.debug = True
    app.run(port=5000)


# Sikrit stahs DNT TCH
'''
data_a = [
    {
        "name": "sensor1",
        "status": "ONLINE",
        "temp": "20.2",
        "hum": "40%",
        "light": "970",
        "datetime": "11.5.2020 12:25",
        "lat": "56.11988",
        "lon": "10.15921",
    },
    {
        "name": "sensor2",
        "status": "OFFLINE",
        "temp": "19",
        "hum": "35%",
        "light": "785",
        "datetime": "11.5.2020 12:35",
        "lat": "56.11919",
        "lon": "10.15833",
    },
]
'''
