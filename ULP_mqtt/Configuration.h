// Configuration.h
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "Secrets.h"

#define TOPIC_PREFIX "home"

// Power meter config
#define DEVICE_NAME "power"
#define ULP_SLEEP_TIME 15
#define ULP_COUNT_PIN_PULLUP_EN false

#define GPIO_SENSOR_PIN GPIO_NUM_9  // GPIO pin connected to the sensor
#define RTC_GPIO_INDEX 16            // attain dynamically with: rtc_io_number_get(GPIO_SENSOR_PIN)
//https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gpio.html

#define LED_PIN 21
#define BATTERY_PIN 1        // ADC pin for battery voltage (use ADC1 pins 32-29 as wifi can interfere with ADC2 pins)
#define NO_SLEEP_PIN 2
#define VOLTAGE_DIVIDER 2.31   // Voltage divider ratio (R1 + R2) / R2  - should be 2.0 - however may be some issue with the voltage reg etc
#define ADC_RESOLUTION 4095   // 12-bit ADC resolution
#define REF_VOLTAGE 3.3       // Reference voltage of ESP32 ADC

#include <Arduino.h> //for serial

void loadConfiguration();

#endif
