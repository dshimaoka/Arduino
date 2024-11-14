unsigned int onDuration = 5; //[us] //min should be 62.5ns but somehow cannot make smaller than 4us. max4000us for timer1


const int inPin = 2;    // input from 2p [fixed] should be 2 or 3 for uno
const int outPin = 8;       // output to screen [fixed]

void setup()
{

  cli(); //stop interrputs 19/8/19

  pinMode(inPin, INPUT); //portD 0-7 ... maybe slow?
  pinMode(outPin, OUTPUT); //portB 8-13

  // Disable a few unused interrupts
  TIFR0 = 0;
  TIFR2 = 0;
  UCSR0B = 0;
  TCCR0B = 0; //this helps to reduce jitter https://forum.arduino.cc/index.php?topic=206513.0
  TCCR0A = 0;//this helps to reduce jitter https://forum.arduino.cc/index.php?topic=206513.0
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B

  // initialize output port
  PORTB &= 0;

  // for timer1
  TCCR1A = 0;           // set entire TCCR1A register to 0
  //TCCR1A = (1<<COM1A1)| (1<<WGM11); 
  TCCR1B = 0;          // input capture noise canceller disabled, capture on falling edge (may adjust this later), stop timer
  TCCR1B = (1 << ICNC1); //enable noise reduction
  TCCR1B |= (1 << WGM12); //turn on CTC mode 19/8/19
  //TCCR1B |= (1 << WGM13); //turn on CTC mode 19/8/19 ??
  //ACSR = 0;             // input capture NOT from analog comparator
  TCNT1 = 0; // initialize counter value to 0
  OCR1A = 16 * onDuration;  // set compare match register
  //ICR1 = 1; // // set compare match register ??
  TIMSK1 = 0;          // timer 1 interrupts disabled
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt

  attachInterrupt(digitalPinToInterrupt(inPin), startOutput, RISING); //takes ~3us to evoke

  sei(); //start interrupt
}


void startOutput() { // start timer1
  PORTB |= 1; // OUTPUT to be HIGH

  TCNT1 = 0; // initialize counter value to 0
  TIFR1 = 1 << ICF1;            // clear input capture bit

  if (digitalRead(inPin) == HIGH) { //sort of debouncing 19/8/19
    TCCR1B |= (1 << CS10);     // start timer, prescaler = 1
  }
}

ISR(TIMER1_COMPA_vect) { // stop timer1 ... takes ~3us https://www.avrfreaks.net/forum/timercounter1-output-compare-interrupt-too-slow
  PORTB &= 0;  // OUTPUT to be LOW
  TCCR1B = 0;        // stop timer 1?
}

//ISR(TIMER1_OVF_vect) { //this may be more quicker but then should use timer0 or 2
//  PORTB &= 0;  // OUTPUT to be LOW
//  TCCR1B = 0;        // stop timer 1?  
//}


void loop()
{
}
