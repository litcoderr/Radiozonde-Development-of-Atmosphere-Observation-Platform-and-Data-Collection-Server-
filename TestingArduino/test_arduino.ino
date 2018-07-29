#include <SFE_BMP180.h>
#include <Wire.h>
SFE_BMP180 pressure; //SFE_BMP180 Object called "pressure"
#define ALTITUDE 1655.0 //Altitude of the testing area in Meters

const int xPin = 0;
const int yPin = 1;
const int zPin = 2;

int minVal = 265;
int maxVal = 402;
 
//tilted angle in degree 
double x;
double y;
double z;

//dust
int measurePin = 6;
int ledPower = 12;
int samplingTime = 280;
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Start Detection");
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(ledPower,LOW);
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin);
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower,HIGH);
  delayMicroseconds(sleepTime);
  calcVoltage = voMeasured * (3.3 / 1024);
  dustDensity = 0.17 * calcVoltage - 0.1;
 
  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);
 
  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);
 
  Serial.print(" - Dust Density: ");
  Serial.println(dustDensity);
 
  delay(1000);
}
