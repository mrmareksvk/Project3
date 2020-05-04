import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT


class Sensor_DB:
    def __init__(self, host="127.0.0.1", database="postgres", user="postgres", password=""):
        self.__DBconnection = psycopg2.connect(
            host=host, database="postgres", user=user, password=password
        )
        self.__DBconnection.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)

        self.__DBcursor = self.__DBconnection.cursor()

        self.__DBcursor.execute("SELECT 1 FROM pg_catalog.pg_database WHERE datname = %s", (database,),)
        self.__DBexists = self.__DBcursor.fetchone()
        if not self.__DBexists:
            self.__DBcursor.execute("CREATE DATABASE " + database)

        self.__DBcursor.close()
        self.__DBconnection.close()

        ###############################################

        self.__DBconnection = psycopg2.connect(
            host=host, database=database, user=user, password=password
        )
        self.__DBcursor = self.__DBconnection.cursor()

        self.__DBtables = set()

    def __createTable(self, tableID):
        self.__DBcursor.execute(
            "CREATE TABLE IF NOT EXISTS sensor%s ("
            "   id_entry SERIAL PRIMARY KEY,"
            "   temperature DOUBLE PRECISION,"
            "   humidity INT,"
            "   lux INT,"
            "   latitude DOUBLE PRECISION,"
            "   longitude DOUBLE PRECISION,"
            "   dt TIMESTAMP"
            ")",
            (int(tableID),),
        )
        self.__DBconnection.commit()

        self.__DBtables.add(tableID)

    def saveData(self, sensorID, temperature, humidity, lux, latitude, longitude, date, time):
        if sensorID not in self.__DBtables:
            self.__createTable(sensorID)
        self.__DBcursor.execute(
            "INSERT INTO sensor%s (temperature, humidity, lux, latitude, longitude, dt) VALUES(%s, %s, %s, %s, %s, %s)",
            (
                int(sensorID),
                temperature,
                humidity,
                lux,
                latitude,
                longitude,
                date + " " + time,
            ),
        )
        self.__DBconnection.commit()
