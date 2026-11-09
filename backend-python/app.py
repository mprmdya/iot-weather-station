import os
import sqlite3
from datetime import datetime
from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS

app = Flask(__name__, static_folder='../frontend')
CORS(app)

DB_PATH = os.path.join(os.path.dirname(__file__), 'weather.db')

def init_db():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS weather_records (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            station_id TEXT NOT NULL,
            temperature REAL NOT NULL,
            humidity REAL NOT NULL,
            rain_intensity INTEGER NOT NULL,
            rain_status TEXT NOT NULL,
            raw_rain INTEGER,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    conn.commit()
    conn.close()

init_db()

@app.route('/')
def index():
    return send_from_directory(app.static_folder, 'index.html')

@app.route('/<path:path>')
def static_proxy(path):
    return send_from_directory(app.static_folder, path)

@app.route('/api/weather/telemetry', methods=['POST'])
def receive_telemetry():
    data = request.get_json(force=True, silent=True)
    if not data:
        return jsonify({'error': 'Payload JSON tidak valid'}), 400

    station_id = data.get('stationId', 'UNKNOWN_STATION')
    temperature = data.get('temperature')
    humidity = data.get('humidity')
    rain_intensity = data.get('rainIntensity', 0)
    rain_status = data.get('rainStatus', 'Normal')
    raw_rain = data.get('rawRain', 0)

    if temperature is None or humidity is None:
        return jsonify({'error': 'Parameter temperature dan humidity wajib diisi'}), 422

    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO weather_records (station_id, temperature, humidity, rain_intensity, rain_status, raw_rain)
        VALUES (?, ?, ?, ?, ?, ?)
    ''', (station_id, temperature, humidity, rain_intensity, rain_status, raw_rain))
    conn.commit()
    record_id = cursor.lastrowid
    conn.close()

    return jsonify({
        'status': 'success',
        'message': 'Data cuaca tersimpan',
        'recordId': record_id
    }), 201

@app.route('/api/weather/latest', methods=['GET'])
def get_latest():
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        SELECT id, station_id, temperature, humidity, rain_intensity, rain_status, timestamp
        FROM weather_records
        ORDER BY id DESC LIMIT 1
    ''')
    row = cursor.fetchone()
    conn.close()

    if not row:
        return jsonify({
            'status': 'empty',
            'data': {
                'temperature': 27.5,
                'humidity': 68.0,
                'rainIntensity': 0,
                'rainStatus': 'Kering',
                'stationId': 'DEMO_SIMULATION',
                'timestamp': datetime.now().strftime('%Y-%m-%d %H:%M:%S')
            }
        })

    return jsonify({
        'status': 'success',
        'data': {
            'id': row[0],
            'stationId': row[1],
            'temperature': row[2],
            'humidity': row[3],
            'rainIntensity': row[4],
            'rainStatus': row[5],
            'timestamp': row[6]
        }
    })

@app.route('/api/weather/history', methods=['GET'])
def get_history():
    limit = request.args.get('limit', 20, type=int)
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    cursor.execute('''
        SELECT timestamp, temperature, humidity, rain_intensity, rain_status
        FROM weather_records
        ORDER BY id DESC LIMIT ?
    ''', (limit,))
    rows = cursor.fetchall()
    conn.close()

    # Urutkan kronologis menaik untuk sumbu waktu Chart.js
    rows.reverse()

    history = [
        {
            'timestamp': r[0],
            'temperature': r[1],
            'humidity': r[2],
            'rainIntensity': r[3],
            'rainStatus': r[4]
        }
        for r in rows
    ]

    return jsonify({'status': 'success', 'count': len(history), 'history': history})

if __name__ == '__main__':
    print("=== IoT Weather Station Python API Server ===")
    print("Berjalan pada http://0.0.0.0:5000")
    app.run(host='0.0.0.0', port=5000, debug=True)
