#!/bin/sh
export FLASK_APP=/home/pi/apps/gateway_recv.py
export FLASK_DEBUG=1
python3 -m flask run --host=0.0.0.0 --port=8080
