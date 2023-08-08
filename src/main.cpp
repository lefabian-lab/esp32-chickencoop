#include <Arduino.h>
#include <variables.h>
#include <wifi_functions.h>
#include <ntp_functions.h>

#include <ezButton.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Wire.h>  
#include <SPI.h> 

#include <LiquidCrystal_I2C.h>
#include <dht_functions.h>

#include <AsyncTCP.h>
#include <AsyncElegantOTA.h>

ezButton button1(BUTTON_PIN1);  // create ezButton object that attach to pin 19;
ezButton button2(BUTTON_PIN2);  // create ezButton object that attach to pin 23;
ezButton button3(BUTTON_PIN3);  // create ezButton object that attach to pin 18;

LiquidCrystal_I2C LCD = LiquidCrystal_I2C(lcdReg, lcdCol, lcdRow);

AsyncWebServer server(80);
AsyncWebServer update_server(8080);

#include <func.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
  LCD.init();

  dht.begin();

  pinMode(RELAY_PIN1, OUTPUT); // set ESP32 pin to output mode
  pinMode(RELAY_PIN2, OUTPUT); // set ESP32 pin to output mode
  pinMode(RELAY_PIN3, OUTPUT); // set ESP32 pin to output mode 

  pinMode(POWER_PIN, INPUT_PULLUP);
  pinMode(DHTPIN,OUTPUT);

  button1.setDebounceTime(50); // set debounce time to 50 milliseconds
  button2.setDebounceTime(50); // set debounce time to 50 milliseconds
  button3.setDebounceTime(50); // set debounce time to 50 milliseconds

  digitalWrite(RELAY_PIN1, relay_state1);
  digitalWrite(RELAY_PIN2, relay_state2);
  digitalWrite(RELAY_PIN3, relay_state3);

  initWiFi();
    
  LCD.setCursor(0, 0);
  LCD.println("RTC check");
  if (rtc_adjust()) {
    LCD.setCursor(0, 1);
    LCD.println("Synced...");
    new_message();
    Serial.println("Time synced ...");
  } else {
    LCD.setCursor(0, 1);
    LCD.println("RTC error!");
    new_message();
    Serial.println("Missing RTC module");
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage1;
    String inputMessage2;
    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      digitalWrite(inputMessage1.toInt(), inputMessage2.toInt());
      if (inputMessage1.toInt() == RELAY_PIN1) {
        relay_state1 = !relay_state1;
      }
      if (inputMessage1.toInt() == RELAY_PIN2) {
        relay_state2 = !relay_state2;
      }
      if (inputMessage1.toInt() == RELAY_PIN3) {
        relay_state3 = !relay_state3;
      }
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/status", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String Response;
    Response += RTC.getTimeDate(false).c_str();
    Response += ";";
    Response += humidity(false).c_str();
    Response += ";";
    Response += temperature(false).c_str();
    Response += ";";
    Response += String(analogRead(POWER_PIN)*0.00712).c_str();
    Response += ";";
    Response += String(digitalRead(RELAY_PIN1)).c_str();
    Response += ";";
    Response += String(digitalRead(RELAY_PIN2)).c_str();
    Response += ";";
    Response += String(digitalRead(RELAY_PIN3)).c_str();
    request->send(200, "text/plain", Response.c_str());
  });

  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    uint8_t Element;
    if (request->hasParam(PARAM_INPUT_1)) {
      Element = request->getParam(PARAM_INPUT_1)->value().toInt();
    } else {
      Element = RELAY_PIN1;
    }
    request->send(200, "text/plain", String(digitalRead(Element)).c_str());
  });

  server.on("/power", HTTP_GET, [] (AsyncWebServerRequest *request) {
    float Power;
    Power=(analogRead(POWER_PIN)*0.00712);
    request->send(200, "text/plain", String(Power).c_str());
  });

  server.on("/temperature", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", temperature(true).c_str());
  });

  server.on("/humidity", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", humidity(true).c_str());
  });

  server.on("/RTCDateTime", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", RTC.getTimeDate(false).c_str());
  });

  // Start server
  server.begin();

  update_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "OTA update server of Chicken land ...");
  });

  AsyncElegantOTA.begin(&update_server);    // Start ElegantOTA
  update_server.begin();
  
}

void loop() {

  unsigned long currentMillis = millis();

  button1.loop(); // MUST call the loop() function first
  button2.loop(); // MUST call the loop() function first
  button3.loop(); // MUST call the loop() function first

  if (button1.isPressed()) {
    relay_state1 = !relay_state1;
    digitalWrite(RELAY_PIN1, relay_state1);
  }
  if (button2.isPressed()) {
    relay_state2 = !relay_state2;
    digitalWrite(RELAY_PIN2, relay_state2);
  }
  if (button3.isPressed()) {
    relay_state3 = !relay_state3;
    digitalWrite(RELAY_PIN3, relay_state3);
  }

  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    if (AUTOMATE()) {
      if (! MESSAGE) {
        show_clock();
        show_relaystatus();
      }
    } else {
      if (! MESSAGE) {
        manual_mode();
      }
    }
  }
 
// show last message for 10s
  if ((currentMillis - LCDpreviousMillis >= Timeout10s) && (MESSAGE == true)) {
    LCDpreviousMillis = currentMillis;
    MESSAGE = false;
  }

// enable backlight for LCD for 20s
  if (currentMillis - lcdcurrentMillis >= Timeout20s) {
    lcdcurrentMillis = currentMillis;
    LCD.noBacklight();
  }

  // if WiFi is down, try to reconnect
  if ((WiFi.status() == WL_CONNECTED) &&  (WiFi_statuschanged == false)) {
    print_WiFi();
    rtc_adjust();
  }

  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - WiFipreviousMillis >= 300000)) {
    WiFipreviousMillis = currentMillis;
    WiFi_statuschanged = false;
    reconnect_wifi();
  }

  AlarmSets();
}
