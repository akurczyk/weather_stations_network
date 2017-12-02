from flask import Flask, request, Response
import pymysql
import json
from datetime import datetime
from decimal import Decimal

DB_HOST = '127.0.0.1'
DB_PORT = 3306
DB_USER = 'server'
DB_PASS = 'zaq1@WSX'
DB_NAME = 'server'

def push_data_into_db(data):
	query = 'INSERT INTO measurements SET date_time=%s, latitude=%s, longitude=%s, '
	query += 'temperature=%s, pressure=%s, altitude=%s, humidity=%s'
	connection = pymysql.connect(host=DB_HOST, port=DB_PORT, user=DB_USER, passwd=DB_PASS, db=DB_NAME)

	for row in data:
		cursor = connection.cursor()
		cursor.execute(query, (row['date_time'], row['latitude'], row['longitude'], row['temperature'], row['pressure'], row['altitude'], row['humidity']))
		connection.commit()

	connection.close()

def pull_data_from_db():
	query = 'SELECT * FROM measurements WHERE id IN'
	query += ' (SELECT MAX(id) FROM measurements GROUP BY latitude, longitude)'
	query += ' ORDER BY id DESC LIMIT 0, 1000'

	connection = pymysql.connect(host=DB_HOST, port=DB_PORT, user=DB_USER, passwd=DB_PASS, db=DB_NAME)
	cursor = connection.cursor()
	cursor.execute(query)
	result = cursor.fetchall()
	connection.commit()
	connection.close()

	data_array = []
	for row in result:
		data_entry = {}
		i = 0
		for field in row:
			data_entry[cursor.description[i][0]] = field
			i += 1
		data_array.append(data_entry)
	return data_array

class DataEncoder(json.JSONEncoder):
	def default(self, o):
		if isinstance(o, datetime):
			return o.isoformat(' ')[0:19]
		elif isinstance(o, Decimal):
			return float(o)
		else:
			return json.JSONEncoder.default(self, o)

app = Flask(__name__)

@app.route('/recv_data', methods=['POST'])
def recv_data():
	try:
		data = json.loads(request.form['json_data'])
		push_data_into_db(data)
		print('Received data')
		return 'OK'
	except KeyError:
		return 'ERROR'

@app.route('/')
def main():
	response = Response(json.dumps(pull_data_from_db(), cls=DataEncoder))
	response.headers['Access-Control-Allow-Origin'] = '*'
	return response
