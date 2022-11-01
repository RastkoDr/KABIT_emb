#include <string.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <LoRa.h>
#include <WiFiManager.h>

#include <DHT.h>

#define DHTTYPE DHT11


/*
  led-hodnik LEDPIN2 D8
  led-dnevna LEDPIN D4
  led-kupatilo LEDPIN1 D3
*/
#define ss 5
#define rst 14
#define dio0 2


char* mqttServer = "10.6.60.51";
int mqttPort = 1883;
char* mqttUser = "";
char* mqttPassword = "";

bool forceConfig = false;

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {

  Serial.begin(115200);

  while (!Serial);

  Serial.println("LoRa Receiver");
  LoRa.setPins(ss, rst, dio0);
     if (!LoRa.begin(868E6)) {
      Serial.println("Starting LoRa failed!");
      delay(5000);
      Serial.println("Now waiting for Watchdog to restart");
      while (1);
    }else{
      Serial.println("Starting LoRa successful");
      }
  WiFiManager wifiManager;


  WiFiManagerParameter mqttserver_text_box("key_text", "MQTT Broker Adresa", mqttServer, 50);

  wifiManager.addParameter(&mqttserver_text_box);
  
  if (!wifiManager.autoConnect("Esp-settup1", "configure445"))
  {
      Serial.println("connection failed");
      delay(3000);
      ESP.restart();
      delay(5000);
  }
  
  strcpy(mqttServer, mqttserver_text_box.getValue());
  Serial.println(mqttServer);
  Serial.println(mqttserver_text_box.getValue());

 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP8266Client2", mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
      
  client.subscribe("esp8266");

  
}

void callback(char* topic, byte* payload, unsigned int length) {
  
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) { 
    Serial.print("Received packet '");

    char message[64] = "";
    while (LoRa.available()) {
      int temp = (int)LoRa.read();
      itoa(temp,message,10);
    
    }
    client.publish("lora-reciever-temp",message);
  }
  client.loop();
}
