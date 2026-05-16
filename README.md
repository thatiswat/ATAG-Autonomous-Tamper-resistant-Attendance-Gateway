# ATAG
## Autonomous Tamper-resistant Attendance Gateway

ATAG is a BLE + motion intelligence based autonomous presence verification system built using ESP32 and MPU6050.

The system performs real-time attendance verification using:
- BLE zone detection
- Human motion validation
- Stateful occupancy logic
- Real-time monitoring dashboard

Unlike traditional attendance systems, ATAG reduces proxy/spoof attendance by combining wireless presence detection with motion-based human verification.

---

# System Architecture
<img width="1000" height="1000" alt="shopping" src="https://github.com/user-attachments/assets/cb868d8e-f551-4573-bb10-e0403b666165" />

<img width="252" height="133" alt="image" src="https://github.com/user-attachments/assets/6b4db1c9-6b38-47b6-b3ce-87cdd1031a73" />

---

# Features

- BLE-based autonomous attendance
- Motion verified human detection
- Tamper-resistant occupancy logic
- Entry/exit debounce system
- Autonomous timeout recovery
- Real-time Flask dashboard
- BLE RSSI zone filtering
- Stateful presence engine

---

# Hardware Used

| Component | Purpose |
|---|---|
| ESP32 | BLE communication |
| MPU6050 | Human motion verification |
| Flask Server | Dashboard backend |
| HTML/CSS | Monitoring UI |

---

# Presence Verification Logic

ATAG uses two independent verification layers:

## 1. BLE Zone Detection

The wearable continuously scans for a predefined BLE zone beacon.

If:
- RSSI > threshold
- beacon stable for delay duration

the system assumes zone proximity.

---

## 2. Human Motion Verification

The MPU6050 continuously samples acceleration data.

Variance analysis is performed:

```cpp
variance =
Δ(ax) + Δ(ay) + Δ(az)
