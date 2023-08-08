#include <ESP32Time.h>

const char* ntpServer = "172.16.1.3";
const long  gmtOffset_sec = 1 * 3600L;
const int   daylightOffset_sec = 1 * 3600L;

ESP32Time RTC;

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
}

bool rtc_adjust() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    if (mktime(&timeinfo)-RTC.getLocalEpoch()>RTCoffset || RTC.getLocalEpoch()-mktime(&timeinfo)>RTCoffset || RTC.getEpoch()<1000) {
      RTC.setTime(mktime(&timeinfo));
      return true;
    }
  }
  return false;
}

String rtc_time(){
  return RTC.getTime();
}

String rtc_date(){
  String rt_tmp;
  struct tm timeinfo = RTC.getTimeStruct();
  rt_tmp = String(print2digits(timeinfo.tm_mday));
  rt_tmp += "/";
  rt_tmp += String(print2digits(timeinfo.tm_mon+1));
  rt_tmp += "/";
  rt_tmp += String(print2digits(timeinfo.tm_year+1900));
  return rt_tmp;
}
