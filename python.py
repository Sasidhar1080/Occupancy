from flask import Flask, request
import time
from threading import Thread
import requests
import json

app = Flask(__name__)

node_data = {
    'AQ-KH00-00': None,
    'AQ-KH00-01': None
}

def print_motion_values():
    while True:
        payload_data = {}  # Initialize payload_data

        for node_id, motion_value in node_data.items():
            if motion_value is not None:
                payload_data[node_id] = motion_value
                node_data[node_id] = None  # Reset motion value after adding to payload
            else:
                payload_data[node_id] = float('nan')  # Assign nan for nodes without recorded data

        timestamp = int(time.time())  # Get timestamp value
        
        payload = json.dumps({"m2m:cin": {"con": json.dumps({timestamp: payload_data})}})
        
        headers = {
            'X-M2M-Origin': 'Tue_20_12_22:Tue_20_12_22',
            'Content-Type': 'application/json;ty=4'
        }
        url = f"http://dev-onem2m.iiit.ac.in:443/~/in-cse/in-name/AE-SR/SR-OC/SR-OC-GW-KH00-00/Data"
        
        response = requests.post(url, headers=headers, data=payload)
        
        print(f"Sent motion values: {payload_data}. Response: {response.text}")

        time.sleep(15)  # Wait for a minute

@app.route('/receive_motion', methods=['POST'])
def receive_motion():
    data = request.json
    motion_value = data.get('motion')
    timestamp = data.get('timestamp')
    node_id = data.get('node_id')

    if node_id in node_data:
        node_data[node_id] = motion_value

    return "Motion data received successfully"

if __name__ == '__main__':
    scheduler = Thread(target=print_motion_values)
    scheduler.daemon = True
    scheduler.start()
    app.run(host='0.0.0.0', port=8080)
