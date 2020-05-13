from flask import Flask, render_template, request
import psycopg2

app = Flask(__name__)


# def data_graph():
#     engine.execute('select temperature, humidity, dt from sensor1')
#     dates = []
#     temperatures = []
#     humidities = []
#
#     for row in c.fetchall():
#         dates.append(row[2])
#         temperatures.append(row[0])
#         humidities.append(row[1])
#     plt.plot(dates, temperatures, humidities, '-')
#     plt.show()
#     plt.savefig('chart_sensor1.png')

#   work in progress
def dataMap():
    for key in keys:
        data[key] = values[len(data)]
#  end of work in progress

def mapData():
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
    return data_a


@app.route("/")
def homePage():
    data_a = mapData()
    return render_template("index.html", data_a=data_a)


@app.route("/sensor")
def sensorPage():
    return render_template("sensor.html")


if __name__ == "__main__":
    app.debug = True
    app.run(port=5000)
