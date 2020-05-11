from flask import Flask, render_template, request, redirect
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy import create_engine
import matplotlib.pyplot as plt
from matplotlib import style
# from wtforms import IntegerField, SubmitField
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


@app.route('/', )
def hello_world():

    return render_template('index.html')


@app.route('/sensor/<sensorID>')
def hello(sensorID=None):
    # create_engine('postgresql://user:password@localhost/database')
    engine = create_engine('postgresql://postgres:gabriel12@127.0.0.1:5432/test')
    with engine.connect() as con:
        rs = con.execute("SELECT * FROM sensor1 ORDER BY dt DESC LIMIT 1")
        for row in rs:
            temperature = row[1]
            humidity = row[2]
            lux = row[3]
            latitude = row[4]
            longitude = row[5]
            dt = row[6]

    return render_template('sensor1.html', number=sensorID, dt=dt,
                           lon=longitude, lat=latitude, temp=temperature,
                           hum=humidity, lux=lux)


if __name__ == '__main__':
    app.debug = True
    app.run(port=5000)
