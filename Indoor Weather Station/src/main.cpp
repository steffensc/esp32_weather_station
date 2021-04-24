#define USE_CCS811 true
#define USE_OLED false



#define BME_ADDRESS_USE 0x76
#define CCS811_ADDRESS_USE 0x5A

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
#include "ccs811.h"
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
CCS811 ccs811; 
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
uint16_t convert_to_ccs811_envformat(float value) {
  uint16_t hi_part = trunc(value);
  hi_part = hi_part & 0x007f; // "cut off first 9 bits (set them to 0) for having a 7 bit uinteger"
  hi_part = hi_part << 9; // shifting 9 bits (adding 9 0-bits to the right)

  float fraction_part = (value - trunc(value));
  uint16_t lo_part = uint16_t(512 * fraction_part); // get 9bit fraction
  lo_part = lo_part & 0x01ff; // "cut off first 7 bits (set them to 0) for having a 7 bit uinteger"

  uint16_t result = hi_part | lo_part;

  return result;
}


void environment_measurement(int measurements, int delay_time) {
  float avg_temperature = 0;
  float avg_pressure = 0;
  float avg_humidity = 0;
  float temp, pres, humi;

  #if (USE_CCS811)
  float avg_eCO2 = 0;
  float avg_TVOC = 0;
  int ccs811_measurements = 0;
  uint16_t eco2, etvoc, errstat, raw;
  #endif


  for (int i = 0; i < measurements; i++)
  {
    temp = bme.readTemperature();
    pres = bme.readPressure() / 100;;
    humi = bme.readHumidity();

    avg_temperature = avg_temperature + temp;
    avg_pressure = avg_pressure + pres;
    avg_humidity = avg_humidity + humi;

    delay(100);

    #if (USE_CCS811)
    ccs811.set_envdata(convert_to_ccs811_envformat(temp), convert_to_ccs811_envformat(humi));

    ccs811.read(&eco2,&etvoc,&errstat,&raw); 
    if( errstat==CCS811_ERRSTAT_OK ) { 
      avg_eCO2 = avg_eCO2 + eco2;
      avg_TVOC = avg_TVOC + etvoc;
      ccs811_measurements += 1;
    }
    #endif

    delay(delay_time);
  }

  temperature = (avg_temperature / measurements) + temperature_correction;
  pressure = (avg_pressure / measurements) + pressure_correction;
  humidity = (avg_humidity / measurements) + humidity_correction;

  #if (USE_CCS811)
  if (ccs811_measurements > 0){
    eCO2 = (avg_eCO2 / ccs811_measurements);
    TVOC = (avg_TVOC / ccs811_measurements);
  }
  else{
    eCO2 = 0;
    TVOC = 0;
  }
  Serial.println(eCO2);
  Serial.println(TVOC);
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
  int bme_status = bme.begin(BME_ADDRESS_USE);
  if (!bme_status)
  {
    Serial.println("Could not find BME280!");
    while (1);
  }


  // ENVIRONMENT CCS //
  #if (USE_CCS811)
  Wire.begin();
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  // Enable CCS811
  if( !ccs811.begin() ){
    Serial.println("setup: CCS811 begin FAILED");
  }
  // Start measuring
  if( !ccs811.start(CCS811_MODE_1SEC) ){
    Serial.println("setup: CCS811 start FAILED");
  }
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
