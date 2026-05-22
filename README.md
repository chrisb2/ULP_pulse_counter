# ESP32 Power Monitor with MQTT and Home Assistant Integration

This project implements a low-power ESP32S3-based power monitoring system that uses the ULP co-processor for energy-efficient operation. It periodically wakes up from deep sleep, counts LED pulses from a power meter, and transmits data over WiFi using MQTT to Home Assistant.

## Features

- **Low-power operation** using the ULP co-processor on the ESP32S3.
- **Real-time pulse counting** providing total pulse count and pulse rate
- **Wifi and battery voltage monitoring**
- **WiFi connectivity** for transmitting data via MQTT.
- **Home Assistant integration** for monitoring power and energy usage.

---

## Overview

1. **Pulse Counter**: The ESP32S3 ULP co-processor counts LED pulses during deep sleep.
2. **Data Transmission**: After a preset interval, the ESP32S3 wakes up, connects to WiFi, and sends the pulse count and rate to MQTT topics.
3. **Home Assistant**: Configures MQTT sensors to display energy (kWh) and power (kW) readings.

---

## Hardware Requirements

- **ESP32S3 Board**: Seeedstudio XIAO ESP32S3
- **Voltage Divider**: Implement an LDR-based voltage divider for LED pulse sensing.

### Power Optimization

- Remove onboard voltage regulator and LEDs.
- Minimize power consumption by selecting boards with efficient USB-to-serial chips.

### LDR Voltage Divider Notes ##

- Monitoring the LED with a LDR
- Disable pull up and pull down
- LDR = 100kOhm with LED on, 500kOhm with LED off
- LDR to ground, 220kOhm to 3.3V
- Voltage = 0.7V LED on, 2.5V LED off

### Battery Voltage Divider Notes ##

- I used two equal values resistors for the battery voltage dividers
- I used quite high value resistors (330 kOhm) to minimise passive current draw.
- However there is impedance in the ADC input pin of approx 1 MOhm which needs to be accounted for in the voltage divider calc.
- An alternative would be to use a GPIO to turn on the voltage divider to take readings, then lower value resistors (i.e. 10 kOhm) can be used.

---

## Software

### Arduino Code

1. **ULP Co-Processor**:
   - Modified from [Michael Thomas ULP_pulse_counter](https://github.com/michaelt88/ULP_pulse_counter) which was based on example by [Neurotech Hub](https://github.com/Neurotech-Hub/ULP_Example-ESP32-S3-Arduino).
   - Added functionality to track total count, and to determine the pulse rate.

2. **WiFi and MQTT**:
   - Uses the [Async MQTT client](https://github.com/marvinroger/async-mqtt-client).
   - Sends data to `home/power/count`, `home/power/rate`, `home/power/voltage` and `home/power/signal` topics.

---

## Home Assistant Configuration

Add the contents of `ha-configuration.yaml` to your `configuration.yaml` file.