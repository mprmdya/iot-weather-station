# IoT Weather Station Platform (ESP8266 + Python REST API + Chart.js)

Platform stasiun pemantau cuaca pintar mandiri berbasis sensor curah hujan, kelembapan, dan suhu dengan data terintegrasi ke cloud database secara berkala dan divisualisasikan melalui grafik interaktif Chart.js.

## 🌦️ Fitur Utama

- **Akuisisi Data Sensor Lingkungan**:
  - Sensor DHT11/DHT22: Pembacaan suhu (°C) dan kelembapan udara relatif (%).
  - Sensor Curah Hujan Analog (FC-37): Deteksi intensitas air hujan (0 - 100%) dan klasifikasi kondisi (Kering, Gerimis, Hujan Deras).
- **Protokol HTTP REST API**: Transmisi data JSON berkala dari modul WiFi ESP8266 ke backend server.
- **Python Backend Service (Flask + SQLite)**:
  - Menyediakan endpoint penerima telemetri (`/api/weather/telemetry`).
  - Penyimpanan persisten riwayat iklim ke database SQLite.
  - Endpoint statistik dan deret waktu (`/api/weather/history`).
- **Dashboard Visualisasi Interaktif (Chart.js)**:
  - Multi-axis Line Chart untuk tren suhu dan kelembapan.
  - Bar Chart untuk intensitas curah hujan historis.
  - Mode simulasi data bawaan untuk demo pengujian tanpa mikrokontroler fisik.

## 📐 Skema Rangkaian (Pinout ESP8266 NodeMCU)

| Sensor | Pin Sensor | Pin NodeMCU | Keterangan |
|---|---|---|---|
| DHT11 / DHT22 | Data Pin | `D4 (GPIO 2)` | Pull-up 10kΩ ke 3.3V |
| Raindrop Board | Analog OUT (AO) | `A0 (ADC)` | Deteksi intensitas presipitasi |
| Sensor VCC | VCC | `3.3V / VIN (5V)` | Sesuaikan spesifikasi modul |
| Ground | GND | `GND` | Common Ground |

## 📡 Dokumentasi Endpoint REST API

### 1. Ingest Telemetri Sensor
- **Method**: `POST`
- **Endpoint**: `/api/weather/telemetry`
- **Payload Request**:
  ```json
  {
    "stationId": "STATION_ALPHA_01",
    "temperature": 28.5,
    "humidity": 72.0,
    "rainIntensity": 25,
    "rainStatus": "Gerimis",
    "rawRain": 450
  }
  ```

### 2. Dapatkan Metrik Terkini
- **Method**: `GET`
- **Endpoint**: `/api/weather/latest`

### 3. Dapatkan Riwayat Waktu (Untuk Chart)
- **Method**: `GET`
- **Endpoint**: `/api/weather/history?limit=30`

## 🚀 Panduan Menjalankan

### 1. Jalankan Backend Python
```bash
cd backend-python
pip install -r requirements.txt
python app.py
```
Server akan aktif di `http://localhost:5000`.

### 2. Buka Frontend Dashboard
Cukup buka `http://localhost:5000` di peramban Anda (atau buka file [`frontend/index.html`](./frontend/index.html) secara langsung).

### 3. Upload Firmware ESP8266
Buka [`firmware/esp8266_weather_station.ino`](./firmware/esp8266_weather_station.ino) di Arduino IDE, sesuaikan SSID WiFi dan IP server backend, lalu upload ke board NodeMCU Anda.

---
Dibuat dengan ❤️ oleh [mprmdya](https://github.com/mprmdya).
