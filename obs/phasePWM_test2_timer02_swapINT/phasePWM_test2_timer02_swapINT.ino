// two independent outputs is a bad idea?
// when not driven, the interval between the two is not constant

// IN pin2, OUT pin5,  timer0 detect FALL with INT0
// IN pin3, OUT pin10, timer1 detect RISE with INT1

volatile unsigned int sparkOnTime =1; //[us] x2 will be the actual duration min3
volatile unsigned int sparkDelayTime_r =40; //[us] %center of delay after input ON min39 UNSTABLE below 40

// X INT1 is more stable than INT0?
// timer0 is less stable than timer1

// below sholud be fixed
const unsigned int count2us_r = 2; //for timer0 prescaler8
const unsigned int count2us_f = 16; //for timer1 
const byte FIRE_SENSOR = 2;  // this port corresponds to interrupt 0 (for INT0_vect)

void setup()
  {

  TCCR1A = 0;  // normal mode
  TCCR1B = 0;  // stop timer
  TIMSK1 = 0;  // cancel timer interrupt
  TIMSK0 = 0;

  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCCR0A = 0;// set entire TCCR0A register to 0
  TCCR0B = 0;// same for TCCR0B


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

// INT1 settings
   pinMode(3,INPUT_PULLUP);
    EICRA |= (1 << ISC11);     // Trigger INT1 on fall
    EICRA |= (0 << ISC10);    // Trigger INT1 on fall
  
   EIFR  =  bit (INTF1);    // clear flag for interrupt 0
   EIMSK =  bit (INT1);     // enable it

}


   
// set delays and on durations
   OCR0A = count2us_r*(sparkDelayTime_r + sparkOnTime); //top value
   OCR0B = OCR0A - count2us_r*(sparkOnTime);

OCR1A =0;
OCR1B=0;
 
  TCNT0 = 0; //reset counter
  TCNT1 = 0;
  }  // end of setup


//// sometimes this interruption does not happen around timer reaches top value
ISR (INT1_vect) //takes 4us to enter this process
{
    TCNT0 = 0;         //reset counter
}
void loop()
  {
  }
