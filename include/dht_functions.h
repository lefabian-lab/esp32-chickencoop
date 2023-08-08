#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 27     
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

String temperature() {
  float t = dht.readTemperature();
  return (String(t)+"°C ");
}

String humidity() {
  float h = dht.readHumidity();
  return (String(h)+"%");
}