/*
// ENTERPRISE WIFI FOR ESP8266
#include <ESP8266WiFi.h>
#include "user_interface.h"
#include "wpa2_enterprise.h"

#include <ESP8266HTTPClient.h>


boolean connectEnterpriseWifi(const char *ssid , const char *password, const char *username, int interval, int max_con_count) {
  // Try to connect
  //Serial.println("Connecting to wifi...");
  WiFi.hostname("ESP-host-" + String(sensorNo));
  WiFi.mode(WIFI_STA);

  // Setting ESP into STATION mode only (no AP mode or dual mode)
  wifi_set_opmode(STATION_MODE);
  struct station_config wifi_config;
  memset(&wifi_config, 0, sizeof(wifi_config));
  strcpy((char*)wifi_config.ssid, ssid);
  wifi_station_set_config(&wifi_config);
  wifi_station_clear_cert_key();
  wifi_station_clear_enterprise_ca_cert();
  wifi_station_set_wpa2_enterprise_auth(1);
  wifi_station_set_enterprise_identity((uint8*)username, strlen(username));
  wifi_station_set_enterprise_username((uint8*)username, strlen(username));
  wifi_station_set_enterprise_password((uint8*)password, strlen(password));
  wifi_station_connect();

  WiFi.begin(ssid, password);
  Serial.println("Waiting for connection and IP Address from DHCP");
  int wifiConnCheckCount = 0;
  while (WiFi.status() != WL_CONNECTED && wifiConnCheckCount < max_con_count)
  {
    delay(interval);
    Serial.print(".");
    wifiConnCheckCount++;
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
*/