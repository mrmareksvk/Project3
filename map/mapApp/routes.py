from flask import Flask, render_template

app = Flask(__name__)


@app.route("/map")
def map():
    # different tables - just number of sensors
    tables = ["sensor1", "sensor2", "sensor3", "sensor4"]
    sensors = len(tables)
    data_a = [
        {"name": "sensor1", "state": "ONLINE", "temp": "20.2", "hum": "40%", "light": "970", "datetime": "11.5.2020 12:25", "lat": "56.11988", "lon": "10.15921"},
        {"name": "sensor2", "state": "OFFLINE", "temp": "19", "hum": "35%", "light": "785", "datetime": "11.5.2020 12:35", "lat": "56.11919", "lon": "10.15833"},
    ]

    return render_template("map.html", sensors=sensors, data_a=data_a)


if __name__ == "__main__":
    app.run(debug=True)
