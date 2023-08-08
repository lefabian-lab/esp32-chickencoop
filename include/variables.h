#include <Arduino.h>

const boolean DEBUG = false;

void warning( const char* msg );
void new_message();
void print_WiFi();
void initWiFi();
void reconnect_wifi();
bool AUTOMATE();
void show_relaystatus();
void show_clock();
void manual_mode();
void RELAY_ON();
void RELAY_OFF();
void AlarmSets();
void wifiCheck();

String temperature();
String humidity();

void printLocalTime();
bool rtc_adjust();
String rtc_time();
String rtc_date();


/*---------------------------*/
/* Timeout const definitions */
/*---------------------------*/

const uint32_t Timeout1s = 1000;
const uint32_t Timeout2s = 2000;
const uint32_t Timeout5s  = 5000;
const uint32_t Timeout10s = 10000;
const uint32_t Timeout15s = 15000;
const uint32_t Timeout20s = 20000;

/*---------------------------*/


/*---------------------------*/
/* LCD definition            */
/*---------------------------*/

const int lcdReg = 0x27;
const int8_t lcdRow = 2;
const int8_t lcdCol = 16;

/*---------------------------*/

boolean MESSAGE = false;
boolean openMessage = false;
boolean closeMessage = false;

unsigned long LCDpreviousMillis = 0;
unsigned long previousMillis = 0;
unsigned long WiFipreviousMillis = 0;
unsigned long lcdcurrentMillis = 0;
 
/*------------------------------*/
/* Button and relay definitions */
/*------------------------------*/

#define BUTTON_PIN1 19  // ESP32 pin GPIO25 Automation button 16
#define BUTTON_PIN2 23  // ESP32 pin GPIO26 Manual door open/close 17
#define BUTTON_PIN3 18  // ESP32 pin GPIO32 Light on/off 18

#define RELAY_PIN1  25  // ESP32 pin GPIO16 connected to relay's pin (Automation control) 25
#define RELAY_PIN2  26  // ESP32 pin GPIO17 connected to relay's pin (Door control) 26
#define RELAY_PIN3  32  // ESP32 pin GPIO18 connected to relay's pin (Light control) 32

// variables will change:
int relay_state1 = LOW;   // the current statne of AUTOMATION realy
int relay_state2 = HIGH;   // the current state of DOOR relay
int relay_state3 = HIGH;   // the current state of LIGHT relay
/*------------------------------*/

/*---------------------------------*/
/* Power monitor variables         */
/*---------------------------------*/
#define POWER_PIN 33

/*---------------------------------*/


/*---------------------------------*/
/* Web server variable definitions */
/*---------------------------------*/

const char* PARAM_INPUT_1 = "output";
const char* PARAM_INPUT_2 = "state";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Chicken Land Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <style>
    html {font-family: Arial, Helvetica, sans-serif; display: inline-block;text-align: center}
    h1 {font-size: 1.8rem; color: white}
    h3 {font-size: 0.8rem; color: white}
    p.RTCDateTime {font-size: 0.8rem; color: white}
    p {font-size: 1.4rem}
    .topnav { overflow: hidden; background-color: #0A1128}
    body { text-align: center;text-align: -webkit-center; max-width: 400px}
    .content { padding: 5px}
    .card-grid { max-width: 400px;margin: 10 auto;display: grid; grid-gap: 2rem; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr))}
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5)}
    .card-title { font-size: 1.2rem;font-weight: bold;color: #034078}
    .switch {position: relative; display: inline-block; width: 100px; height: 48px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
    .slider:before {position: absolute;content: "";height: 28px;width: 33px;left: 8px;bottom: 10px;background-color: #fff;-webkit-transition: .4s;transition: 0.4s;border-radius: 5px}
    input:checked+.slider {background-color: #b30000}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
  else { xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
  xhr.send();
}
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var string = this.responseText;
      var array = string.split(";");
      document.getElementById("RTCDateTime").innerText = array[0];
      document.getElementById("humidity").innerText = array[1];
      document.getElementById("temperature").innerText = array[2];
      document.getElementById("powbox").innerText = array[3]+"V";
      document.getElementById("25").checked = ((array[4]=="0") ? true : false);
      document.getElementById("26").checked = ((array[5]=="0") ? true : false);
      document.getElementById("32").checked = ((array[6]=="0") ? true : false);
    }
  };
  xhttp.open("GET", "/status", true);
  xhttp.send();
}, 1000 );

</script>
</head>
<body>
  <div class="topnav">
    <h1>Chicken Land</h1>
    <h3><p id="RTCDateTime" class="RTCDateTime"></p></h3>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">
        <p class="card-title">Temperature</p>
        <p id="temperature">Connecting...</p>
        <p class="card-title">Humidity</p>
        <p id="humidity">Connecting...</p>
        <p class="card-title">Main power</p>
        <p id="powbox">Connecting...</p>
      </div>
    </div>
  </div>
  <div class="content">
    <div class="card-grid">
      <div class="card">
  %BUTTONPLACEHOLDER%

    </div>
  </div>
</body>
</html>
)rawliteral";

String outputState(int output){
  if(digitalRead(output) == LOW){
    return "checked";
  }
  else {
    return "";
  }
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    buttons += "<p class=\"card-title\">Automation</p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"25\" " + outputState(25) + "><span class=\"slider\"></span></label>";
    buttons += "<p class=\"card-title\">Door</p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"26\" " + outputState(26) + "><span class=\"slider\"></span></label>";
    buttons += "<p class=\"card-title\">Light</p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"32\" " + outputState(32) + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}
/*---------------------------------*/

/*-----------------*/
/* RTC definitions */
/*-----------------*/

unsigned long RTCunpowered = 0;
unsigned long RTCoffset = 1800; // 30 minutes offset for time resync

const char *monthName[12] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

struct tm t = {0};

String print2digits(int number) {
  if (number >= 0 && number < 10) {
    return "0"+String(number);
  }
  return String(number);
}
