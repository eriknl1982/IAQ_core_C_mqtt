#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "brzo_i2c.h"

#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "YOUR WIFI PASSWORD"

#define mqtt_server "YOUR HASS IP OR MQTT SERVER IP"
#define mqtt_user "homeassistant"
#define mqtt_password "YOUR HASS/MQTT PASSWORD"

#define co2_topic "yourtopic/co2"
#define tvoc_topic "yourtopic/tvoc"

uint8_t SDA_PIN = 4;
uint8_t SCL_PIN = 5;
uint8_t iaq_adr = 0x5A; //default IAQ core-c address

uint8_t buffer[10];

uint8_t error = 0;

uint16_t co2;
uint16_t tvoc;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);

  brzo_i2c_setup(SDA_PIN, SCL_PIN, 3000);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
 }

void setup_wifi() {
  delay(10);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.mode(WIFI_STA); //make sure the ESP doen't become a access point
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long lastMsg = 0;

void loop() {
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 60 * 1000) {
    lastMsg = now;


  brzo_i2c_start_transaction(iaq_adr, 100);
   brzo_i2c_read(buffer, 9, true);

   if (buffer[2] == 0x10 ){
       Serial.println("Warming up...");
   } else
   {

    co2 = buffer[0]<<8 | buffer[1];
    tvoc = buffer[7]<<8 | buffer[8];

    Serial.print("CO2 : \t"); Serial.print(co2); 
    Serial.print("TVOC : \t"); Serial.println(tvoc);

    client.publish(co2_topic, String(co2).c_str(), true);
    client.publish(tvoc_topic, String(tvoc).c_str(), true);
    
   }

    
  }
}