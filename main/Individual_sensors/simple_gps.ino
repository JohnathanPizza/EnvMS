#include <SoftwareSerial.h>
#define tx_pin 4
#define rx_pin 3
SoftwareSerial ss(4,3);
void setup() {
  Serial.begin(9600);
  ss.begin(9600);
  Serial.println("Start:");
}

void loop() {
  while(ss.available()){
    byte data = ss.read();
    Serial.write(data);    
  }
  Serial.println("");
  delay(10000);
}

