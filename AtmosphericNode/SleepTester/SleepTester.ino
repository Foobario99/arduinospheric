#include <string.h>
#include <Esp.h>
const int loopDelayMillis = 10000;


void setup() {
  Serial.begin(74880);
  delay(100);
  Serial.println("We are awake!");
  
}

void loop() {
  Serial.println("Hello world!");
  for (int i=10; i > 0; i--){
    Serial.println(i);
    delay(1000);
  }

  Serial.println("Time to sleep");
  ESP.deepSleep(loopDelayMillis*1000, WAKE_RF_DISABLED);
  delay(1000);
}
