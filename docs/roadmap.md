# Blind Helper Development Roadmap

## Version 1 — Forward Obstacle Detection

ESP32-C3
+
TF-Luna
+
Vibration motor

Goal:

Prove that LiDAR distance can be converted into useful tactile feedback.

---

## Version 2 — Wearable Prototype

Goals:

- Rechargeable battery
- Battery monitoring
- Improved enclosure
- ON/OFF switch
- Improved vibration patterns
- Reduced power consumption

---

## Version 3 — Direction Detection

Introduce multiple ranging sensors.

Possible arrangement:

LEFT    CENTER    RIGHT

Feedback should communicate both:

- Distance
- Direction

---

## Version 4 — Ground Hazard Detection

Add a downward-facing sensor.

Detect:

- Downward stairs
- Pavement edges
- Holes
- Raised steps
- Sudden changes in ground height

Potential additional sensor:

IMU

The IMU can help compensate for sensor orientation changes caused by body movement.

---

## Version 5 — Smart Mobility Platform

Potential technologies:

- Camera
- Computer vision
- Object recognition
- GPS
- Smartphone application
- Bluetooth
- Voice feedback
- AI scene interpretation

Long-term objective:

Combine geometric obstacle sensing with semantic environmental understanding.
