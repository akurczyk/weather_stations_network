import pymysql
import json
import time
import signal
import sys
import requests
from datetime import datetime
from decimal import Decimal

URL = "http://127.0.0.1:8081/recv_data"
DB_HOST = '127.0.0.1'
DB_PORT = 3306
DB_USER = 'gateway'
DB_PASS = 'zaq1@WSX'
DB_NAME = 'gateway'
SLEEP_DURATION = 5

class Model:
	def connect_to_db(this):
		this.connection = pymysql.connect(host=DB_HOST, port=DB_PORT, user=DB_USER, passwd=DB_PASS, db=DB_NAME)

	def get_data_from_db(this):
		cursor = this.connection.cursor()
		cursor.execute('SELECT * FROM measurements')
		result = cursor.fetchall();
		this.connection.commit()

		data_array = []
		for row in result:
			data_entry = {}
			i = 0
			for field in row:
				data_entry[cursor.description[i][0]] = field
				i += 1
			data_array.append(data_entry)
		return data_array

	def remove_data_from_db(this, id):
		cursor = this.connection.cursor()
		cursor.execute('DELETE FROM measurements WHERE id=%s', (id))
		this.connection.commit()

	def disconnect_from_db(this):
		this.connection.close()

class DataEncoder(json.JSONEncoder):
	def default(self, o):
		if isinstance(o, datetime):
			return o.isoformat(' ')[0:19]
		elif isinstance(o, Decimal):
			return float(o)
		else:
			return json.JSONEncoder.default(self, o)

model = Model()

def signal_handler(signal, frame):
	model.disconnect_from_db()
	sys.exit(0)

signal.signal(signal.SIGINT, signal_handler)

model.connect_to_db()
while True:
	data_array = model.get_data_from_db()
	if data_array:
		json_data = json.dumps(data_array, cls=DataEncoder)

		try:
			request = requests.post(URL, data={'json_data': json_data})
			if request.status_code == 200:
				print('Data sent successfully')

				for row in data_array:
					model.remove_data_from_db(row['id'])
			else:
				print('Received bad response from HTTP server')
		except requests.exceptions.RequestException:
			print('Could not connect to the upstream server')

	time.sleep(SLEEP_DURATION)
