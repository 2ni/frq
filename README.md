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
  float f = get_frequ();
  DF("frequency on GPIO12/D7: %d.%01dkHz\n", (int)f, (int)(f*10)%10);
  delay(1000);
}
```
