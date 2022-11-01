//EK210 Light Tracker Project
//Prototype Arduino Code
//Section A5 - Professor Grace
//Team Members: David Moody, Brennan Mahoney, Rohan Kumar, Sora Kakigi
//10/31/2022

//UV light intensity analog sensor pins
const int SensorPin_TL = A1;   //top left
const int SensorPin_TR = A2;   //top right
const int SensorPin_BL = A3;   //bottom left
const int SensorPin_BR = A4;   //bottom right

//Variables to store readings from the UV intensity sensors
//(the sum of the past 5 readings for better precision)
int Brightness_TL = 0;
int Brightness_TR = 0;
int Brightness_BL = 0;
int Brightness_BR = 0;

//Minimum brightness readings calculated in calibration and used for tracking
int min_TL = 0;
int min_TR = 0;
int min_BL = 0;
int min_BR = 0;

//Constants for scaling the relative sensor data during calibration
float Cal_TL = 0;
float Cal_TR = 0;
float Cal_BL = 0;
float Cal_BR = 0;
float Offset_X = 0;
float Offset_Y = 0;

//Variables for X and Y tracked position
int Position_X = 0;
int Position_Y =0;

//Calibration indicator LED pins
//(shows the user which sensor to "aim" at during the calibration sequence)
const int LEDPin_TL = 1;   //top left
const int LEDPin_TR = 2;   //top right
const int LEDPin_BL = 3;   //bottom left
const int LEDPin_BR = 4;   //bottom right

//Calibration sequence button pin and state
const int CalButtonPin = 13;
int CalButtonState;

//Variable to check if calibration has been completed at least once
int Calibrated = 0;

//LCD Display Code
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

void setup() {
  // put your setup code here, to run once:
  
  pinMode(CalButtonPin, INPUT);   //Calibration sequence start button

  //Initialize LCD and display calibration on startup request
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration");
  lcd.setCursor(0, 1);
  lcd.print("Required");
}


void loop() {
  // put your main code here, to run repeatedly:
  
  //Run calibration sequence if button is pressed (to allow recalibration while using the system)
  CalButtonState = digitalRead(CalButtonPin);
  if (CalButtonState == LOW){
    CalibrateSensorsFunction();
    Calibrated = 1;
  }

  //Run tracking function if calibration has been completed at least once
  if (Calibrated == 1){
    TrackPositionFunction();
  }
}


void ReadSensorDataFunction(){
  //Used in the calibration and tracking functions, not called directly from the main code
  
  //Reset sensor reading values to 0
  Brightness_TL = 0;
  Brightness_TR = 0;
  Brightness_BL = 0;
  Brightness_BR = 0;
  
  //Reads the sensor values 5 times, with a 10 ms delay between readings
  //Sums the values for each sensor
  for (int i = 0; i<5; i++){
    Brightness_TL += analogRead(SensorPin_TL);
    Brightness_TR += analogRead(SensorPin_TR);
    Brightness_BL += analogRead(SensorPin_BL);
    Brightness_BR += analogRead(SensorPin_BR);
    delay(10);
  } 
}


void CalibrateSensorsFunction(){
  //User "aims" at the 4 corner sensors to gather data for calculating scaling coefficients and offsets
  //to calibrate the tracked position to screen/user alignment and room light
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating");
  lcd.setCursor(0, 1);
  lcd.print("Sensors...");
  
  //Local variables to store maximum brightness readings for calibration
  //Note that minimum readings are global variables assigned at the beginning of the program
  //so that they can be used in the tracking function
  int max_TL = 0;
  int max_TR = 0;
  int max_BL = 0;
  int max_BR = 0;

  delay(10000); //Gives the user time to sit back in their normal pose

  //Reads sensor data for each position, recording maximum for that sensor and minimum for opposite corner
  digitalWrite(LEDPin_TL, HIGH);    //Turn on LED for sensor the user should aim at
  delay(2000);    // 2 seconds to aim at the sensor
  ReadSensorDataFunction();
  max_TL = Brightness_TL;
  min_BR = Brightness_BR;
  digitalWrite(LEDPin_TL, LOW);

  digitalWrite(LEDPin_TR, HIGH);
  delay(2000);
  ReadSensorDataFunction();
  max_TR = Brightness_TR;
  min_BL = Brightness_BL;
  digitalWrite(LEDPin_TR, LOW);

  digitalWrite(LEDPin_BL, HIGH);
  delay(2000);
  ReadSensorDataFunction();
  max_BL = Brightness_BL;
  min_TR = Brightness_TR;
  digitalWrite(LEDPin_BL, LOW);

  digitalWrite(LEDPin_BR, HIGH);
  delay(2000);
  ReadSensorDataFunction();
  max_BR = Brightness_BR;
  min_TL = Brightness_TL;
  digitalWrite(LEDPin_BL, LOW);

  //Minimum brightness value (when aiming at the opposite corner of the screen)is used
  //as a baseline value to make the readings scale more accurately to position
  //Scaling coefficients are now calculated (such that a scaled reading of 50 is peak brightness) 
  Cal_TL = 50.0/(max_TL-min_TL);
  Cal_TR = 50.0/(max_TR-min_TR);
  Cal_BL = 50.0/(max_BL-min_BL);
  Cal_BL = 50.0/(max_BR-min_BR); 

  //User should now aim in the center of the screen
  //so that offsets can be calculated
  digitalWrite(LEDPin_TL, HIGH);    //    All LEDS on to indicate aiming for center
  digitalWrite(LEDPin_TR, HIGH);
  digitalWrite(LEDPin_BL, HIGH);
  digitalWrite(LEDPin_BR, HIGH);
  delay(2000);
  ReadSensorDataFunction();
  Offset_X = 50 - (-Cal_TL*(Brightness_TL-min_TL) + Cal_TR*(Brightness_TR-min_TR) - Cal_BL*(Brightness_BL-min_BL) + Cal_BR*(Brightness_BR-min_BR));
  Offset_Y = 50 - (Cal_TL*(Brightness_TL-min_TL) + Cal_TR*(Brightness_TR-min_TR) - Cal_BL*(Brightness_BL-min_BL) - Cal_BR*(Brightness_BR-min_BR));
  digitalWrite(LEDPin_TL, LOW);
  digitalWrite(LEDPin_TR, LOW);
  digitalWrite(LEDPin_BL, LOW);
  digitalWrite(LEDPin_BR, LOW);

  //serial printing for testing sensors
  //this portion prints calibration coefficients
  Serial.print("Cal_TL: ");
  Serial.print(Cal_TL);
  Serial.print("\t");
  Serial.print("Cal_TR: ");
  Serial.print(Cal_TR);
  Serial.print("\t");
  Serial.print("Cal_BL: ");
  Serial.print(Cal_BL);
  Serial.print("\t");
  Serial.print("Cal_BR: ");
  Serial.print(Cal_BR);
  Serial.print("\t");
  
}


void TrackPositionFunction(){
  //Uses data from the 4 UV brightness sensors to calculate the position of where the user is "aiming"
  //relative to the screen.  Outputs the tracked position to the LCD as a percentage coordinate.

  //Just using a simple linear average to calculate position for now, will test other formulas to try and find something more accurate

  //Reads sensor data
  ReadSensorDataFunction();

  //Serial printing Sensor Data x5
  Serial.print("Top Right Sensor: ");
  Serial.print(Brightness_TR);
  Serial.print("\t");
  Serial.print("Top Left Sensor: ");
  Serial.print(Brightness_TL);
  Serial.print("\t");
  Serial.print("Bottom Right Sensor: ");
  Serial.print(Brightness_BR);
  Serial.print("\t");
  Serial.print("Bottom Left Sensor: ");
  Serial.print(Brightness_BL);
  Serial.print("\t");
  
  //Calculates position coordinates
  //Currently code is allowed to show values over/under the 0-100% screen range.
  Position_X = Offset_X - Cal_TL*(Brightness_TL-min_TL) + Cal_TR*(Brightness_TR-min_TR) - Cal_BL*(Brightness_BL-min_BL) + Cal_BR*(Brightness_BR-min_BR);
  Position_Y = Offset_Y + Cal_TL*(Brightness_TL-min_TL) + Cal_TR*(Brightness_TR-min_TR) - Cal_BL*(Brightness_BL-min_BL) - Cal_BR*(Brightness_BR-min_BR);

  //Serial printing position
  Serial.print("Position X: ");
  Serial.print(Position_X);
  Serial.print("\t");
  Serial.print("Position Y: ");
  Serial.print(Position_Y);
  Serial.print("\t");

  //Print X and Y position to LCD with % symbols
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("X%");
  lcd.setCursor(3, 0);
  lcd.print(Position_X);
  
  lcd.setCursor(0, 1);
  lcd.print("Y%");
  lcd.setCursor(3, 1);
  lcd.print(Position_Y);
}
