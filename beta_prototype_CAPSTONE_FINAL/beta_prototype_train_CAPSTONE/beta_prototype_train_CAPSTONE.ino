/**
 *Ryan Restifo, Joey de la Viesca
 *WFU Engineering Capstone
 *City Circuits Museum Exhibit
 *4/21/23
 *
 *The purpose of this code is the following:
 *  1. Drive the train for the Sustain the Train station
 *  2. Simulate charging capacitors when a momentary switch button is pressed
 *  3. Cut off power to the train when a switch is in the off state 
 *  4. Display a visual representation of train speed and charge on LCDs
 */

 /*Block Info
  * Resistor Block Resistance: 4.7 kΩ
  * Small Capacitor Block Resistance: 100 kΩ
  * Big Capacitor Block Resistance: 330 kΩ
  */
 //libraries
#include <LCD_I2C.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ADS1X15.h>

//Initialize 16 bit ADC
Adafruit_ADS1115 ads;

//I/O pins
#define boardPower 2
#define chargeButton 3
#define train 9
#define sensor 2
#define capSlot1 0
#define capSlot2 1

//variables for computing train speed
 int Rmin = 0;
 int Rmax = 250000;
 float Vin = 5.0;
 float Vout = 0;
 float R1 = 2200.;
 double R2 = 0;
 float buffer = 0;
 int trainSpeed = 0;

//variables for buttons
 int chargeState = 0;
 int boardState = 0;
 int charge = 0;
 int smallCap = 500;
 int bigCap = 1000;
 int totalCap = 0;
 int capPartial1 = 0;
 int capPartial2 = 0;
 bool capSlot1State = false;
 bool capSlot2State = false;

//variables for LCD displays
 int newSpeedDisplay = 0;
 int oldSpeedDisplay = 0;
 int newChargeDisplay = 0;
 int oldChargeDisplay = 0;

//Initializing each lcd display
LCD_I2C lcd4(0x24,20,4); //LCD4: Capacitor Charge
LCD_I2C lcd5(0x23,20,4); //LCD5: Train Speed

//Defining progress bar unit custom character
byte progressUnit[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};


void setup() {
  Serial.begin(9600);

  //SET UP ADS
  ads.setGain(GAIN_ONE);
  ads.begin();
  
  //SET UP PINS
  pinMode(train, OUTPUT);
  pinMode(boardPower, INPUT_PULLUP);
  pinMode(chargeButton, INPUT_PULLUP);
  
  //SET UP LCD DISPLAYS 
  //lcd 4 setup
  lcd4.begin();
  lcd4.backlight();
  lcd4.print("HELLO LCD 1");
  //lcd 5 setup
  lcd5.begin();
  lcd5.backlight();
  lcd5.print("HELLO LCD 2");

  //CUSTOM CHARACTER INDEXING
  lcd4.createChar(0,progressUnit);
  lcd5.createChar(0,progressUnit);



  //display static lcd text
  displayLcdText();

  delay(10);
}

void loop() {
//    Serial.begin(9600);
    
    chargeState = digitalRead(chargeButton);
    boardState = digitalRead(boardPower);
    int16_t adc0 = ads.readADC_SingleEnded(capSlot1);
    int16_t adc1 = ads.readADC_SingleEnded(capSlot2);
//    Serial.print("Capacitor ADC0 value: ");
//    Serial.println(adc0);
//    Serial.print("Capacitor ADC1 value: ");
//    Serial.println(adc1);

//16 bit ADC output floats at around 150-170 when 0, so the lower threshold
//for detecting a capacitor block is set to 200. 
    if(adc0>200){
      capSlot1State = true; //there is a capacitor in slot 1
      //if the adc reads in the range 32000-33000, a small capacitor is placed
      if(adc0 > 32000 && adc0 < 33000){
        capPartial1 = smallCap; //partial capacitance 2 is small
      }
      //if the adc reads in the range 29000-30000, a big capacitor is placed
      if(adc0 > 29000 && adc0 < 30000){
        capPartial1 = bigCap; //partial capciatance 1 is big
      }
    }
    //if there is no capacitor in slot 1, the partial capicatance is 0
    else{
      capSlot1State = false;
      capPartial1 = 0;
    }
    if(adc1>200){
      capSlot2State = true;
      if(adc1 > 32000 && adc1 < 33000){
        capPartial2 = smallCap;
      }
      if(adc1 > 29000 && adc1 < 30000){
        capPartial2 = bigCap;
      }
    }
    else{
      capSlot2State = false;
      capPartial2 = 0;
    }

    //total capacitance calculated by adding the two partial capacitances
    //this value serves as a limit for the charge counter
    totalCap = capPartial1 + capPartial2;
//    Serial.print("Total Capacitance: ");
//    Serial.println(totalCap);

    //Initialize and read raw train speed voltage from adc pin 2
    int16_t adc2;
    adc2 = ads.readADC_SingleEnded(2);

    //Calculate R2 by converting adc value to voltage and using voltage
    //divider equation.
    Vout = (adc2 * Vin)/32768.0;
//    Vout = map(adc1, 0, 32768, 0, 5);
    buffer = (Vin/Vout) - 1;
    R2 = (R1 * buffer);
//    R2 = constrain(R2, Rmin, Rmax);
    //If the board is turned off, train stops and capacitors cannot discharge
    if(boardState == true){
      trainSpeed = 0;
      if(chargeState != true){
        charge++; //increment charge counter if the charge button is being pressed 
        if(charge > totalCap){
          charge = totalCap;
        }
      }
    }
    //If board is on and charge is greater than 0, train will run 
    else{
      trainSpeed = map(adc2, 0, 32768, 0, 255);
      //if charge button is not pressed, charge will decrease (discharging)
      if(chargeState == true){
        charge--;
        //if charge reaches 0, the train will stop
        if(charge<0){
          charge = 0;
          trainSpeed = 0;
        }
      }
      //if charge button is pressed, increase charge
      else{
        charge++;
        //cap charge at total capacitance value
        if(charge > totalCap){
          charge = totalCap;
        }
      }
  
    }

   //Charge bar LCD display
   oldChargeDisplay = newChargeDisplay;
   newChargeDisplay = map(charge, 0, 1500, 0, 20);

   if(newChargeDisplay>oldChargeDisplay){
   for(int i = oldChargeDisplay; i < newChargeDisplay; i++){
      lcd4.setCursor(i,2);
      lcd4.write((byte)0);
    }
   }
   else if(oldChargeDisplay>newChargeDisplay){
    for(int i = oldChargeDisplay; i > newChargeDisplay; i--){
      lcd4.setCursor(i,2);
      lcd4.print(" ");
    }
//    Serial.print("Charge: ");
//    Serial.println(charge);
    

//    Serial.print("Speed: ");
//    Serial.println(trainSpeed); 
//
//    if(trainSpeed > 200){
//      analogWrite(train, 0);
//    }
//    else{
      analogWrite(train, trainSpeed);
//    }

   //Speed bar LCD display
   oldSpeedDisplay = newSpeedDisplay;
   newSpeedDisplay = map(trainSpeed, 0, 255, 0, 20);
   
   if(newSpeedDisplay>oldSpeedDisplay){
   for(int i = oldSpeedDisplay; i < newSpeedDisplay; i++){
      lcd5.setCursor(i,2);
      lcd5.write((byte)0);
    }
   }
   else if(oldSpeedDisplay>newSpeedDisplay){
    for(int i = oldSpeedDisplay; i > newSpeedDisplay; i--){
      lcd5.setCursor(i,2);
      lcd5.print(" ");
    }
   }


//    Serial.print("R2: ");
//    Serial.println(R2);

}

//Display static text for LCD displays. Only needs to be called once.
void displayLcdText(){
  //Display charge text for sustain the train LCD
  lcd4.clear();
  lcd4.setCursor(6,1);
  lcd4.print("CHARGE: ");
  lcd4.setCursor(0,2);
  lcd4.print("                    ");

  //Display speed text for sustain the train LCD
  lcd5.clear();
  lcd5.setCursor(6,1);
  lcd5.print("SPEED: ");
  lcd5.setCursor(0,2);
  lcd5.print("                    ");
 
}
