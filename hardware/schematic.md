# Blind Helper V1 Schematic

## Simplified Circuit

             + Motor Supply
                   |
                   |
             +-----+------+
             |            |
           MOTOR        DIODE
             |            |
             +------------+
             |
             |
        Collector
           2N2222
        Emitter
             |
            GND


ESP32 GPIO1
     |
    470R
     |
     +---------- Base 2N2222
     |
    10k
     |
    GND


TF-Luna
-----------------
VCC  -> 5V
GND  -> GND
SDA  -> GPIO4
SCL  -> GPIO3
MODE -> GND
