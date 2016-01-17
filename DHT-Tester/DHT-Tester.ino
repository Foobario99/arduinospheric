// Include required libraries
#include <SPI.h>
#include <string.h>
#include "DHT.h"
 
// DHT11 sensor pins
#define DHTPIN 5 
#define DHTTYPE DHT11
 
// DHT instance
DHT dht(DHTPIN, DHTTYPE);
                                         
void setup(void)
{
 
  // Initialize DHT sensor
  dht.begin();
  
  Serial.begin(115200);
  
}
  
void loop(void)
{
  
    // Measure the humidity & temperature
    float h = dht.readHumidity();
    float t = dht.readTemperature();
   
    // Transform to String
    String temp = String((int) t);
    String hum = String((int) h);
    
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);
    Serial.println("");
    delay(500);
}
