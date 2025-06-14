# Project: Weather Station with Bluetooth and Dashboard

This project implements a **wireless weather station** based on an STM32 microcontroller, a Bluetooth module (HM-10), and a lightweight backend for logging and real-time data visualization.

## 📦 Project Structure

- `bluetooth_logger.py` – Receives data via BLE and logs it to `.csv` files
- `dashboard.py` – HTTP server with live updating charts
- `log.csv`, `mq9.csv`, `temperature.csv`, etc. – Log files
- Sensors: `BME280`, `MQ9`, etc.

## 🛠️ Requirements

### ✅ Python 3.9+
### ✅ Required Python libraries:
```
pip install bleak dash pandas plotly
```

## ⚙️ How It Works

### 🛰️ `bluetooth_logger.py`
- Scans for nearby BLE devices (e.g., HM-10 module).
- Connects to the device and listens for sensor data.
- Each incoming text line is logged into `log.csv`.
- Gas concentration (from MQ9) is parsed and written to `mq9.csv`.
- Optionally, temperature, pressure, and humidity can be extracted to separate files.

### 📊 `dashboard.py`
- Reads data from `log.csv`.
- Parses text and extracts values for:
  - Temperature
  - Pressure
  - Humidity
  - Gas concentration (MQ9)
- Refreshes charts every 10 seconds.
- Runs a local Dash web server (by default on `http://127.0.0.1:8050`).

## 🚀 Running the Project

1. **Start the logger** (make sure the BLE device is powered on):
```
python bluetooth_logger.py
```

2. **In a separate terminal, start the dashboard**:
```
python dashboard.py
```

3. **Open your browser and visit**:
```
http://127.0.0.1:8050/
```

## 🧪 Sample Input Format

The logger receives sensor messages in formats like:
```
Temperature: 24.6
Pressure: 1013.25
Humidity: 42.1
The estimated value based on Rs/R0=9.95 is: 15.8 ppm
```

The dashboard automatically detects and plots each value on its corresponding graph.

## 📌 Notes

- The logger runs asynchronously and supports Ctrl+C termination.
- If sensors send data at different intervals, it's fine — everything is timestamped and logged in real time.
- You can extend this project to add alerts, data export, or host the dashboard online (e.g., using `ngrok`).

