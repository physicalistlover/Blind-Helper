# Blind Helper V1 Wiring

## TF-Luna to ESP32-C3

| TF-Luna | ESP32-C3 |
|---|---|
| +5V | 5V |
| SDA | GPIO4 |
| SCL | GPIO3 |
| GND | GND |
| MODE | GND |
| Pin 6 | Not connected |

## Vibration Motor Driver

GPIO1 is used to generate the motor PWM control signal.

Connection:

GPIO1 -> 470 ohm resistor -> 2N2222 Base

A 10 kohm resistor connects:

2N2222 Base -> GND

The pull-down resistor ensures that the transistor remains OFF when the ESP32 GPIO is floating during startup or reset.

### Transistor

2N2222 Collector -> Motor negative

2N2222 Emitter -> GND

Motor positive -> Motor supply positive

## Flyback Diode

The diode is connected across the vibration motor.

Cathode / striped side -> Motor positive

Anode -> Motor negative / 2N2222 collector

The flyback diode suppresses the inductive voltage transient produced when current through the motor is switched off.

## Ground

The ESP32 ground and motor power supply ground must share a common electrical reference.
