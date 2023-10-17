/**
 *Ryan Restifo, Joey de la Viesca
 *WFU Engineering Capstone
 *City Circuits Museum Exhibit
 *4/21/23
 *
 *The purpose of this code is the following:
 *  1. To read an input voltage from the "Power Generation Station" crank and output
 *     a visual representation of the resulting speed of the crank on an LCD display. 
 *  2. To control the brightness of RGB LED lights for the "City Circuits" building lights.
 *  3. Print the total resistance of the circuit in the "Illumination Station" to an LCD display.  
 *  4. Print a visual representation of the brightness of the building lights to an LCD display.
 */


 /*Block Info
  * Resistor Block Resistance: 4.7 kΩ
  */

 //libraries
#include <LCD_I2C.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ADS1X15.h>

//Initialize 16 bit ADC
Adafruit_ADS1115 ads;

//I/O pins
#define ferris_wheel 9  //detects input voltage of "Power Generation Station" crank generator
#define sensor A0 //detects input voltage from "Illumination Station" circuit
#define red_LED 3 
#define green_LED 2
#define blue_LED 4

//variables for computing speed of ferris wheel 
  int rawCrank = 0;
  int newSpeed = 0;
  int oldSpeed = 0; 

//variables for computing resistance of tray
  int Rmin = 0;
  int Rmax = 250000;
  float Vin = 5.0;
  float Vout = 0;
  float R1 = 2200.;
  double R2 = 0;
  float buffer = 0;

//intensity variables
  int old_intensity;
  int intensity; 
  int red_int = 0; 
  int green_int = 0;
  int blue_int = 0;

//warm white color offset values
  int g_offset = 2; //green offset factor
  int b_offset = 22; //blue offset factor

//Initializing each lcd display
LCD_I2C lcd1(0x27,20,4); //LCD1: Ferris Wheel Speed
LCD_I2C lcd2(0x26,20,4); //LCD2: City Lights Brightness
LCD_I2C lcd3(0x25,20,4); //LCD3: City Lights Total Resistance

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

//Defining capital Omega custom character
byte ohm[8] = {
  0b00000,
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b01010,
  0b11011,
  0b00000
};

void setup() {
  Serial.begin(9600);
  /*
   *****************************************************
   *POWER GENERATION STATION SETUP
   *****************************************************
   */

   ads.setGain(GAIN_ONE);
   ads.begin();

   pinMode(ferris_wheel, OUTPUT);

   analogWrite(ferris_wheel, 100);
   delay(500);
   analogWrite(ferris_wheel, 0);
   

  /*
   *****************************************************
   * ILLUMINATION STATION SETUP
   *****************************************************
   */
   
  //SET UP PINS
  pinMode(red_LED, OUTPUT);
  pinMode(green_LED, OUTPUT);
  pinMode(blue_LED, OUTPUT);
  pinMode(A0, INPUT);

  //TEST ALL LEDs
    analogWrite(red_LED, 255);
      delay(250);
    analogWrite(green_LED, 255);
      delay(250);
    analogWrite(blue_LED, 255);
      delay(250);
    analogWrite(red_LED, 0);
    analogWrite(green_LED, 0);
    analogWrite(blue_LED, 0);
      delay(250);
    
  
  //TEST LIGHTS (fade in and out) 
  for(int i=0; i<255; i++){
    analogWrite(red_LED, i);
    analogWrite(green_LED, i);
    analogWrite(blue_LED, i);
    delay(1);
  }
  for(int i=255; i>0; i--){
    analogWrite(red_LED, i);
    analogWrite(green_LED, i);
    analogWrite(blue_LED, i);
    delay(1);
  }

  
  //SET UP LCD DISPLAYS 
  //lcd 1 setup
  lcd1.begin();
  lcd1.backlight();
  lcd1.print("HELLO LCD 1");
  //lcd2 setup
  lcd2.begin();
  lcd2.backlight();
  lcd2.print("HELLO LCD 2");
  //lcd3 setup
  lcd3.begin();
  lcd3.backlight();
  lcd3.print("HELLO LCD 3");

  //CUSTOM CHARACTER INDEXING
  lcd1.createChar(0,progressUnit);
  lcd1.createChar(1,ohm);
  lcd2.createChar(0,progressUnit);
  lcd2.createChar(1,ohm);
  lcd3.createChar(0,progressUnit);
  lcd3.createChar(1,ohm);


  //display static lcd text
  displayLcdText();

}

void loop() {
  
  /*
   *****************************************************
   *POWER GENERATION STATION
   *****************************************************
   */

   oldSpeed = newSpeed;
   
   int16_t adc0;
    
   adc0 = ads.readADC_SingleEnded(0);
   float voltage = adc0 * 4.096 / 32768;

//   Serial.print("Voltage: ");
//   Serial.print(voltage,4);
//   Serial.println(" V");

   float minVoltage = 0.05;
   float maxVoltage = 1.2; //0.65
   int pwmVal = map(abs(voltage) * 1000, minVoltage * 1000, maxVoltage * 1000, 0, 255);
   pwmVal = constrain(pwmVal, 0, 255);

   analogWrite(ferris_wheel, pwmVal);

   newSpeed = map(pwmVal,0,255,0,19);
   
   if(newSpeed>oldSpeed){
    for(int i = oldSpeed; i < newSpeed; i++){
      lcd1.setCursor(i,2);
      lcd1.write((byte)0);
    }
   }
   else if(oldSpeed>newSpeed){
    for(int i = oldSpeed; i > newSpeed; i--){
      lcd1.setCursor(i,2);
      lcd1.print(" ");
    }
   }

   delay(250);
   

  /*
   *****************************************************
   * ILLUMINATION STATION
   *****************************************************
   */

    int16_t adc1;
    adc1 = ads.readADC_SingleEnded(1);
    
    Vout = (adc1 * Vin)/32768.0;
//    Vout = map(adc1, 0, 32768, 0, 5);
    buffer = (Vin/Vout) - 1;
    R2 = (R1 * buffer);
//    R2 = constrain(R2, Rmin, Rmax);

    //Serial.print(R2); //print resistance to serial for debugging (optional)
    //Serial.println(" kOhms");

    //create new PWM value to write to LEDs
    intensity = 1.4*map(adc1, 0, 32768, 0, 255); // writeValue = (255./1023.) * raw; 
    intensity = constrain(intensity, 0, 255);
    if(intensity < 20){
      intensity = 0;
    }
    Serial.print("R2: ");
    Serial.println(R2);
    Serial.print("intensity: ");
    Serial.println(intensity);

   

    //If resistance is less than 800 Ω and lights are off, indicate open circuit
    if(intensity == 0 && R2 < 800){
      lcd3.setCursor(3,1);
      lcd3.print("OPEN CIRCUIT");
      lcd3.setCursor(3,2);
      lcd3.print("PLACE BLOCKS!");
//      lcd3.setCursor(3,2);
//      lcd.print("NO RESISTORS");
    }
    //Else, display resistance in kΩ
    else{
    lcd3.setCursor(3,1);
    lcd3.print(" RESISTANCE: ");
    lcd3.setCursor(0,2);
    lcd3.print("                    ");
    lcd3.setCursor(7,2);
    lcd3.print(R2/1000);
    lcd3.setCursor(12,2);
    lcd3.print(" k");
    lcd3.setCursor(14,2);
    lcd3.write((byte)1);
    }
    
    
  //When building light brightness increases, increment RGB values to new intensity
    if((int)old_intensity < (int)intensity){
      for(int i=old_intensity; i<intensity; i++){
        analogWrite(red_LED, i);
        analogWrite(green_LED, i/g_offset);
        analogWrite(blue_LED, i/b_offset);

        int barProg = map(i,0,255,0,19); //map current R value to progress bar position
        int oldBarProg = map(i-1,0,255,0,19); //map previous R value to progress bar position

        //If current bar progress is less than old bar progress, add unit at current bar progress position
        if(barProg>oldBarProg){
          lcd2.setCursor(barProg,2);
          lcd2.write((byte)0);
        }
 
        delay(5);
      }
    }
  //When building light brightness decreases, decrement RGB values to new intensity
    else if((int)old_intensity > (int)intensity){
      for(int i=old_intensity; i>intensity; i--){
          analogWrite(red_LED, i);
          analogWrite(green_LED, i/g_offset);
          analogWrite(blue_LED, i/b_offset);

          int barProg = map(i,0,255,0,19); //map current R value to progress bar position
          int oldBarProg = map(i+1,0,255,0,19); //map previous R value to progress bar position

          //If current bar progress is less than old bar progress, remove the unit at old bar progress position
          if(barProg<oldBarProg){
            lcd2.setCursor(oldBarProg,2);
            lcd2.print(" ");
          }
        
          delay(5);
       }
    }
    else if(R2 > 13000){
      analogWrite(red_LED,0);
      analogWrite(green_LED, 0);
      analogWrite(blue_LED, 0);
    }

    old_intensity = intensity;
    
    delay(100);
}


// Helper function to smooth out the light sensor data.
// Takes any number of readings and smoothes out the data to an average value.
// Returns 8-bit value (0-255).
int smooth(){
  int j;
  int value = 0;
  int numReadings = 10;

  for (j = 0; j < numReadings; j++){
    // Read light sensor data.
    value = value + analogRead(A0);

    // 1ms pause adds more stability between reads.
    delay(1);
  }

  // Take an average of all the readings.
  value = value / numReadings;

}

//Display static text for LCD displays. Only needs to be called once.
void displayLcdText(){
  //Display speed text for power generation station LCD
  lcd1.clear();
  lcd1.setCursor(6,1);
  lcd1.print("SPEED: ");
  lcd1.setCursor(0,2);
  lcd1.print("                    ");

  //Diplay brightness text for city lights 
  lcd2.clear();
  lcd2.setCursor(4,1);
  lcd2.print("BRIGHTNESS: ");
  lcd2.setCursor(0,2);
  lcd2.print("                    ");
  
  //Display resistance text for city lights
  lcd3.clear();
  lcd3.setCursor(4,1);
  lcd3.print("RESISTANCE: ");
  lcd3.setCursor(0,2);
  lcd3.print("                    ");
}
