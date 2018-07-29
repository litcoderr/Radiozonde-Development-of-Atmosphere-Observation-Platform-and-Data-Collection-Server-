#include <SFE_BMP180.h>
#include <Wire.h>
#include <SoftwareSerial.h>
SFE_BMP180 pressure; //SFE_BMP180 Object called "pressure"
SoftwareSerial mySerial(19,18);
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
double voMeasured = 0;
double calcVoltage = 0;
double dustDensity = 0;


//data_processing
int data_num = 0;
int prev_num = -1;
double data_clust[8];

char numberortext=0;
char enable=0;
String intext;
String send_data;
String innumber;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(ledPower, OUTPUT);
  while (!Serial)
  {
  ; // wait for serial port to connect.
  }
  Serial.println("Start Detection");

  //Beginning BMP 180 Detection
  if (pressure.begin()) {
    Serial.println("BMP180 init Success....Starting Detection");
  } else {
    Serial.println("BMP180 init fail...Stopping Process\n\n");
    while (1); //Pausing Forever
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  data_num++;
  BMP180Process();
  GyroProcess();
  Dust();
  Bluetooth_Send();
  delay(5000);
  //Pause for 0.1 seconds
}

void Bluetooth_Send(){
   if(prev_num != data_num){
    int i = 0;
   for(i=0;i<8;i++){
      if(i>0){
      send_data = send_data +","+String(data_clust[i]);
      }else{
        send_data = send_data +String(data_clust[i]);
      }
    if(i==7)send_data = send_data + "\n";
    prev_num = data_num;
   }
   mySerial.print(send_data);
   send_data = "";
   }
}

void Dust() {
  digitalWrite(ledPower, LOW);
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin);
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(sleepTime);
  calcVoltage = voMeasured * (3.3 / 1024);
  dustDensity = 0.17 * calcVoltage - 0.1;

  Serial.print("Raw Signal Value (0-1023): ");
  Serial.print(voMeasured);

  Serial.print(" - Voltage: ");
  Serial.print(calcVoltage);

  Serial.print(" - Dust Density: ");
  Serial.println(dustDensity);

  data_clust[7] = dustDensity;
}

void GyroProcess() {
  int xread = analogRead(xPin);
  int yread = analogRead(yPin);
  int zread = analogRead(zPin);

  int xAng = map(xread, minVal, maxVal, -90, 90);
  int yAng = map(yread, minVal, maxVal, -90, 90);
  int zAng = map(zread, minVal, maxVal, -90, 90);

  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

  data_clust[4] = x;
  data_clust[5] = y;
  data_clust[6] = z;

  Serial.print("x: ");
  Serial.print(x);
  Serial.print(" | y: ");
  Serial.print(y);
  Serial.print(" | z: ");
  Serial.println(z);
}

void BMP180Process() {
  //BMP180Process in the loop() function
  char status;
  double T, P, p0, a;

  // Loop here getting pressure readings every 10 seconds.`1


  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:

  Serial.println();
  Serial.print(data_num);
  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE, 0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE * 3.28084, 0);
  Serial.println(" feet");

  data_clust[0] = (double)data_num;//datanum -> data_clust[0]
  
  // If you want to measure altitude, and not pressure, you will instead need
  // to provide a known baseline pressure. This is shown at the end of the sketch.

  // You must first get a temperature measurement to perform a pressure reading.

  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.

  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T, 2);
      Serial.print(" deg C, ");
      Serial.print((9.0 / 5.0)*T + 32.0, 2);
      Serial.println(" deg F");
      data_clust[2] = T; //temperature at #2 in celcius
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P, 2);
          Serial.print(" mb, ");
          Serial.print(P * 0.0295333727, 2);
          Serial.println(" inHg");

          data_clust[3] = P; // pressure in mb
          
          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P, ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0, 2);
          Serial.print(" mb, ");
          Serial.print(p0 * 0.0295333727, 2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P, p0);
          Serial.print("computed altitude: ");
          Serial.print(a, 0);
          Serial.print(" meters, ");
          Serial.print(a * 3.28084, 0);
          Serial.println(" feet");
          data_clust[1] = a; // computed altitude
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");
}
