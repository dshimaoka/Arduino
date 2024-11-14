
const unsigned int count2us = 16; //18 theoretical value assuming 16Mhz & prescaler1

const unsigned int detectEdge =1; //2: detect rising or falling edge; 1:detect rising edge, 0:detect falling edge
//weird behavior

volatile unsigned int sparkOnTime = 7; //[us]
volatile unsigned int sparkDelayTime = 45; //[us] %delay after sparkoff


// allow for time taken to enter ISR (determine empirically)
const unsigned int isrDelayFactor = 4;        //1-4 microseconds

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

  // Defining PB1 as outputs by setting PORTB1 
  // Setting DDB1
  DDRB |= bit (DDB1);// | bit (DDB2);
 
  // stop timer 1
  TCCR1A = 0;
  TCCR1B = 0;
  
  TCCR1A = bit (COM1B0) | bit (COM1B1)  // Set OC1B on Compare Match, clear
                                        // OC1B at BOTTOM (inverting mode)
         | bit (COM1A1)                 // Clear OC1A on Compare Match, set
                                        // OC1A at BOTTOM (non-inverting mode) ??
         | bit (COM1A0)               // inverting mode                
         | bit (WGM11);                 // Fast PWM, top at ICR1
  TCCR1B = bit (WGM12)  | bit (WGM13)   //       ditto
         | bit (CS10);                  // Start timer, prescaler of 1

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

  // Initialize OCR1A = 300 (pulse_width = 150us), OCR1B, and ICR1
  ICR1 = count2us*(sparkOnTime + sparkDelayTime);//prescaler1 750=12kHz
  OCR1A = count2us*(sparkDelayTime);


  }  // end of setup


// sometimes this interruption does not happen around timer reaches top value
ISR (INT0_vect) //takes 4us to enter this process
{
//  if (((detectEdge == 1) && digitalRead(FIRE_SENSOR == HIGH)) ||  //
//  ((detectEdge == 0) && digitalRead(FIRE_SENSOR == LOW))) { // debouncing ... does not work
 
 noInterrupts();     
    //TCCR1B = (0 << CS00);                  // Stop timer
     TCNT1 = 0;         //reset counter
    OC1A = 0;
    
    ICR1 = count2us*(sparkOnTime + sparkDelayTime  - isrDelayFactor);//prescaler1 750=12kHz
  
    TCCR1B = bit (WGM12)  | bit (WGM13)   //       ditto
         | bit (CS10);                  // Start timer, prescaler of 1

    //EIFR   =  bit (INTF0);    // clear flag for interrupt 0
    //EIMSK |=  bit (INT0);     // enable it
interrupts();  
//  }
}
void loop()
  {
  }
