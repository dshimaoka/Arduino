// two independent outputs is a bad idea?
// when not driven, the interval between the two is not constant

// IN pin2, OUT pin5,  timer0 detect FALL with INT0
// IN pin3, OUT pin10, timer1 detect RISE with INT1

// VARIABLE PARAMETERS
const unsigned int sparkCentre = 1; 
//if 1, sparkOn center  = input flip time(sparkDelay param is meaningless)
//if 0  sparkDelay specifies time of sparkOn after input flip

volatile unsigned int sparkOnTime = 2; //[us] %min2
volatile unsigned int sparkDelayTime_r =38; //[us] if 0, disable triggering by rising edge
volatile unsigned int sparkDelayTime_f = 38; //[us] if 0. disable triggerinb by falling edge


// FIXED PARAMETERS
const unsigned int duty = 42; //[us]
const unsigned int isrDelay = 4; // [us]
const unsigned int count2us_r = 2; //for timer0 prescaler8
const unsigned int count2us_f = 16; //for timer1 
const byte FIRE_SENSOR_r = 2;  // this port corresponds to interrupt 0 (for INT0_vect)
const byte FIRE_SENSOR_f = 3;  // this port corresponds to interrupt 0 (for INT1_vect)

void setup()
  {

  TCCR1A = 0;  // normal mode
  TCCR1B = 0;  // stop timer
  TIMSK1 = 0;  // cancel timer interrupt

  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TIMSK2 = 0;  // cancel timer interrupt

  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B
  TIMSK0 = 0;  // cancel timer interrupt


if (sparkDelayTime_r>0){
// PD5 (=pin5) as output for timer0
  DDRD|= (1<<PD5);

  TCCR0A = bit (COM0B0) | bit (COM0B1)  // Set OC1B on Compare Match, clear
                                        // OC1B at BOTTOM (inverting mode)
         | bit (COM0A1)                 // Clear OC1A on Compare Match, set
                                        // OC1A at BOTTOM (non-inverting mode) ??
         | bit (COM0A0)               // inverting mode                
         | (0 << WGM01) | (1 << WGM00); // phase correct PWM, top at OCR1A 
  TCCR0B = bit (WGM02)  
         | bit (CS01);                  // Start timer, prescaler of 8

  //INT0 settings
  EICRA &= ~(bit(ISC00) | bit (ISC01));  // clear existing flags
   pinMode (FIRE_SENSOR_r, INPUT_PULLUP);
    // Trigger INT0 on rise
    EICRA |= (1 << ISC01);
    EICRA |= (1 << ISC00);
  
   EIFR   =  bit (INTF0);    // clear flag for interrupt 0
   EIMSK |=  bit (INT0);     // enable it
}


if (sparkDelayTime_f>0){
// PB2 (=pin10) as output for timer1
  DDRB|= (1<<PB2);
  TCCR1A = bit (COM1B0) | bit (COM1B1)  // Set OC1B on Compare Match, clear
                                        // OC1B at BOTTOM (inverting mode)
         | bit (COM1A1)                 // Clear OC1A on Compare Match, set
                                        // OC1A at BOTTOM (non-inverting mode) ??
         | bit (COM1A0)               // inverting mode                
         | (0 << WGM11) | (1 << WGM10); // phase correct PWM, top at OCR1A
  TCCR1B = (0 << WGM12) | bit(WGM13)
         | bit (CS10);                  // Start timer, prescaler of 1

// INT1 settings
   pinMode(FIRE_SENSOR_f,INPUT_PULLUP);
    EICRA |= (1 << ISC11);     // Trigger INT1 on fall
    EICRA |= (0 << ISC10);    // Trigger INT1 on fall
  
   EIFR  |=  bit (INTF1);    // clear flag for interrupt 0
   EIMSK |=  bit (INT1);     // enable it

}

   
// set delays and on durations
if (sparkCentre){
   OCR0A = count2us_r*(duty - isrDelay/2); //ON center coincides tith flip time
   OCR1A = count2us_f*(duty - isrDelay/2);
}
else if (sparkCentre ==0){
   OCR0A = count2us_r*(sparkDelayTime_r + sparkOnTime/2 -isrDelay/2);
   OCR1A = count2us_f*(sparkDelayTime_f + sparkOnTime/2 -isrDelay/2); //top value
}
   OCR0B = OCR0A - count2us_r*(sparkOnTime/2);
   OCR1B = OCR1A - count2us_f*(sparkOnTime/2);

  TCNT0 = 0; //reset counter
  TCNT1 = 0;
  }  // end of setup


ISR (INT0_vect) //takes 4us to enter this process
{
  TCNT0 = 0;         //reset counter
}

ISR (INT1_vect) //takes 4us to enter this process
{
    TCNT1 = 0;         //reset counter
}
void loop()
  {
  }
