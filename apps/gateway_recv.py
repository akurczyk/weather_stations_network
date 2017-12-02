from flask import Flask, request
import pymysql
import json

DB_HOST = '127.0.0.1'
DB_PORT = 3306
DB_USER = 'gateway'
DB_PASS = 'zaq1@WSX'
DB_NAME = 'gateway'

def push_data_to_db(date_time, latitude, longitude, temperature, pressure, altitude, humidity):
	query = 'INSERT INTO measurements SET date_time=%s, latitude=%s, longitude=%s, '
	query += 'temperature=%s, pressure=%s, altitude=%s, humidity=%s'
	connection = pymysql.connect(host=DB_HOST, port=DB_PORT, user=DB_USER, passwd=DB_PASS, db=DB_NAME)
	cursor = connection.cursor()
	cursor.execute(query, (date_time, latitude, longitude, temperature, pressure, altitude, humidity))
	connection.commit()
	connection.close()

app = Flask(__name__)

@app.route('/recv_data', methods=['POST'])
def recv_data():
	try:
		data = json.loads(request.form['json_data'])
		push_data_to_db(data['date_time'], data['latitude'], data['longitude'], data['temperature'],
				data['pressure'], data['altitude'], data['humidity'])
		return 'OK'
	except (KeyError):
		return 'ERROR'

@app.route('/')
def main():
	return 'ERROR'
