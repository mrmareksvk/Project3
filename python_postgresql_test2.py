import psycopg2
from psycopg2.extensions import ISOLATION_LEVEL_AUTOCOMMIT

##################

con = psycopg2.connect(
    host="127.0.0.1", database="postgres", user="postgres", password="admin"
)
con.set_isolation_level(ISOLATION_LEVEL_AUTOCOMMIT)

cur = con.cursor()

cur.execute("SELECT 1 FROM pg_catalog.pg_database WHERE datname = 'createtest'")
exists = cur.fetchone()
if not exists:
    cur.execute('CREATE DATABASE createtest')

cur.close()
con.close()

##################

con = psycopg2.connect(
    host="127.0.0.1", database="createtest", user="postgres", password="admin"
)

cur = con.cursor()

cur.execute(
    "CREATE TABLE IF NOT EXISTS testtable ("
    "   id_person SERIAL PRIMARY KEY,"
    "   name VARCHAR(32),"
    "   age INT"
    ")"
)

cur.execute("INSERT INTO testtable (name, age) VALUES('Adam', 19)")
cur.execute("INSERT INTO testtable (name, age) VALUES('Marek', 20)")

cur.execute("SELECT * FROM testtable")
rows = cur.fetchall()
print(rows)

# commit changes
con.commit()

cur.close()
con.close()
