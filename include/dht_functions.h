#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 27     
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

String temperature(bool mode) {
  float t = dht.readTemperature();
  if (mode) {
    return (String(t));
  } else {
    return (String(t)+"°C");
  }
}

String humidity(bool mode) {
  float h = dht.readHumidity();
  if (mode) {
    return (String(h));
  } else {
    return (String(h)+"%");
  }
}