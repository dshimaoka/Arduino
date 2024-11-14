// this code uses Timer1 with prescaler 8, 64, or 256
// needs digitalWriteFast
// Timer with an ISR example
// Author: Nick Gammon
// Date: 13th January 2012
// Modified: 19 October 2014
// added "polarity" 1/9/2019 DS
// added "detectEdge" 2/9/2019 DS
// assume timer1 prescaler = 1 
// added detectEdge == 2 (both rising and falling edges)
// TODO sparks at just before rising and falling (Keller)

#include <digitalWriteFast.h>

const boolean polarity = 1; //1: spark = HIGH, 0:spark = LOW
const unsigned int detectEdge = 1; //2: detect rising or falling edge; 1:detect rising edge, 0:detect falling edge

volatile unsigned int sparkDelayTime = 39;   // microseconds. min 10 - max 4000
volatile unsigned int sparkOnTime = 8;     // microseconds. min8 - max 4000
volatile unsigned int scannerOnTime = 84 - sparkOnTime; //microseconds. slightly larger than 1/12*1e3 18/2/20

const byte FIRE_SENSOR = 2;  // this port corresponds to interrupt 0 (for INT0_vect)
const byte SPARKPLUG = 9;

// allow for time taken to enter ISR (determine empirically)
const unsigned int isrDelayFactor = 4;        // microseconds 4

// is spark currently on?
volatile boolean sparkOn;

volatile unsigned int prescaler_delay;
volatile unsigned int prescaler_duration;


void setup()
{
  TCCR1A = 0;  // normal mode
  TCCR1B = 0;  // stop timer
  TIMSK1 = 0;  // cancel timer interrupt

  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B

  EIMSK |= (1 << INT0);     // Enable external interrupt INT0
  EICRA &= ~(bit(ISC00) | bit (ISC01));  // clear existing flags

  if (detectEdge == 0) {
    // Trigger INT0 on falling edge
    EICRA |= (1 << ISC01);
    EICRA |= (0 << ISC00);
  }
  else if (detectEdge == 1) {
    // Trigger INT0 on rising edge
    EICRA |= (1 << ISC01);
    EICRA |= (1 << ISC00);
  }
  else if (detectEdge == 2){
    //Trigger INT0 on rising or falling edge
    EICRA |= (0 << ISC01);
    EICRA |= (1 << ISC00);
  }

  pinMode (SPARKPLUG, OUTPUT);
  pinMode (FIRE_SENSOR, INPUT_PULLUP);

  activateInterrupt0 ();

// start with CTC mode
     TCCR1B |= (1 << WGM12) | bit(CS10);  //CTC prescaler 1 
      OCR1A = 1000;  
  sparkOn = 0; //18/2/20
  TIMSK1 = bit (OCIE1A);  //start with ISR (TIMER1_COMPA_vect) 18/2/20

}  // end of setup


void activateInterrupt0 ()
{
  EICRA &= ~(bit(ISC00) | bit (ISC01));  // clear existing flags
  if (detectEdge == 0) {
    EICRA |= (1 << ISC01);
    EICRA |= (0 << ISC00);
  }
  else if (detectEdge == 1) {
    EICRA |= (1 << ISC01);
    EICRA |= (1 << ISC00);
  }
  else if (detectEdge == 2){
    //Trigger INT0 on rising or falling edge
    EICRA |= (0 << ISC01);
    EICRA |= (1 << ISC00);
  }


  EIFR   =  bit (INTF0);    // clear flag for interrupt 0
  EIMSK |=  bit (INT0);     // enable it

 }  // end of activateInterrupt0

void deactivateInterrupt0 ()
{
  EIMSK &= ~bit (INT0);     // disable it
}  // end of deactivateInterrupt0


// specify delay here
ISR (INT0_vect) // start timer for delay parameter
{
    sparkOn = false;                  // make sure flag off just in case

  if (sparkDelayTime == 0){
   if (polarity)
    {
      digitalWriteFast (SPARKPLUG, HIGH);
    }
    else
    {
      digitalWriteFast (SPARKPLUG, LOW);
    }

      TCNT1 = 0;                          // count back to zero
    OCR1A = (sparkOnTime * 16) - (isrDelayFactor * 16) - 1;
sparkOn = 1;
   
  }
  else {
    //TCCR1B = 0;                         // stop timer 18/2/20
    //TIMSK1 = 0;                         // cancel timer interrupt 18/2/20


    // set up Timer 1
    TCCR1A = 0;                       // normal mode
    TCNT1 = 0;                        // count back to zero

    OCR1A = (sparkDelayTime * 16) - (isrDelayFactor * 16) - 1;  
    TIMSK1 = bit (OCIE1A);            // interrupt on Compare A Match
  }
    deactivateInterrupt0 ();          // no more interrupts yet

   
} // end of ISR (INT0_vect)



//specify duration here
ISR (TIMER1_COMPA_vect)
{

  if (sparkOn == 0) // start timer for duration
  {
    if (polarity)
    {
      digitalWriteFast (SPARKPLUG, HIGH);
    }
    else
    {
      digitalWriteFast (SPARKPLUG, LOW);
    }
    //TCCR1B = 0;                         // stop timer
    TCNT1 = 0;                          // count back to zero

    OCR1A = (sparkOnTime * 16) - (isrDelayFactor * 16) - 1;
    //sparkOn = 1;
  }
  else //called when spark is finished
  {  
    if (polarity)
    {
      digitalWriteFast (SPARKPLUG, LOW);
    }
    else
    {
      digitalWriteFast (SPARKPLUG, HIGH);
    }
    //TCCR1B = 0;                         // stop timer
    //TIMSK1 = 0;                         // cancel timer interrupt
     
      // reset the timer with a new CTC counter (assume prescaler_duration ==1)
     TCNT1 = 0;                          // count back to zero
     OCR1A = (scannerOnTime * 16) - (isrDelayFactor * 16) - 1; 

    activateInterrupt0 ();              // re-instate interrupts for firing time
    //sparkOn = 0;
  }

  sparkOn = !sparkOn;                  // toggle

}  // end of TIMER1_COMPA_vect



void loop()
{
  // read sensors, compute time to fire spark

  if (false)  // if we need to change the time, insert condition here ...
  {
    noInterrupts ();        // atomic change of the time amount
    //sparkDelayTime = 500;   // delay before spark in microseconds
    //sparkOnTime = 2000;     // spark on time in microseconds
    interrupts ();
  }
}  // end of loop
