// created from ignite_wDelay_allOpt

#include "digitalWriteFast.h"

const boolean polarity = 0; //1: spark = HIGH, 0:spark = LOW
const boolean detectEdge = 0; //1:detect rising edge, 0:detect falling edge

// camera parameter
const unsigned int nRows = 900;
const unsigned int exposureTime = 10; //[ms]
const unsigned int busyTime = 1000; //[us] guess!
const unsigned int lineTime = 12.136; //[us] fixed

// timeline parameter
const unsigned int frameRate = 30; //[Hz]

volatile unsigned int sparkDelayTime = nRows*lineTime;   // microseconds. min ~100 max 1024000 (=1.024s)
volatile unsigned int sparkOnTime = ceil(1e6/frameRate) - 1e3*exposureTime - sparkDelayTime - busyTime;     // microseconds. max 1024000 (=1.024s)

const byte FIRE_SENSOR = 2;  // this port corresponds to interrupt 0 (for INT0_vect)
const byte SPARKPLUG = 9;

// allow for time taken to enter ISR (determine empirically)
const unsigned int isrDelayFactor = 4;        // microseconds

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
  else {
    // Trigger INT0 on rising edge
    EICRA |= (1 << ISC01);
    EICRA |= (1 << ISC00);
  }

  if (sparkDelayTime < 32000) {
    prescaler_delay = 8;
  }
  else if (sparkDelayTime < 256000) {
    prescaler_delay = 64;
  }
  else if (sparkDelayTime < 1024000) {
    prescaler_delay = 256;
  }

  if (sparkOnTime < 32000) {
    prescaler_duration = 8;
  }
  else if (sparkOnTime < 256000) {
    prescaler_duration = 64;
  }
  else if (sparkOnTime < 1024000) {
    prescaler_duration = 256;
  }

  pinMode (SPARKPLUG, OUTPUT);
  pinMode (FIRE_SENSOR, INPUT_PULLUP);

  activateInterrupt0 ();
}  // end of setup


void activateInterrupt0 ()
{
  EICRA &= ~(bit(ISC00) | bit (ISC01));  // clear existing flags
  //EICRA |=  bit (ISC01);    // set wanted flags (falling level interrupt)

  if (detectEdge == 0) {
    EICRA |= (1 << ISC01);
    EICRA |= (0 << ISC00);
  }
  else {
    EICRA |= (1 << ISC01);
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
  if (((polarity == 1) && digitalRead(FIRE_SENSOR == HIGH)) ||  ((polarity == 0) && digitalRead(FIRE_SENSOR == LOW))) { // debouncing
    sparkOn = false;                  // make sure flag off just in case

    // set up Timer 1
    TCCR1A = 0;                       // normal mode
    TCNT1 = 0;                        // count back to zero

    if (prescaler_delay == 8) {
      TCCR1B = bit(WGM12) | bit(CS11);  // CTC, scale to clock / 8
      // time before timer fires - zero relative
      OCR1A = (sparkDelayTime * 2) - (isrDelayFactor * 2) - 1;
    }
    else if (prescaler_delay == 64) {
      TCCR1B = bit(WGM12) | bit(CS11) | bit(CS10);  // CTC, scale to clock / 64
      OCR1A = (sparkDelayTime / 4) - (isrDelayFactor / 4) - 1;
    }
    else if (prescaler_delay == 256) {
      TCCR1B = bit(WGM12) | bit(CS12);
      OCR1A = (sparkDelayTime / 16) - (isrDelayFactor / 16) - 1;
    }
    //OCR1A = ceil((sparkDelayTime * 16 / prescaler_delay) - (isrDelayFactor * 16 / prescaler_delay) - 1); ng


    TIMSK1 = bit (OCIE1A);            // interrupt on Compare A Match

    deactivateInterrupt0 ();          // no more interrupts yet
  }
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
    TCCR1B = 0;                         // stop timer
    TCNT1 = 0;                          // count back to zero

    if (prescaler_duration == 8) {
      TCCR1B = bit(WGM12) | bit(CS11);    // CTC, scale to clock / 8
      // time before timer fires (zero relative)
      // multiply by two because we are on a prescaler of 8
      OCR1A = (sparkOnTime * 2) - (isrDelayFactor * 2) - 1;
    }
    else if (prescaler_duration == 64) {
      TCCR1B = bit(WGM12) | bit(CS11) | bit(CS10);    // CTC, scale to clock / 64
      // time before timer fires (zero relative)
      // multiply by two because we are on a prescaler of 64
      OCR1A = (sparkOnTime / 4) - (isrDelayFactor / 4) - 1;
    }
    else if (prescaler_duration == 256) {
      TCCR1B = bit(WGM12) | bit(CS12);    // CTC, scale to clock / 64
      OCR1A = (sparkOnTime / 16) - (isrDelayFactor / 16) - 1;
    }
  }
  else
  {
    if (polarity)
    {
      digitalWriteFast (SPARKPLUG, LOW);
    }
    else
    {
      digitalWriteFast (SPARKPLUG, HIGH);
    }
    TCCR1B = 0;                         // stop timer
    TIMSK1 = 0;                         // cancel timer interrupt
    activateInterrupt0 ();              // re-instate interrupts for firing time
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
