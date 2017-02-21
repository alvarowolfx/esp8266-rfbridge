/*
 Attach a rf 433 receiver to port d4 on ESP8266
*/

#include <ESP8266WiFi.h>    // https://github.com/esp8266/Arduino
#include <PubSubClient.h>   // https://github.com/knolleary/pubsubclient/releases/tag/v2.6
#include <RCSwitch.h>  // https://github.com/sui77/rc-switch


#define MQTT_CLIENT_ID "rfbridge"
#define MQTT_SERVER "192.168.0.4"
#define MQTT_PORT 1883
#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASSWORD"

RCSwitch mySwitch = RCSwitch();
WiFiClient    wifiClient;
PubSubClient  mqttClient(wifiClient);

/*
 * Publish new 433 sensor data found
 */
void publishFoundValue(int value){
  char topic[50];  
  sprintf(topic, "rfbridge/value/%d", value);
  Serial.print("Publishing to ");
  Serial.println( topic );  
  mqttClient.publish(topic, "ON", true);   
}


/*
  Function called to reset the configuration of the switch
*/
void reset() {
  WiFi.disconnect();
  delay(1000);
  ESP.reset();
  delay(1000);
}

/*
  Function called to connect/reconnect to the MQTT broker
*/
void reconnect() {
  uint8_t tries = 0;
  while (!mqttClient.connected()) {
    bool ok = mqttClient.connect(MQTT_CLIENT_ID, "","");
    if(!ok){         
      Serial.println("Trying to reconnect to broker");
      delay(1000);
      if (tries == 3) {
        reset();
      }
      tries++;
    }
  }
}


void setupWifi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  reconnect();
  mySwitch.enableReceive(2);  // Receiver on interrupt 0 => that is pin #2  
}

void loop() {

  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      int value = mySwitch.getReceivedValue();
      Serial.print("Received ");
      Serial.print( value );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
      
      publishFoundValue(value);
    }

    mySwitch.resetAvailable();
  }
}
