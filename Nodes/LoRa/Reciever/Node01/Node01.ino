#include <string.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <WiFiManager.h>

#include <DHT.h>

#define DHTTYPE DHT11
#define DHTPIN 12

#define PUMPPIN 13

#define TEMPTIME 10000

#define RESTARTPIN 14

/*
  led-hodnik LEDPIN2 D8
  led-dnevna LEDPIN D4
  led-kupatilo LEDPIN1 D3
*/

#define LEDPIN 0
#define LEDPIN1 2
#define LEDPIN2 15

DHT dht(DHTPIN, DHTTYPE);
unsigned long dhtTimer = millis();
unsigned long resTimer = millis();

int lastTemp = 0;
int lastSet = 0;

char* mqttServer = "10.6.60.51";
int mqttPort = 1883;
char* mqttUser = "";
char* mqttPassword = "";

bool forceConfig = false;

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {

  Serial.begin(115200);
 
  WiFiManager wifiManager;
  
  dht.begin();
  
  pinMode(PUMPPIN, OUTPUT);
  pinMode(RESTARTPIN, OUTPUT);
  digitalWrite(RESTARTPIN, HIGH);

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

    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
      
  client.subscribe("esp8266");
  client.subscribe("led-hodnik");
  client.subscribe("led-dnevna");  
  client.subscribe("led-kupatilo");

  client.subscribe("set-temp1");

  client.subscribe("status-check");
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

  if(strcmp(topic, "led-hodnik") == 0)
  {
    int i_payload = atoi(s_payload);
    analogWrite(LEDPIN2, map(i_payload, 0, 100, 0, 255));
    Serial.print("Set lights to: ");
    Serial.println(i_payload);
  }
  else if(strcmp(topic, "led-dnevna") == 0)
  {
    int i_payload = atoi(s_payload);
    analogWrite(LEDPIN1, map(i_payload, 0, 100, 0, 255));
    Serial.print("Set lights to: ");
    Serial.println(i_payload);
  }
  else if(strcmp(topic, "led-kupatilo") == 0)
  {
    int i_payload = atoi(s_payload);
    analogWrite(LEDPIN, map(i_payload, 0, 100, 0, 255));
    Serial.print("Set lights to: ");
    Serial.println(i_payload);
  }
  else if(strcmp(topic, "set-temp1") == 0)
  {
    int i_payload = atoi(s_payload);
    lastSet = i_payload;
    Serial.print("Set temp to: ");
    Serial.println(i_payload);
  }
  else if(strcmp(topic, "status-check") == 0)
  {
    Serial.print("Last status");
    Serial.println(millis()-resTimer);
    resTimer = millis();
  }
 
}

void loop() {
  if(millis() - dhtTimer > TEMPTIME)
  {
    float t = dht.readTemperature();
    char s_temp[64] = "";
    lastTemp = (int)t;
    itoa(t, s_temp, 10);
    if(t != 2147483647)
    {
      client.publish("dht-prizemlje", s_temp);
      //Serial.print("published prizemlje: ");
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
    digitalWrite(RESTARTPIN, HIGH);        
    dhtTimer = millis();
  }
  if(millis() - resTimer > TEMPTIME*2)
  {
    digitalWrite(RESTARTPIN, LOW);
    resTimer = millis();
  }
  client.loop();
}
