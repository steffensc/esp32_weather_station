#define USE_CCS811 true
#define USE_OLED false



#define BME_ADDRESS 0x76
#define CCS811_ADDRESS 0x5A

#define SSD_ADDRESS 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define DISPLAY_ON_TIME 7500

#define TOUCH_PIN_THROSHOLD 40 // The greater the value, the more the sensitivity

#define WIFI_CONN_CHECK_INTERVAL 1000
#define WIFI_CONN_MAX_CHECK_COUNT 5

#define uS_TO_S_FACTOR 1000000  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP 300       // Time ESP32 will go to sleep (in seconds)


// INCLUDES
#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#if (USE_CCS811)
#include "Adafruit_CCS811.h"
#endif
#if (USE_OLED)
#include <Adafruit_GFX.h>
#endif
#include <Adafruit_SSD1306.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>

#include <config.h>


// CONFIGURATION VARS
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

String apiEndpoint = SERVER_URL;
String apiToken = SERVER_TOKEN;
String sensorNo = SENSOR_NO; // Watch out for keeping this unique

float temperature_correction = -0.5;
float pressure_correction = 0;
float humidity_correction = 0;


// GLOBAL VARS
Adafruit_BME280 bme;
#if (USE_OLED)
Adafruit_SSD1306 display;
#endif
#if (USE_CCS811)
Adafruit_CCS811 ccs;
#endif


float temperature;
float pressure;
float humidity;

#if (USE_CCS811)
float eCO2;
float TVOC;
#endif

RTC_DATA_ATTR int bootCount = 0;
boolean wifi_error_state = false;



/* - - - - - - METHODS - - - - - - */
void environment_measurement(int measurements, int delay_time) {
  float avg_temperature = 0;
  float avg_pressure = 0;
  float avg_humidity = 0;

  #if (USE_CCS811)
  float avg_eCO2 = 0;
  float avg_TVOC = 0;
  ccs.setTempOffset(bme.readTemperature()); // set Temperature offset via bme Temperature
  #endif


  for (int i = 0; i < measurements; i++)
  {
    avg_temperature = avg_temperature + bme.readTemperature();
    avg_pressure = avg_pressure + bme.readPressure() / 100;
    avg_humidity = avg_humidity + bme.readHumidity();

    #if (USE_CCS811)
    avg_eCO2 = avg_eCO2 + ccs.geteCO2();
    avg_TVOC = avg_TVOC + ccs.getTVOC();
    #endif

    delay(delay_time);
  }

  temperature = (avg_temperature / measurements) + temperature_correction;
  pressure = (avg_pressure / measurements) + pressure_correction;
  humidity = (avg_humidity / measurements) + humidity_correction;

  #if (USE_CCS811)
  eCO2 = (avg_eCO2 / measurements);
  TVOC = (avg_TVOC / measurements);
  #endif
}

boolean connectWifi(const char *ssid , const char *password, int interval, int max_con_count) {
  // Try to connect
  Serial.println("Connecting to wifi...");
  Serial.println(ssid);
  Serial.println(password);
  WiFi.begin(ssid, password);

  int wifiConnCheckCount = 0;
  while (WiFi.status() != WL_CONNECTED && wifiConnCheckCount < max_con_count)
  {
    delay(interval);
    wifiConnCheckCount++;
    Serial.print(".");
  }
  Serial.println();

  // Check if connected
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Could not connect to wifi");
    WiFi.disconnect();
    return false;
  }
  else
  {
    Serial.println("Wifi connected. IP address: " + WiFi.localIP().toString());
    return true;
  }
}


boolean sendSensorData() {
  if ((WiFi.status() == WL_CONNECTED))
  {
    Serial.println("Sending data to api");

    // Start new http connection
    HTTPClient http;
    http.begin(apiEndpoint);

    // Format data as json
    String htmlapirequest = "token=" + apiToken + "&";
    htmlapirequest += "sensor=" + sensorNo + "&";
    htmlapirequest += "temperature=" + String(temperature) + "&";
    htmlapirequest += "humidity=" + String(humidity) + "&";
    htmlapirequest += "pressure=" + String(pressure);
    #if (USE_CCS811)
    htmlapirequest += "&eco=" + String(eCO2) + "&";
    htmlapirequest +=  "tvoc=" + String(TVOC);
    #endif

    Serial.println(htmlapirequest);

    // Post json
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpCode = http.POST(htmlapirequest);

    // Check if request was successful
    if (httpCode == 200)
    {
      Serial.println("Data was successfully sent to api");
    }
    else
    {
      Serial.println("Error sending data to api: " + String(httpCode));
    }

    // Close connection
    http.end();

    return true;
  }

  return false;
}

#if (USE_OLED)
void displaySensorData(int displaytime) {
  Serial.println("Display Sensor Data");

  display.clearDisplay();
  display.setCursor(0, 0);

  display.setTextSize(2);
  display.cp437(true);
  display.println(String(temperature));
  display.setCursor(60, 0);

  display.drawCircle(62, 2, 2, SSD1306_WHITE);

  
  display.setCursor(80, 0);
  display.setTextSize(1);
  if(wifi_error_state)
  {
    display.println("WiFi OK");
  }
  else
  {
    display.println("WiFi E!");
  }
  
  display.setCursor(0, 24);
  display.setTextSize(1);
  display.println(String(humidity) + "% " + String(pressure) + " hPa");
  display.display();


  delay(displaytime);

  display.clearDisplay();
  display.display();
}
#endif

void callback(){
  //placeholder callback function
}


/* - - - - - - - MAIN - - - - - - - */
void setup() {
  // put your setup code here, to run once:
  ++bootCount;

  // SERIAL //
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  // SLEEP WAKEUP //
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); //TIMER
  esp_sleep_enable_touchpad_wakeup(); //TOUCH
  touchAttachInterrupt(T3, callback, TOUCH_PIN_THROSHOLD);


  // TEMPERATURE BME //
  int bme_status = bme.begin(BME_ADDRESS);
  if (!bme_status)
  {
    Serial.println("Could not find BME280!");
    while (1)
      ;
  }


  // ENVIRONMENT CCS //
  #if (USE_CCS811)
  if(!ccs.begin(CCS811_ADDRESS)){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }
  
  //calibrate temperature sensor
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
  #endif


  // DISPLAY //
  #if (USE_OLED)
  display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
  display.begin(SSD1306_SWITCHCAPVCC, SSD_ADDRESS);
  
  // Display splash screen
  if(bootCount < 2){
    display.display();
    delay(2000);
  }

  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  #endif


  // Start WiFi
  wifi_error_state = connectWifi(ssid, password, WIFI_CONN_CHECK_INTERVAL, WIFI_CONN_MAX_CHECK_COUNT);
}

void loop() {
  // put your main code here, to run repeatedly:

  environment_measurement(3, 1000);

  delay(100);

  sendSensorData();

  #if (USE_OLED)
  if(esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TOUCHPAD)
  {
    displaySensorData(DISPLAY_ON_TIME);
  }
  #endif

  esp_deep_sleep_start();  
}
