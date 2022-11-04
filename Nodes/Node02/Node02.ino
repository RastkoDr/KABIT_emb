#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <string.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <WiFiManager.h>

#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN 14
#define DHTPIN1 12

#define PUMPPIN 13

#define TEMPTIME 10000


/*
  led-stepenice LEDPIN2 D8
  led-soba1 LEDPIN D4
  led-soba2 LEDPIN1 D3
*/

#define LEDPIN 0
#define LEDPIN1 2
#define LEDPIN2 15

DHT dht(DHTPIN, DHTTYPE);
DHT dht1(DHTPIN1, DHTTYPE);
unsigned long dhtTimer = millis();

int lastTemp = 0;
int lastSet = 0;

char* mqttServer = "10.6.60.51";
int mqttPort = 1883;
char* mqttUser = "";
char* mqttPassword = "";

bool forceConfig = false;

WiFiClient espClient;
PubSubClient client(espClient);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {

  Serial.begin(115200);
 
  WiFiManager wifiManager;
  
  dht.begin();
  dht1.begin();

  pinMode(PUMPPIN, OUTPUT);

  WiFiManagerParameter mqttserver_text_box("key_text", "MQTT Broker Adresa", mqttServer, 50);

  wifiManager.addParameter(&mqttserver_text_box);
 
  if (!wifiManager.autoConnect("Esp-settup", "configure445"))
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

    if (client.connect("ESP8266Client1", mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
      
  client.subscribe("esp8266");
  client.subscribe("led-stepenice");
  client.subscribe("led-soba1");  
  client.subscribe("led-soba2");

  client.subscribe("set-temp2");
  
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  
  lcd.setCursor(2,0);   
  lcd.print("KABIT v0.2");
  
  lcd.setCursor(2,1);
  lcd.print("Node01");

}

void callback(char* topic, byte* payload, unsigned int length) {
  char s_payload[64] = "";
  
  for (int i = 0; i < length; i++) 
  {
      s_payload[i] = ((char)payload[i]);
  }
  
  
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  Serial.println(s_payload);
  
  Serial.println();
  Serial.println("-----------------------");

  if(strcmp(topic, "led-stepenice") == 0)
  {
    int i_payload = atoi(s_payload);
    analogWrite(LEDPIN2, map(i_payload, 0, 100, 0, 255));
    Serial.print("Set lights to: ");
    Serial.println(i_payload);
  }
  else if(strcmp(topic, "led-soba2") == 0)
  {
    int i_payload = atoi(s_payload);
    analogWrite(LEDPIN1, map(i_payload, 0, 100, 0, 255));
    Serial.print("Set lights to: ");
    Serial.println(i_payload);
  }
  else if(strcmp(topic, "led-soba1") == 0)
  {
    int i_payload = atoi(s_payload);
    analogWrite(LEDPIN, map(i_payload, 0, 100, 0, 255));
    Serial.print("Set lights to: ");
    Serial.println(i_payload);
  }
  else if(strcmp(topic, "set-temp2") == 0)
  {
    int i_payload = atoi(s_payload);
    lastSet = i_payload;
    Serial.print("Set temp to: ");
    Serial.println(i_payload);
  }
 
}

void loop() {
  if(millis() - dhtTimer > TEMPTIME)
  {
    float t = dht.readTemperature();
    char s_temp[64] = "";
    itoa(t, s_temp, 10);
    lastTemp = (int)t;
    if(t != 2147483647)
    {
      client.publish("dht-sprat1", s_temp);
      //Serial.print("published dht-sprat1: ");
      //Serial.println(t);
    }
    Serial.println(s_temp);
    if(lastTemp < lastSet)
    {
      digitalWrite(PUMPPIN, HIGH);
    }
    else
    {
      digitalWrite(PUMPPIN, LOW);
    }
    
    t = dht1.readTemperature();
    char s_temp1[64] = "";    
    itoa(t, s_temp1, 10);
    if(t != 2147483647)
    {
      client.publish("dht-spolja", s_temp1);
      //Serial.print("published dht-spolja: ");
      //Serial.println(t);
    }
    //Serial.println(s_temp1);
    client.publish("status-check", "1");
    dhtTimer = millis();
  }
  client.loop();
}
