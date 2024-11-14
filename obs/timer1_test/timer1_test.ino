const int flybackDuration = 657; //[us] ThorImage>Capture Setup>Galvo/Resonance Scanner Control>Flyback Cycles
const int margin = 50; //[us]
unsigned int onDuration;

int currentInput = 0;
int previousInput = 0;

const int inPin = 1;    // input from 2p [ch]
const int outPin = 2;       // output to screen [fixed]

void setup()
{
  onDuration = flybackDuration - margin; //[ms]
  pinMode(inPin, INPUT); //portB 8-13 ... maybe slow?
  pinMode(outPin, OUTPUT); //portD 0-7

  // for timer1
  TCCR1A = 0;
  TCCR1B = 0;          // input capture noise canceller disabled, capture on falling edge (may adjust this later), stop timer
  TIMSK1 = 0;          // timer 1 interrupts disabled
  ACSR = 0;             // input capture NOT from analog comparator
}

void loop()
{
  static bool posEdge = true;

  TCCR1B = (posEdge) ? (1 << ICES1) : 0;        // set up timer 1 to capture on whichever edge we want and stop timer
  TCNT1H = 0;
  TCNT1L = 0;          // clear timer 1
  unsigned long start = micros();  // get the time

  cli();
  TIFR1 = 1 << ICF1;            // clear input capture bit

  //currentInput = digitalRead(inPin); //this is slow
  //currentInput = bitRead(PORTB, 0); //does not work?
  currentInput = PORTB;

  if (currentInput == 1 && previousInput == 0) {
    TCCR1B |= (1 << CS10);     // start timer, prescaler = 1
    PORTD |= (1 << 2);          // set output high
    sei();

    unsigned int capture = 0;
    do
    {
      if ((TIFR1 & (1 << ICF1)) != 0)
      {
        byte temp = ICR1L;
        capture = (ICR1H << 8) | temp;
      }
    } while (capture == 0 && micros() - start < 100);     // time out after 100us

    PORTD &= ~(1 << 2);     // set output low
  }

  else {
    PORTD &= ~(1 << 2);     // set output low
  }

  previousInput = currentInput;
}
