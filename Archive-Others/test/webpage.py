from flask import Flask, render_template, request
import psycopg2

app = Flask(__name__)
key_prev = 0

def dataMap():
    for key in keys:
        data[key] = values[len(data)]


def mapData():
    con = psycopg2.connect(
        host = "localhost",
        database = "first_db",
        user = "postgres",
        password = "0000")

    cur = con.cursor()
    cur.execute("select * from sensortest ORDER BY dt DESC LIMIT 1")

    result = cur.fetchall()

    cur.close()
    con.close()

    for r in result:
        print(r)
        key_new = r[0]

        global key_prev

        if key_prev == key_new:
            status = "OFFLINE"
        else:
            status = "ONLINE"

        temp = r[1]
        humid = r[2]
        light = r[3]
        lat = r[4]
        lng = r[5]
        time = r[6]

    data_a = [
        {
            "name": "sensor1",
            "status": status,
            "temp": temp,
            "hum": humid,
            "light": light,
            "datetime": time,
            "lat": lat,
            "lon": lng,
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
    key_prev = key_new
    return data_a


@app.route("/")
def homePage():
    data_a = mapData()
    return render_template("index.html", data_a=data_a)


#@app.route("/sensor")
#def sensorPage():
#    return render_template("sensor.html")


if __name__ == "__main__":
    app.debug = True
    app.run(port=5000)