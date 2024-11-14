

const byte OUTPIN = 12;
const unsigned int onDuration = 20; //[ms]
const unsigned int offDuration = 20; // [ms]


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(OUTPIN, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(OUTPIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(onDuration);                       // wait for a second
  digitalWrite(OUTPIN, LOW);    // turn the LED off by making the voltage LOW
  delay(offDuration);                       // wait for a second
}
