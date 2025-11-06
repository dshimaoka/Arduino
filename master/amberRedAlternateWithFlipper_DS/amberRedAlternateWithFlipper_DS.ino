// created from amberredAlternateWithFlipper_DS
// blue > amber, amber > red

const bool debugMode = 0;  //1 to use simulated camera exposure and show outputs
const bool acqLiveMode = 1; //1 to evoke this code only when acqLive signal is on, 0 to ignore acqLive signal
const int camrate = 2; //frame rate of camera [Hz] (used only in debugMode)
const int duration = 1000 / camrate; //[ms]
int tmod;

const int PCOinPin = 13;//2;    // pin for "all lines exposing" 
const int copyPCOinPin = 12;//2;    // copy of PCOinPin
const int amberOutPin = 6;//9;       // pin for amber's Gate1 
const int redOutPin = 9;//4;       // pin for red's Gate1 
const int acqLiveInPin = 7;//5;       // pin for acqLive from Timeline
const int extraGndPin = 1;       // pin for acqLive from Timeline

const int poissonPin = 11;
const int minPoissonDur = 10; //ms
const int maxPoissonDur = 200; //ms

bool flipflopState = 0;         // current state of the alternation
bool lastPCOstate = 0;
bool currentPCOstate = 0;
bool lastAcqLiveState = 0;
bool currentAcqLiveState = 0;
// internal variables
bool currentamberInternalState = 0;
bool currentredInternalState = 0;
// output to LED
bool currentamberOutState = 0;
bool currentredOutState = 0;

unsigned long lastFlipTime = 0;
unsigned long timeNow = 0;
unsigned long poissonStateDur = 0;
int currentPoissonState = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(PCOinPin, INPUT);
  pinMode(copyPCOinPin, OUTPUT);
  pinMode(amberOutPin, OUTPUT);
  pinMode(redOutPin, OUTPUT);
  pinMode(acqLiveInPin, INPUT);
  pinMode(extraGndPin, OUTPUT);
  Serial.begin(9600);
  digitalWrite(extraGndPin, LOW);

  pinMode(poissonPin, OUTPUT);
  digitalWrite(poissonPin, LOW);
  currentPoissonState = LOW;
  poissonStateDur = random(minPoissonDur, maxPoissonDur);
  lastFlipTime = millis();

  if (debugMode == 1) {
    pinMode(PCOinPin, OUTPUT);
    pinMode(acqLiveInPin, OUTPUT);
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
    flipflopState = 0; // guarantee that color is amber on first frame when acquisition starts.
    currentamberInternalState = 1;
    currentredInternalState = 0;
    }

  lastAcqLiveState = currentAcqLiveState;

  currentPCOstate = digitalRead(PCOinPin);
  digitalWrite(copyPCOinPin, currentPCOstate);

  if (currentPCOstate == HIGH & lastPCOstate == LOW) {
    flipflopState = (flipflopState + 1) % 2;

    if (flipflopState == 1) {
      currentamberInternalState = 1;
      currentredInternalState = 0;
    } else {
      currentamberInternalState = 0;
      currentredInternalState = 1;
    }
  }

  lastPCOstate = currentPCOstate;

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

  // AND operation
  digitalWrite(amberOutPin, currentamberInternalState & currentAcqLiveState);
  digitalWrite(redOutPin, currentredInternalState & currentAcqLiveState);

  currentamberOutState = digitalRead(amberOutPin);
  currentredOutState = digitalRead(redOutPin);

  if (debugMode == 1) {
    //Serial.print(tmod); Serial.print(" ");
    Serial.print(currentPCOstate); Serial.print(" ");
    Serial.print(currentAcqLiveState); Serial.print(" ");
    Serial.print(currentamberOutState); Serial.print(" ");
    Serial.println(currentredOutState);
  }

  delayMicroseconds(50);
  //}
}
