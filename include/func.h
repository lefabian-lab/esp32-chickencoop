
void warning( const char* msg ) {
  LCD.backlight();
  MESSAGE = true;
  LCDpreviousMillis = millis();
  LCD.setCursor(0, 0);
  LCD.print("*** WARNING *** ");
  LCD.setCursor(0, 1);
  LCD.print(msg);
  lcdcurrentMillis = millis();
}

void new_message() {
  MESSAGE = true;
  lcdcurrentMillis = millis();
}

bool AUTOMATE() {
  if (digitalRead(RELAY_PIN1) == LOW) {
    return true;
  } else {
    return false;
  }
}

void show_relaystatus() {
  if (DEBUG) {
    Serial.print("relay_state1: ");
    Serial.println(relay_state1);
    Serial.print("relay_state2: ");
    Serial.println(relay_state2);
    Serial.print("relay_state3: ");
    Serial.println(relay_state3);
  }
}

void show_clock() {
  int power=0;
  if (DEBUG) {
    Serial.print("Time: ");
    Serial.println(rtc_time());
    Serial.print("Date: ");
    Serial.println(rtc_date());
    Serial.print("Analog33: ");
    power=analogRead(POWER_PIN);
    Serial.println(power*0.00712);
  }
  if (! MESSAGE) {
    LCD.setCursor(0, 0);
    LCD.print("Time:   ");
    LCD.print(rtc_time());

    LCD.setCursor(0, 1);
    LCD.print("Date: ");
    LCD.print(rtc_date());
  }
}

void manual_mode() {
  if (DEBUG) {
    Serial.println("Manual mode...");
  }

  LCD.setCursor(0, 0);
  LCD.print("*** WARNING *** ");
  LCD.setCursor(0, 1);
  LCD.print("  Manual mode!  ");

  show_relaystatus();
}

void RELAY_ON() {
  if (AUTOMATE()) {
    relay_state2 = HIGH;
    digitalWrite(RELAY_PIN2, relay_state2);
  }
}

void RELAY_OFF() {
  if (AUTOMATE()) {
    relay_state2 = LOW;
    digitalWrite(RELAY_PIN2, relay_state2);
  }
}

void AlarmSets() {
  t.tm_year = RTC.getYear()-1900;
  t.tm_mon = RTC.getMonth();
  t.tm_mday = RTC.getDay();

  t.tm_hour = 5;
  t.tm_min = 0;
  t.tm_sec = 0;
  time_t timeSinceEpoch_open = mktime(&t); // Open

  t.tm_hour = 21;
  t.tm_min = 0;
  t.tm_sec = 0;
  time_t timeSinceEpoch_close = mktime(&t); // Close

  if (RTC.getEpoch() >= timeSinceEpoch_open && RTC.getEpoch() <= timeSinceEpoch_close) {
    RELAY_ON(); // OPEN DOOR
    if ( openMessage == false ) {
      warning("Door openning...");
      openMessage = true;
      closeMessage = false;
      MESSAGE=true;
    }
  } else {
    RELAY_OFF(); // CLOSE DOOR
    if ( closeMessage == false ) {
      warning("Door closing ...");
      openMessage = false;
      closeMessage = true;
      MESSAGE=true;
    }  
  }
}

void wifiCheck() {
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() == WL_CONNECTED) &&  (WiFi_statuschanged == false)) {
    print_WiFi();
    rtc_adjust();
  }

  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - WiFipreviousMillis >= 300000)) {
    WiFipreviousMillis = currentMillis;
    WiFi_statuschanged = false;
    reconnect_wifi();
  }
}

void print_WiFi() {
  WiFi_statuschanged = true;
  if (DEBUG) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println(WiFi.SSID());
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  LCD.setCursor(0, 0);
  LCD.print("SSID: ");
  LCD.println(WiFi.SSID());
  LCD.setCursor(0, 1);
  LCD.print("IP: ");
  LCD.println(WiFi.localIP());
  
  new_message();
}

void initWiFi() {
  // Connect to Wi-Fi

  WiFi.mode(WIFI_STA);

  wifiMulti.addAP("nardoS20", "vnun2406");
  wifiMulti.addAP("charlie", "Alaska202011");
  wifiMulti.addAP("charlie_Wi-Fi5", "Alaska202011");

  Serial.println("");
  Serial.println("Connecting WiFi...");

  LCD.setCursor(0, 0);
  LCD.println("Network connect");
  LCD.setCursor(0, 1);
  LCD.println("connecting...");
  if(wifiMulti.run() == WL_CONNECTED) {
    print_WiFi();
  } else {
    LCD.setCursor(0, 1);
    LCD.println("Connection error");
    new_message();
  }
}

void reconnect_wifi() {
  if (wifiMulti.run(Timeout10s) == WL_CONNECTED) {
    print_WiFi();
  }
}