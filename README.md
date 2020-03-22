# ESP32 Micro controller
## Weather Sensor
Hardware:
- ESP32 Node MCU
- I2C BME280 (temperature, pressure, humidity)

Pin G21 is I2C Clock
Pin G22 is I2C Data

The sensor software sets the ESP32 to deepsleep. A timer interrupt wakes the controller every 5 minutes which then performs a measurement, sends the data to the "Weather Server" and sets the controller back to sleep then.



## Weather Station
Hardware:
- ESP32 Node MCU
- I2C BME280 (temperature, pressure, humidity)
- I2C 128x32 Oled Display
- (Touch Pin)

Pin G21 is I2C Clock
Pin G22 is I2C Data

The station software is a more advanced version of the sensor. It is extended with an OLED display (connected to the same I2C Pins as the BME Sensor) which is used for displaying the currently measured temperature, pressure an humidity of the environment.
It comes with a second interrupt when which is called activates the OLED.
When the Timer interrupt is called the OLED stays off.

# Weather Server

