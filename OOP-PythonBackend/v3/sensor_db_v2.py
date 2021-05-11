import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT


class Sensor_DB:
    def __init__(self, database="postgres", DBusername="postgres", DBpassword=""):
        self.__DBname = database
        self.__DBusername = DBusername
        self.__DBpassword = DBpassword
        self.__DBtables = set()

    def setDB(self, database):
        self.__DBname = database

    def setLogin(self, username, password):
        self.__DBusername = username
        self.__DBpassword = password

    def connectDB(self, host="127.0.0.1", port="5432"):
        if "" not in {self.__DBname, self.__DBusername, self.__DBpassword}:
            self.__DBconnection = psycopg2.connect(
                host=host, port=port, database="postgres", user=self.__DBusername, password=self.__DBpassword
            )
            self.__createDB(self.__DBname)
            self.__DBconnection.close()

            self.__DBconnection = psycopg2.connect(
                host=host, port=port, database=self.__DBname, user=self.__DBusername, password=self.__DBpassword
            )
            self.__DBcursor = self.__DBconnection.cursor()

    def __createDB(self, database):
        self.__DBconnection.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)
        self.__DBcursor = self.__DBconnection.cursor()
        self.__DBcursor.execute("SELECT 1 FROM pg_catalog.pg_database WHERE datname = %s", (database,),)
        self.__DBexists = self.__DBcursor.fetchone()
        if not self.__DBexists:
            self.__DBcursor.execute("CREATE DATABASE " + database)
        self.__DBcursor.close()

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
