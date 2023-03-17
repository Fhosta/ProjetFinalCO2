#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_CCS811.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>


// Variable
const char* ssid = "EcoleDuWeb5.0g";
const char* password = "EcoleDuWEB";
const char MQTT_SERVER[] = "172.16.5.103";
const uint16_t MQTT_PORT = 1883;
const char MQTT_CLIENT[] = "ESP32_MQTT";\
const char TOPIC_CO2[] = "CO2";
const char TOPIC_TVOC[] = "TVOC";



WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_CCS811 ccs;



void setup_wifi() 
{
  delay(500);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void messageHandler(char *topic, uint8_t *payload, unsigned int len)
{
  Serial.print(topic);
  Serial.print(" with value: ");
  for (uint16_t i = 0; i < len; ++i)
    Serial.print((char)payload[i]);
  Serial.println();
}

void mqttPubSub(void)
{
  WiFiClient client;
  PubSubClient mqtt(client);

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(messageHandler);

   while (1)
  {
    if (!mqtt.connected())
    {
      Serial.print("Connecting to MQTT broker");
      Serial.print(MQTT_SERVER);
      Serial.println("...");
      if (mqtt.connect(MQTT_CLIENT))
      {
        Serial.println("Connected to MQTT broker");
      }
      else
      {
        Serial.println("Failed to connect to MQTT!");
        delay(1000);
      }
    }
    else
    {
      // Regarde si les données sont présente 
      if(ccs.available()) 
      {
        if(!ccs.readData())
        {
          // Lecture des donées du senseur de CO2
          float eCO2 = ccs.geteCO2();
          float TVOC = ccs.getTVOC();

          // Affiche les données
          Serial.print("eCO2: ");
          Serial.print(eCO2);
          Serial.print(" ppm, TVOC: ");
          Serial.println(TVOC);
          mqtt.loop();
          if (!mqtt.publish(TOPIC_CO2,String(eCO2).c_str()))
            {
              Serial.println("Error publishing Co2");
            }
          if (!mqtt.publish(TOPIC_TVOC,String(TVOC).c_str()))
            {
              Serial.println("Error publishing TVOC");
            }
        }
        delay(5000);
      }
    } 
  }
}
void reconnect() 
{
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client"))
    {
      Serial.println("connected");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() 
{
  Serial.begin(9600);
  setup_wifi();

  if(!ccs.begin())
  {
    Serial.println("CCS811 not found!");
    while(1);
   }

  // Attente avant que le capteur se lance 
  delay(100);
}

void loop() 
{
  mqttPubSub();
}