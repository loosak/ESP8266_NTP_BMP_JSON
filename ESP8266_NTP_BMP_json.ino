#include <Adafruit_BMP085.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Time.h>
#include <WiFiUdp.h>
#include <Wire.h>

MDNSResponder mdns;

Adafruit_BMP085 bmp;

void setup() {

  Serial.begin(115200);
  Serial.println("Booting");
  Serial.println();
  Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
  Serial.printf("Free size: %u\n", ESP.getFreeSketchSpace());
  Serial.printf("Heap: %u\n", ESP.getFreeHeap());
  Serial.printf("Boot Vers: %u\n", ESP.getBootVersion());
  Serial.printf("CPU: %uMHz\n", ESP.getCpuFreqMHz());
  Serial.printf("SDK: %u\n", ESP.getSdkVersion());
  Serial.printf("Chip ID: %u\n", ESP.getChipId());
  Serial.printf("Flash ID: %u\n", ESP.getFlashChipId());
  Serial.printf("Flash Size: %u\n", ESP.getFlashChipRealSize());
  Serial.printf("Vcc: %u\n", ESP.getVcc());

  if (SPIFFS.begin()) {
    Serial.println(F("SPIFFS Mount succesfull"));
  }
  Serial.printf("\nConnecting ...");

  WiFi.begin();
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin();
    Serial.println("Retrying connection...");
    //    Serial.println("Connection Failed! Rebooting...");
    //    delay(5000);minicom
    //    ESP.restart();
  }

  Serial.printf("\nWiFi connected\nIP address: %s\n",
                WiFi.localIP().toString().c_str());
  // c_str() on the string object, which returns a pointer to a null-terminated
  // string.
  // C function does not understand C++ objects

  Wire.begin(D4, D3);
  bmp.begin();

  setupNtp();

  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  File f = SPIFFS.open("/data.json", "w");
  if (!f) {
    Serial.println(F("open for write failed"));
  }
  f.write('var jdata = [\\\n');
  f.close();
}

char tmpstr[40];

const unsigned long postRate = 10000;
unsigned long lastPost = 0;

void loop() {
  // int32_t beginWait = millis();

  if (lastPost + postRate <= millis()) {
    digitalClockDisplay();
    lastPost = millis();
  }
}
// if (millis() % 10000 == 0) {
// digitalClockDisplay(); delay(1);
// yield();
//}

// while (millis() - beginWait < 10000) {
//  yield();
// }

void digitalClockDisplay() {
  // digital clock display of the time
  static char buf[20] = {0};
  StaticJsonBuffer<200> jsonBuffer;

  sprintf(buf, "%02d.%02d.%d %02d:%02d:%02d\0", day(), month(), year(), hour(),
          minute(), second());
  JsonObject &root = jsonBuffer.createObject();
  root["time"] = buf;
  root["temp"] = bmp.readTemperature();
  root["pres"] = bmp.readPressure();
  root["alti"] = bmp.readAltitude();
  root.printTo(Serial);

  File f = SPIFFS.open("/data.json", "a");
  if (!f) {
    Serial.println(F("open for write failed"));
    return;
  }
  root.printTo(f);
  f.write(',\\\n'); // ,\\new line!!!
  f.close();

  ElapsedStr(tmpstr);
  Serial.printf(" Heap: %u %s\n", ESP.getFreeHeap(), tmpstr);
}
