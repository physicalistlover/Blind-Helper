# Bill of Materials — Blind Helper V1

| Component | Quantity | Purpose |
|---|---:|---|
| ESP32-C3 Super Mini | 1 | Main microcontroller |
| TF-Luna LiDAR | 1 | Distance measurement |
| Vibration motor | 1 | Tactile feedback |
| 2N2222 NPN transistor | 1 | Motor driver |
| 470 ohm resistor | 1 | Transistor base current limiting |
| 10 kohm resistor | 1 | Base pull-down |
| Flyback diode | 1 | Motor transient protection |
| Rechargeable battery | 1 | Power source |
| Power switch | 1 | Device ON/OFF |
| Prototype PCB/breadboard | 1 | Prototype construction |

## Notes

The final motor supply voltage must correspond to the rated voltage of the selected vibration motor.

The ESP32 GPIO must never directly supply the vibration motor current.

The transistor or a suitable MOSFET must be used as the motor driver.
