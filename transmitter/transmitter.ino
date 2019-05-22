#include "DHT.h"
#include <SPI.h>  
#include "RF24.h"

#define DHTPIN 4  
#define DHTTYPE DHT22 
#define led_pin 13

struct package
{
  float temperature ;
  float humidity ;
} data;

RF24 myRadio (7, 8);
DHT dht(DHTPIN, DHTTYPE);
const byte address[6] = "00001";


void setup()
{
    Serial.begin(9600);
    pinMode(led_pin, OUTPUT);
    dht.begin();
    myRadio.begin();  
    myRadio.setChannel(115); 
    myRadio.setPALevel(RF24_PA_MAX);
    myRadio.setDataRate(RF24_250KBPS);
    myRadio.openWritingPipe(address);
    myRadio.stopListening();
    delay(1000);
}

void loop()
{
  digitalWrite(led_pin, LOW);  // Flash a light to show transmitting
  data.humidity = dht.readHumidity();
  data.temperature = dht.readTemperature();
  Serial.println(data.temperature);
  Serial.println(data.humidity);
  myRadio.write(&data, sizeof(data)); 
  digitalWrite(led_pin, HIGH);
  delay(1000);
}
