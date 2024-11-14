const bool debugMode = 0;  //1 to use simulated camera exposure and show outputs
const bool acqLiveMode = 1; //1 to evoke this code only when acqLive signal is on, 0 to ignore acqLive signal
const int camrate = 2; //frame rate of camera [Hz] (used only in debugMode)
const int duration = 1000 / camrate; //[ms]
int tmod;

const int FVALPin = 5; //FIXED "Frame Valid" in TSI-IOBOB2
const int TrigInPin = 3; //FIXED "Trig In" in TSI-IOBOB2 or "tlExposeClock" from NIDAQ
const int PCOinPin = 4;//2;    //FIXED pin for "all lines exposing" specified in TSI-IOBOB2
const int acqLiveInPin = 13;//5;       // pin for acqLive from Timeline
const int extraGndPin = 1;       // 

const int blueOutPin = 8;//9;       // pin for blue's Gate1 
const int purpleOutPin = 9;//4;       // pin for purple's Gate1 
const int poissonPin = 10;          //pin for "flipper"
const int bklightPin = 11; //pin for backlight control 23/4/20

const int minPoissonDur = 10; //ms
const int maxPoissonDur = 200; //ms

bool flipflopState = 0;         // current state of the alternation
bool lastPCOstate = 0;
bool currentPCOstate = 0;
bool lastAcqLiveState = 0;
bool currentAcqLiveState = 0;
// internal variables
bool currentBlueInternalState = 0;
bool currentPurpleInternalState = 0;
// output to LED
bool currentBlueOutState = 0;
bool currentPurpleOutState = 0;

unsigned long lastFlipTime = 0;
unsigned long timeNow = 0;
unsigned long poissonStateDur = 0;
int currentPoissonState = 0;
bool currentTrigInstate = 0;
bool lastTrigInstate = 0;
bool currentFVALstate = 0;
bool lastFVALstate = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(FVALPin, INPUT);
  pinMode(TrigInPin, INPUT); //NOT OUTPUT
  pinMode(PCOinPin, INPUT);
  pinMode(blueOutPin, OUTPUT);
  pinMode(purpleOutPin, OUTPUT);
  pinMode(acqLiveInPin, INPUT);
  pinMode(bklightPin, OUTPUT);
  digitalWrite(bklightPin, LOW);
  pinMode(extraGndPin, OUTPUT);
  digitalWrite(extraGndPin, LOW);

  pinMode(poissonPin, OUTPUT);
  digitalWrite(poissonPin, LOW);
  currentPoissonState = LOW;
  poissonStateDur = random(minPoissonDur, maxPoissonDur);
  lastFlipTime = millis();

  if (debugMode == 1) {
    pinMode(PCOinPin, OUTPUT);
    pinMode(acqLiveInPin, OUTPUT);
    Serial.begin(9600);
  }

  if (acqLiveMode == 0) {
    digitalWrite(acqLiveInPin, LOW); //HIGH
  }
}


void loop() {
  // put your main code here, to run repeatedly:

  currentAcqLiveState = digitalRead(acqLiveInPin);
  timeNow = millis();

  if (debugMode == 1) {

    tmod = timeNow % duration;

    //rectanglar wave
    if (tmod > duration / 2) {
      digitalWrite(PCOinPin, HIGH);
    }
    else {
      digitalWrite(PCOinPin, LOW);
    }
  }


  if (currentAcqLiveState == HIGH & lastAcqLiveState == LOW) {
    flipflopState = 0; // guarantee that color is blue on first frame when acquisition starts.
    currentBlueInternalState = 1;
    currentPurpleInternalState = 0;
    }

  lastAcqLiveState = currentAcqLiveState;

  currentPCOstate = digitalRead(PCOinPin);

  if (currentPCOstate == HIGH & lastPCOstate == LOW) {
    flipflopState = (flipflopState + 1) % 2;

    if (flipflopState == 0) {
      currentBlueInternalState = 1;
      currentPurpleInternalState = 0;
    } else {
      currentBlueInternalState = 0;
      currentPurpleInternalState = 1;
    }
  }
  lastPCOstate = currentPCOstate;

currentFVALstate = digitalRead(FVALPin);
  if (currentFVALstate == LOW & lastFVALstate == HIGH) {
    digitalWrite(bklightPin, HIGH); //24/4/20
  }
  
  lastFVALstate = currentFVALstate;

  //code for flipper
  if (currentAcqLiveState == HIGH && (timeNow - lastFlipTime) > poissonStateDur) {
    lastFlipTime = timeNow;
    if (currentPoissonState == LOW) {
      currentPoissonState = HIGH;
    } else {
      currentPoissonState = LOW;
    }
    digitalWrite(poissonPin, currentPoissonState);
    poissonStateDur = random(minPoissonDur, maxPoissonDur);
  } else if (timeNow < lastFlipTime) {
    // this can only happen when the millis() function wraps around at the limit of the unsigned long datatype
    lastFlipTime = 1; // pretend we flipped at 1
  }

  //code for backlight 23/4/20
  currentTrigInstate = digitalRead(TrigInPin);
  if (currentTrigInstate == HIGH && lastTrigInstate == LOW) {
    digitalWrite(bklightPin, LOW);
  }

  lastTrigInstate = currentTrigInstate;

  // AND operation
  digitalWrite(blueOutPin, currentBlueInternalState & currentPCOstate);
  digitalWrite(purpleOutPin, currentPurpleInternalState & currentPCOstate);

  currentBlueOutState = digitalRead(blueOutPin);
  currentPurpleOutState = digitalRead(purpleOutPin);

  if (debugMode == 1) {
    //Serial.print(tmod); Serial.print(" ");
    Serial.print(currentPCOstate); Serial.print(" ");
    Serial.print(currentAcqLiveState); Serial.print(" ");
    Serial.print(currentBlueOutState); Serial.print(" ");
    Serial.println(currentPurpleOutState);
  }

  delayMicroseconds(50);
  //}
}
