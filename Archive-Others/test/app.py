
#FLASK CODE FOR DISPLAYING REAL TIME MEASUREMENTS AND EVERY SINGLE DATA FROM A DATABASE
#127.0.0.1:5000/measurement <--- current value
#127.0.0.1:5000/archive <--- database values

from flask import Flask, render_template
import psycopg2

app = Flask(__name__)

@app.route('/measurement', methods=['GET']) #PAGE TO DISPLAY REAL TIME VALUES
def measurement():
	con = psycopg2.connect(
            host = "localhost",
            database = "first_db",
            user = "postgres",
            password = "0000")


	cur = con.cursor()
	cur.execute("select * from database ORDER BY dt DESC LIMIT 1")

	result = cur.fetchall()

	cur.close()
	con.close()
	return render_template('database.html', measurement = result)


@app.route('/archive', methods=['GET']) #PAGE TO DISPLAY EVERY DATA IN THE DB
def archive():
	con = psycopg2.connect(
            host = "localhost",
            database = "first_db",
            user = "postgres",
            password = "0000")


	cur = con.cursor()
	cur.execute("select * from database")

	result2 = cur.fetchall()

	cur.close()
	con.close()
	return render_template('database.html', measurement = result2)

if __name__ == '__main__':
	app.run(debug=True)