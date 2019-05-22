#include <LCDWIKI_GUI.h>
#include <LCDWIKI_KBV.h>
#include <Sodaq_DS3231.h>
#include "DHT.h"
#include "RF24.h"

#define DHTPIN 31  
#define DHTTYPE DHT22

// DHT object initialization -- Temperature and Humidity Sensor
DHT dht(DHTPIN, DHTTYPE);

// RF24 object initialization -- Wireless Communitication Module
RF24 myRadio (34,33);
const byte address[6] = "00001";      // address string for crc

// LCDWIKI_KBV object initialization -- TFT Display Module
LCDWIKI_KBV my_lcd(ILI9486,A3,A2,A1,A0,A4); 
int16_t xmax = my_lcd.Get_Display_Height()-1;
int16_t ymax = my_lcd.Get_Display_Width()-1;

// Structure and Variable for data from Remote Location
struct package
{
  float temperature ;
  float humidity ;
} data;

float previousRemoteHumidity = 0.1;
float previousRemoteTemperature = 0.1;
float remoteHumidity = 0.0;
float remoteTemperature = 0.0;

// Variables for data from Indoor Location
float previousIndoorHumidity = 0;
float previousIndoorTemperature = 10;
float indoorHumidity = 0;
float indoorTemperature = 0;

// Variables for Clock Module
String dateString;
String hours;
int minuteNow=0;
int minutePrevious=0;

// Only for first time to setup clock time 
void setRTCTime()
{
  // Adjust date-time as defined 'dt' 
  // Year, Month, Day, Hour, Minutes, Seconds, Day of Week
  DateTime dt(2019, 4, 27, 18, 57, 15, 6); 
  rtc.setDateTime(dt); 
}

// Function to print string on display
void show_string(uint8_t *str, int16_t x, int16_t y, uint8_t csize, uint16_t fc, uint16_t bc, boolean mode)
{
    my_lcd.Set_Text_Mode(mode);
    my_lcd.Set_Text_Size(csize);
    my_lcd.Set_Text_colour(fc);
    my_lcd.Set_Text_Back_colour(bc);
    my_lcd.Print_String(str,x,y);
}

// --- Main Display ---
void UI_setup(void)
{
  my_lcd.Set_Draw_color(230, 35, 35);
  
  // header
  my_lcd.Fill_Rectangle(0, 0, xmax, 15);
  show_string("* WEATHER STATION *", CENTER, 1, 2, 0x0000, 0, 1);
  
  // footer
  my_lcd.Fill_Rectangle(0, ymax-15, xmax, ymax);
  show_string("ANURAG AND AYUSH", CENTER, ymax-14 , 2, 0x0000, 0, 1);
  
  // indoor
  my_lcd.Draw_Rectangle(30, 45, (xmax+1)/2 - 15, ymax-120);
  my_lcd.Fill_Rectangle(45, 30, (xmax+1)/2 - 30, 60);
  show_string("INDOOR", 75, 34, 3, 0x0000, 0, 1);
  show_string("Temp", 34, 65 , 4, 0xE165, 0, 1);
  show_string("Humidity", 34, 133 , 4, 0xE165, 0, 1);
  
  // outdoor
  my_lcd.Draw_Rectangle((xmax+1)/2 + 15, 45, xmax-30, ymax-120);
  my_lcd.Fill_Rectangle((xmax+1)/2 + 30, 30, xmax-45, 60);
  show_string("REMOTE", (xmax+1)/2 + 62, 34, 3, 0x0000, 0, 1);
  show_string("Temp", (xmax+1)/2 + 19, 65 , 4, 0xE165, 0, 1);
  show_string("Humidity", (xmax+1)/2 + 19, 133 , 4, 0xE165, 0, 1);
   
  // clock
  my_lcd.Draw_Rectangle(30, ymax-90 , xmax-30 , ymax-30 );
  my_lcd.Fill_Rectangle(45, ymax-105 , xmax-45 , ymax-75 );
  show_string("CLOCK", CENTER, ymax-100, 3, 0x0000, 0, 1);
}

// ----- CLOCK ----- 
String getDayOfWeek(int i)
{
  switch(i)
  {
    case 1:   return "Monday";    break;
    case 2:   return "Tuesday";   break;
    case 3:   return "Wednesday"; break;
    case 4:   return "Thursday";  break;
    case 5:   return "Friday";    break;
    case 6:   return "Saturday";  break;
    case 7:   return "Sunday";    break;
    default:  return "Monday";    break;
  }
}

void getTime()
{
   DateTime now = rtc.now();  //get the current date-time
   minuteNow = now.minute();
   if(minuteNow!=minutePrevious)
   {
    dateString = getDayOfWeek(now.dayOfWeek())+" ";
    dateString = dateString+String(now.date())+"."+String(now.month());
    dateString= dateString+"."+ String(now.year()); 
    minutePrevious = minuteNow;
    hours = String(now.hour());
    if(now.minute()<10)
      hours = hours+":0"+String(now.minute());
    else
      hours = hours+":"+String(now.minute());
    String dateAndTime = dateString+" "+hours;
    Serial.println(dateAndTime);
    uint8_t buf[50];
    dateAndTime.toCharArray(buf,50);
    my_lcd.Set_Draw_color(0, 0, 0);
    my_lcd.Fill_Rectangle(31, ymax-74 , xmax-31 , ymax-31 );
    show_string(buf, CENTER, ymax-61, 3, 0xE165, 0, 1);
   }
}

// --- Indoor Temperature ---
void printIndoorTemperature()
{
  indoorTemperature = dht.readTemperature();
  String temperature = String(indoorTemperature);;
  if(indoorTemperature != previousIndoorTemperature & temperature!=" NAN")
  {
    temperature[4]=' ';//'C';
    Serial.println("Indoor Temperature : "+temperature);
    uint8_t buf[6];
    temperature.toCharArray(buf,6);
    my_lcd.Set_Draw_color(0, 0, 0);
    my_lcd.Fill_Rectangle(31, 98, (xmax+1)/2 - 16, 132);
    show_string(buf , 34, 99 , 4, 0xE165, 0, 1);
    previousIndoorTemperature = indoorTemperature;
  }
}

// --- Indoor Humidity ---
void printIndoorHumidity()
{
  previousIndoorHumidity = indoorHumidity;
  indoorHumidity = dht.readHumidity();
  String humidity = String(indoorHumidity);
  if(indoorHumidity != previousIndoorHumidity & humidity!=" NAN")
  {
    humidity[4]=' ';//'%';
    Serial.println("Indoor Humidity : "+humidity);
    uint8_t buf[6];
    humidity.toCharArray(buf,6);
    my_lcd.Set_Draw_color(0, 0, 0);
    my_lcd.Fill_Rectangle(31, 166, (xmax+1)/2 - 16, 198);
    show_string(buf, 34, 167 , 4, 0xE165, 0, 1);
    previousIndoorHumidity = indoorHumidity; 
  }
}

// --- Getting data from Remote Communication ---
void checkForWirelessData()
{
  if ( myRadio.available()) 
    {
      myRadio.read( &data, sizeof(data) );
      previousRemoteTemperature = remoteTemperature;
      previousRemoteHumidity = remoteHumidity;
      remoteTemperature = data.temperature;
      remoteHumidity = data.humidity;
    }
}  

// --- Remote Temperature ---
void printRemoteTemperature()
{
  String temperature;
  if(remoteTemperature != previousRemoteTemperature)
  {/*
    if(remoteHumidity == 0.0 && remoteTemperature == 0.0) //We just booted up
      temperature = "---";
    else 
    {  */
      temperature = String(remoteTemperature);
      temperature[4]=' ';//'C';
      Serial.println("Remote Temperature : "+temperature);
      uint8_t buf[6];
      temperature.toCharArray(buf,6);
      my_lcd.Set_Draw_color(0, 0, 0);
      my_lcd.Fill_Rectangle((xmax+1)/2 + 16, 98, xmax-31, 132); 
      show_string(buf, (xmax+1)/2 + 19, 99 , 4, 0xE165, 0, 1);   
      previousRemoteTemperature = remoteTemperature;
   // }
  }
}

// --- Remote Humidity ---
void printRemoteHumidity()
{
  String humidity;
  if(remoteHumidity != previousRemoteHumidity)
  {
    if(remoteHumidity == 0.0 && remoteTemperature == 0.0) //We just booted up
      humidity = "---";
    else
    {
      humidity = String(remoteHumidity,1);
      humidity[4]=' ';//'%';
      Serial.println("Remote Humidity : "+humidity);
      uint8_t buf[6];
      humidity.toCharArray(buf,6);
      my_lcd.Set_Draw_color(0, 0, 0);
      my_lcd.Fill_Rectangle((xmax+1)/2 + 16, 166, xmax-31, 198);
      show_string(buf, (xmax+1)/2 + 19, 167 , 4, 0xE165, 0, 1);
      previousRemoteHumidity = remoteHumidity;
    }
  }
}




void setup() 
{
  Serial.begin(9600);
  my_lcd.Init_LCD();
  Serial.println(my_lcd.Read_ID(), HEX);
  my_lcd.Fill_Screen(0x0);  
  my_lcd.Set_Rotation(1);
  UI_setup();
  rtc.begin();
  dht.begin();
  myRadio.begin(); 
  myRadio.setChannel(115); 
  myRadio.setPALevel(RF24_PA_MAX);
  myRadio.setDataRate( RF24_250KBPS ) ; 
  myRadio.openReadingPipe(1, address);
  myRadio.startListening();
  delay(100);
}

void loop() 
{ 
  getTime();
  printIndoorTemperature();
  printIndoorHumidity();
  checkForWirelessData();
  printRemoteTemperature();  
  printRemoteHumidity();
  //delay();
}

