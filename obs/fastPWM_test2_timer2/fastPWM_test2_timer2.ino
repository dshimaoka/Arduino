// BUG: 2*driven freq = spt_freq ... should be driven = spt

const unsigned int count2us = 2; 
const unsigned int detectEdge =1; //2: detect rising or falling edge; 1:detect rising edge, 0:detect falling edge
//weird behavior

volatile unsigned int sparkOnTime = 4; //[us] min4
volatile unsigned int sparkDelayTime = 40; //[us] %delay after sparkoff min40


const byte FIRE_SENSOR = 2;  // this port corresponds to interrupt 0 (for INT0_vect)

void setup()
  {

    TCCR1A = 0;  // normal mode
  TCCR1B = 0;  // stop timer
  TIMSK1 = 0;  // cancel timer interrupt

  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B

  //DDRB |=  bit (DDB2); //26/2/20
  // PD5 as output
  DDRD|= (1<<PD5);

  // stop timer 0
  TCCR0A = 0;
  TCCR0B = 0;
  
  TCCR0A = bit (COM0B0) | bit (COM0B1)  // Set OC1B on Compare Match, clear
                                        // OC1B at BOTTOM (inverting mode)
         | bit (COM0A1)                 // Clear OC1A on Compare Match, set
                                        // OC1A at BOTTOM (non-inverting mode) ??
         | bit (COM0A0)               // inverting mode                
         | bit (WGM01)                
         | bit (WGM00); // Fast PWM, top at OCR1A (mode15)
  TCCR0B = bit (WGM02)  // | bit (WGM03)   //       ditto
         | bit (CS01);                  // Start timer, prescaler of 8
  
  //INT0 settings
  EICRA &= ~(bit(ISC00) | bit (ISC01));  // clear existing flags

   pinMode (FIRE_SENSOR, INPUT_PULLUP);

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
   EIFR   =  bit (INTF0);    // clear flag for interrupt 0
   EIMSK |=  bit (INT0);     // enable it

   OCR0A = count2us*(sparkDelayTime + sparkOnTime); //top value
   OCR0B = OCR0A - count2us*(sparkOnTime);


  }  // end of setup


// sometimes this interruption does not happen around timer reaches top value
ISR (INT0_vect) //takes 4us to enter this process
{
//  if (((detectEdge == 1) && digitalRead(FIRE_SENSOR == HIGH)) ||  //
//  ((detectEdge == 0) && digitalRead(FIRE_SENSOR == LOW))) { // debouncing ... does not work
 
    TCNT0 = 0;         //reset counter

//    OCR1A = count2us*(sparkDelayTime + sparkOnTime); //top value
//    OCR1B = count2us*(sparkDelayTime);

    TCCR0B = bit (WGM02)//  | bit (WGM13)   //       ditto
         | bit (CS01);                 // Start timer, prescaler of 8
        
//  }
}
void loop()
  {
  }
