#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"  
 DHT dht2(2,DHT11); 

#define DHT11_PIN 2

int counter = 0;
#define ss 4    
#define rst  5 
#define dio0 -1 



void setup() {
  dht2.begin();
  Serial.begin(115200);
  while (!Serial);
  delay(1000);

  Serial.println("LoRa Sender");
  LoRa.setPins(ss, rst, dio0);
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    delay(5000);
    Serial.println("Now waiting for Watchdog to restart");
    while (1);
  }else{
    Serial.println("Starting LoRa successful");
    }
  
}

// Sends a string every 5000ms (5 seconds)
void loop() {

  
  Serial.print("Temperature = ");
  Serial.println(dht2.readTemperature());
  // send packet
  LoRa.beginPacket();
  
  LoRa.print(int(dht2.readTemperature()));

  LoRa.endPacket();

  delay(5000);

  LoRa.beginPacket();
  
  LoRa.print(int(dht2.readHumidity()));

  LoRa.endPacket();

  delay(5000);

}

