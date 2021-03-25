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

//calculations
//tire radius ~ 13.5 inches
//circumference = pi*2*r =~85 inches
//max speed of 35mph =~ 616inches/second
//max rps =~7.25

#define reed 12 //pin connected to read switch
const int ledPin =  13;      // the number of the LED pin

//storage variables
// int reedVal;
long durOfRotationMs; // time between one full rotation (in ms)
float mph;
float radius = 13.5; // tire radius (in inches)
float circumference;

bool lastWasHigh;
int minDurRotationMs = 100; //min time (in ms) of one rotation (for debouncing)
int maxStandingTime = 2000;


void setup(){
  
  circumference = 2*3.14*radius;
  pinMode(reed, INPUT);
  pinMode(ledPin, OUTPUT);

  
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


ISR(TIMER1_COMPA_vect) {//Interrupt at freq of 1kHz to measure reed switch
  //Might be worth getting pin value directly
  //      See: https://www.arduino.cc/en/Reference/PortManipulation
  bool reedValue = digitalRead(reed);
  digitalWrite(ledPin, reedValue);

  // if reed switch is closed 
  //  and enough time has passed that we're sure it's a new rotation
  //  and we're not stuck at the mag/moving at exactly 1kHz    
  if (reedValue && durOfRotationMs > minDurRotationMs && !lastWasHigh){
    //calculate miles per hour
    //TODO: check math
    mph = (56.8*float(circumference))/float(durOfRotationMs);
    //reset counter
    durOfRotationMs = 0;
  }

  lastWasHigh = reedValue;

  if (durOfRotationMs > maxStandingTime){
    //if no new pulses from reed switch- tire is still, set mph to 0
    mph = 0;
  }

  // add another ms to counter
  durOfRotationMs += 1; 
}

void displayMPH(){
  Serial.println(mph);
}

void loop(){
  //print mph once a second
  displayMPH();
  delay(1000);
}
