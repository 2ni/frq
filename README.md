# Platformio library to measure frequency
This is a c library to measure frequencies up to ~280kHz with the ESP8266 implemented in native espressif code

# Usage
It can be used as follows in your sketch (main.cpp):
```
#define D(x) Serial.print(x)
#define DL(x) {Serial.println(x); /*client.publish(MQTT_TOPIC, "message");*/}
#define DF(...) Serial.printf(__VA_ARGS__);

#include <Arduino.h>
#include <frq.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  DF("\n\nBooting.\nSimple frequency tester on GPIO12/D7\n\n");
}

void loop() {
  float value;
  int ready;
  char cv[5];

  get_frequ(12, &value, &ready);
  while(!ready) { yield(); }
  if(value>=0) {
    dtostrf(value, 2, 1, cv);
    DF("frequency on GPIO12/D7: %skHz\n", cv);
  }

  delay(1000);
}
```
