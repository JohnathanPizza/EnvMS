#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>


// The TinyGPSPlus object
TinyGPSPlus gps;
SoftwareSerial ss(4,3);

void setup()
{
  Serial.begin(9600);
  ss.begin(9600);
  Serial.println("Start");
}

void loop()
{
  delay(10000);
  
  while(ss.available() > 0){
    byte data = ss.read();
    gps.encode(data);
    Serial.write(data);
  }

  if(gps.location.isValid()){
    Serial.println(gps.location.lat()); // Latitude in degrees (double)
    Serial.println(gps.location.lng()); // Longitude in degrees (double)
  }
  
}

void Time(){
  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
}
