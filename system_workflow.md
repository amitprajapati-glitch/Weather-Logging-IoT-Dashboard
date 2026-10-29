System Workflow — Weather Logging & IoT Dashboard

1. System Overview

The system uses an ESP8266 NodeMCU to collect environmental readings from sensors, display them locally, and send data to the ThingSpeak cloud platform.

2. Working Process

1. The ESP8266 starts and initializes the connected sensors and OLED display.
2. The DHT22 measures temperature and relative humidity.
3. The BMP280 measures atmospheric pressure.
4. The ESP8266 processes the readings.
5. The OLED displays the sensor values.
6. The ESP8266 connects to Wi-Fi and sends readings to ThingSpeak.
7. The push button controls the system ON/OFF state.
8. The buzzer provides an audio indication when the system turns on or off.

3. System Architecture

Sensors → ESP8266 NodeMCU → OLED Display

ESP8266 NodeMCU → Wi-Fi → ThingSpeak Cloud Dashboard

Push Button → ESP8266 NodeMCU → System Control and Buzzer Alert

4. Technical Concepts

- Sensor interfacing
- Embedded C++ programming
- I2C communication
- Wi-Fi connectivity
- IoT cloud data visualization
- Hardware and software integration

5. Purpose

The project demonstrates how embedded hardware, environmental sensors, and cloud services can work together to monitor environmental conditions.
