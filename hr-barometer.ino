
#include <MS5611.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "credentials.h"

// #define WIFI_SSID           ""
// #define WIFI_PASSWORD       ""
// #define MQTT_SERVER         "mosquitto"
// #define MQTT_CLIENT_NAME    "hr-barometer"
// #define MQTT_TOPIC_TEMP     "hr-barometer/pressure"
// #define MQTT_TOPIC_IPADDR   "hr-barometer/ipaddr"
// #define MQTT_TOPIC_MACADDR  "hr-barometer/macaddr"


#define PUBLISH_RATE        2         // publishing rate in seconds

#define DEBUG               true      // debug to serial port


// --- LIBRARIES INIT ---
WiFiClient    wifi;
PubSubClient  mqtt(MQTT_SERVER, 1883, wifi);
MS5611 MS5611(0x77);


// --- SETUP ---
void setup()
{
  Serial.begin(115200);
  while(!Serial);

  // MS5611
  Serial.println();
  Serial.println(__FILE__);
  Serial.print("MS5611_LIB_VERSION: ");
  Serial.println(MS5611_LIB_VERSION);

  if (MS5611.begin() == true)
  {
    Serial.println("MS5611 found.");
    Serial.println(MS5611.getDeviceID(), HEX);
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    Serial.println("MS5611 not found. halt.");
    while (1);
  }

  MS5611.setOversampling(OSR_HIGH);

  //WIFI
  WiFi.hostname(MQTT_CLIENT_NAME);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.println("connecting Wifi");
    delay(500);
  }

  // mqtt ip/mac
  while ( !mqtt.connected() ) {
    Serial.println("connecting MQTT");
    if ( !mqtt.connect(MQTT_CLIENT_NAME) ) delay(500);
  }
  mqtt.publish(MQTT_TOPIC_IPADDR, WiFi.localIP().toString().c_str(), true);
  mqtt.publish(MQTT_TOPIC_MACADDR, WiFi.macAddress().c_str(), true);

  Serial.println();
}


void loop()
{
  MS5611.read();           // note no error checking => "optimistic".
  float temperature = MS5611.getTemperature();
  float pressure = MS5611.getPressure();

  if(DEBUG) {
    Serial.print("T:\t");
    Serial.print(temperature, 2);
    Serial.print("\tP:\t");
    Serial.print(pressure, 2);
    Serial.println();
  }
  
  // mqtt (re)connexion
  while ( !mqtt.connected() ) {
    Serial.println("connecting MQTT");
    if ( !mqtt.connect(MQTT_CLIENT_NAME) ) delay(500);
  }

  // publish to mqtt
  int result = mqtt.publish(MQTT_TOPIC_TEMP,  String(pressure).c_str(), true);
  if(DEBUG) {
    Serial.print("mqtt publish result ");
    Serial.println(result);
  }

  // delay
  if (DEBUG) {
    Serial.print("waiting ");
    Serial.print(PUBLISH_RATE);
    Serial.print(" s");
  }
  delay(PUBLISH_RATE * 1e3);
}


// -- END OF FILE --
