#include <SPI.h>
#include <string.h>
#include "DHT.h"
#include <ESP8266WiFi.h>
#include <Esp.h>

//ESP needs to reconfigure the ADC at startup in order for this feature to be available. 
ADC_MODE(ADC_VCC);

// DHT11 sensor pins
#define DHTPIN 4 
#define DHTTYPE DHT11

const bool reportVCC = false;

// DHT instance
DHT dht(DHTPIN, DHTTYPE);

const char* ssid     = "Tundra";
const char* password = "*******";

const char* apikey = "ZGL7TFM1VVCTEM3S"; 
const char* host = "api.thingspeak.com";
String tempFieldName;
String humidityFieldName;

const int loopDelayMillis = 300000; // 5 minutes
//const int loopDelayMillis = 10000;
void loopExec(void);
void initWifi();

void flashRed(int length);
void flashBlue(int length);

void setup() {
  Serial.begin(115200);
//  Serial.begin(74880);
  delay(1000);
  Serial.println("We are awake!");

  initWifi();

  //It appears that for up to 1second after power-on, the DHT sensor
  //is in a strange state, so we need to wait before sending it any commands.
  delay(1000);
  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  
  loopExec();
  
  Serial.println("Time to sleep");
  ESP.deepSleep(loopDelayMillis*1000);
  delay(1000);
}

void initWifi() {
   String macAddress;

  // We start by connecting to a WiFi network
 
  Serial.println();
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
  Serial.print("Mac address: ");
  macAddress = WiFi.macAddress();
  Serial.println(macAddress);
  
  //Use our mac address to tell which sensor we are

  if ( macAddress == "18:FE:34:DB:55:54" ) {
    //Running on Livingroom monitor
    Serial.println("Running on Livingroom node");
    tempFieldName = "field1";
    humidityFieldName = "field2";    
  }
  else if ( macAddress == "5C:CF:7F:07:5E:5B" ) {
    //Running on AE Bedroom monitor
    Serial.println("Running on AE Bedroom node");
    tempFieldName = "field3";
    humidityFieldName = "field4";    
  }else if ( macAddress == "5C:CF:7F:01:59:7F" ) {
    //Running on Tesla Bedroom monitor
    Serial.println("Running on Tesla Bedroom node");
    tempFieldName = "field5";
    humidityFieldName = "field6";    
  }else if ( macAddress == "5C:CF:7F:02:0A:11" ) {
    //Running on Peej Bedroom monitor
    Serial.println("Running on Peej Bedroom node");
    tempFieldName = "field7";
    humidityFieldName = "field8";    
  }else {
    Serial.println("Node type unknown!");
  }
  

}



void thingsSpeakSubmit(float temp, float humidity) { 
  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/update?key=";
  url += apikey;
  url += "&";
  url += tempFieldName;
  url += "=";
  url += String(temp);
  url += "&";
  url += humidityFieldName;
  url += "=";
  url += String(humidity);

  if ( reportVCC ) {
    float vcc = ESP.getVcc() / 1000.0; //report in volts
    url += "&";
    url += "field3";
    url += "=";
    url += String(vcc);
  }    
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(500);
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
}

void loopExec(void)
{
    // Measure the humidity & temperature
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if ( t == NAN ) {
      t = -1.0;
    }

    if ( h == NAN ) {
      h = -1.0;
    }
    thingsSpeakSubmit(dht.convertCtoF(t),h);      

}


void flashRed(int length) {
    digitalWrite(0, LOW);//For some reason LOW means "on" :/
    delay(length);
    digitalWrite(0, HIGH);
}
void flashBlue(int length) {
    digitalWrite(2, LOW);//For some reason LOW means "on" :/
    delay(length);
    digitalWrite(2, HIGH);
}



