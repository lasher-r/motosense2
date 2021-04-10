//arduino bike speedometer w serial.print()
//by Amanda Ghassaei 2012
//https://www.instructables.com/id/Arduino-Bike-Speedometer/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
*/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//calculations
//tire radius ~ 13.5 inches
//circumference = pi*2*r =~85 inches
//max speed of 35mph =~ 616inches/second
//max rps =~7.25

const int reed = 2; 
const int ledPin =  13;      // the number of the board LED pin
const int interruptPin = 3;

//storage variables
 // time between one full rotation (in ms)
float radius = 13.5; // tire radius (in inches)
float circumference;
int minDurRotationMs = 100; //min time (in ms) of one rotation (for debouncing)
int maxStandingTime = 2000;

long durOfRotationMs;
float inches;
float mph;
bool lastWasHigh;

void setup(){
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  
  lastWasHigh = true;
  circumference = 2*3.14*radius;
  inches = 0.0;
  
  pinMode(reed, INPUT);
  pinMode(ledPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), resetOdo, RISING);

  
  // TIMER SETUP- the timer interrupt allows precise timed measurements of the reed switch
  //for more info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1

//   R- Link didn't help.  Idk what this code does, taking on faith that it's calling ISR every ms
//  better link: https://www.instructables.com/Arduino-Timer-Interrupts/

  cli();//stop interrupts
  //set timer1 interrupt at 1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;
  // set timer count for 1khz increments
  OCR1A = 1999;// = (1/1000) / ((1/(16*10^6))*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei();//allow interrupts
  //END TIMER SETUP
  
  Serial.begin(9600);
}

void displayOled() {
  display.clearDisplay();

  display.setTextSize(2);             
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("MPH: "));
  display.println(mph);
  display.print(F("ODO: "));
  display.println(inches / 5280);
  display.display();
}

void resetOdo() {
  inches = 0.0;
}

/**
 * Interrupt at freq of 1kHz to measure reed switch
 * 
 * Might be worth getting pin value directly
 * See: https://www.arduino.cc/en/Reference/PortManipulation
 * 
 * if reed switch is closed 
 * and enough time has passed that we're sure it's a new rotation
 * and we're not stuck at the mag/moving at exactly 1kHz,
 * calculate mph and add to odometer,
 * reset the time it takes for the wheel to turn,
 * 
 * 
 * if the time this rotation is taking > the maxStandingTime,
 * the wheel is stoped.  set mph to 0
 * 
 * save the value of the reed switch in lastWasHigh
 * Add one ms to the time this rotation is taking.
 * 
 */
ISR(TIMER1_COMPA_vect) {
  bool reedValue = digitalRead(reed);
  digitalWrite(ledPin, reedValue);

  if (reedValue && durOfRotationMs > minDurRotationMs && !lastWasHigh){

    mph = (56.8*float(circumference))/float(durOfRotationMs);
    inches += float(circumference);
    durOfRotationMs = 0;
  }

  if (durOfRotationMs > maxStandingTime){
    mph = 0;
  }

  lastWasHigh = reedValue;
  durOfRotationMs += 1; 
}

void displaySerial(){
  Serial.print("mph: ");
  Serial.print(mph);
  Serial.print(" miles: ");
  Serial.println(inches / 5280);
}

void loop(){
  //print mph once a second
  displayOled();
  delay(500);
}
