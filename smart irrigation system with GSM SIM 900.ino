#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SoftwareSerial.h> // Library for using serial communication
SoftwareSerial SIM900(7, 8); // Pins 7, 8 are used as used as software serial pins
LiquidCrystal_I2C lcd(0x27, 16, 2);



String incomingData; // for storing incoming serial data
String hold = "///";
String message = "";   // A String for storing the message
int relay_pin = 5;    // Initialized a pin for relay module
int soilpin = 6;
int waterpin = A1;
int waterval;
int soilval;
const int trigPin = 10;
const int echoPin = 11;
bool flag = true;

int duration;
int Distance;
int checkDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  int calcDistance = (duration * 0.0343) / 2;
  delay(10);
  return calcDistance;
}

void SIM900power()
{
  pinMode(9, OUTPUT);
  //  digitalWrite(9,LOW);
  //  delay(1000);
  digitalWrite(9, HIGH);
  delay(3000);
  digitalWrite(9, LOW);
  delay(1000);
}
void setup()
{
  SIM900power();
  pinMode(soilpin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(12, OUTPUT);

  //  digitalWrite(12, HIGH);
  //  delay(500);
  //  digitalWrite(12, LOW);
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.print("SMART IRRIGATION");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM.");
  delay(1500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("initializing...");
  delay(500);
  Serial.begin(9600); // baudrate for serial monitor
  SIM900.begin(9600); // baudrate for GSM shield
  //  delay(20000);
  pinMode(relay_pin, OUTPUT);   // Setting erlay pin as output pin
  digitalWrite(relay_pin, HIGH);  // Making relay pin initailly low

  // set SMS mode to text mode
  SIM900.print("AT+CMGF=1\r");
  delay(100);

  // set gsm module to tp show the output on serial out
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("GSM Booting up");
  lcd.setCursor(0, 1);
  lcd.print("Please wait...");
  delay(10000);
}


void loop()
{
  soilval = digitalRead(soilpin);
  if (soilval == 1) // the soil is dry
  {
    flag = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Moisture < 50%");
    lcd.setCursor(0, 1);
    lcd.print("checking water...");
    delay(3000);
    waterval = analogRead(waterpin);
    Distance = checkDistance();
    delay(10);
    if (waterval > 400 || Distance < 15) //there's water in the tank
    {
      lcd.setCursor(0, 1);
      lcd.print("Waterlevel 100%");
      send_message("Hello, the farm needs water, send:on pump to on the pump");
      delay(2000);
recheck:
      //      Serial.println("b4 while");
      while (!(SIM900.available() > 0)) {
        if (digitalRead(soilpin) == 0) return;
      }
      //      Serial.println("after while");
      receive_message();
      if (!(incomingData.indexOf("on pump") > 0)) goto recheck;

      if (incomingData.indexOf("on pump") > 0)
      {
        digitalWrite(relay_pin, LOW);
        message = "pump is ON, Currently watering the farm.";
        Serial.println(message);
        lcd.clear();
        lcd.print("Pump is On");
//        delay(3000);
//        lcd.clear();
//        lcd.print("Currently");
//        lcd.setCursor(0,1);
//        lcd.print("Irrigating...");
        send_message(message);// Send a sms back to confirm that the relay is turned on
        while (soilval == 1)
        {
          soilval = digitalRead(soilpin);
        }
        digitalWrite(relay_pin, HIGH);
        message = "pump is OFF";
        lcd.clear();
        lcd.print(message);
        send_message(message);
      }
    }
    else {
      message = "Water level is LOW! <50% water in tank";
      lcd.clear();
      lcd.print("Water level LOW!");
      lcd.setCursor(0, 1);
      lcd.print("<50% in tank.");
      send_message(message);
    }

  }
  else {
    if (flag) {
      Serial.println("inside big else");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Soil moisture OK.");
      lcd.setCursor(0, 1);
      lcd.print("Sending message...");
      send_message("Hello, the soil has been watered.");
      flag = false;
    }

  }


}
void receive_message()
{
  if (SIM900.available() > 0)
  {
    incomingData = SIM900.readString(); // Get the data from the serial port.
    Serial.print(incomingData);
    delay(10);
  }
}

void send_message(String message)
{
  Serial.println("Initializing...");
  delay(1000);

  SIM900.println("AT"); //Handshaking with SIM900
  updateSerial();

  SIM900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CMGS=\"+2348068591032\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  SIM900.print(message); //text content
  updateSerial();
  SIM900.write(26);
}

void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    SIM900.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (SIM900.available() >= 1)
  {
    Serial.write(SIM900.read());//Forward what Software Serial received to Serial Port
  }
}
