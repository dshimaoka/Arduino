const bool debugMode = 0;  //1 to use simulated camera exposure and show outputs
const bool acqLiveMode = 1; //1 to evoke this code only when acqLive signal is on, 0 to ignore acqLive signal
const int camrate = 2; //frame rate of camera [Hz] (used only in debugMode)
const int duration = 1000 / camrate; //[ms]
int tmod;

const int PCOinPin = 13;//2;    // pin for "all lines exposing" 
const int copyPCOinPin = 12;//2;    // copy of PCOinPin
const int blueOutPin = 6;//9;       // pin for blue's Gate1 
const int purpleOutPin = 9;//4;       // pin for purple's Gate1 
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
bool currentBlueInternalState = 0;
bool currentPurpleInternalState = 0;
// output to LED
bool currentBlueOutState = 0;
bool currentPurpleOutState = 0;

unsigned long lastFlipTime = 0;
unsigned long timeNow = 0;
unsigned long poissonStateDur = 0;
int currentPoissonState = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(PCOinPin, INPUT);
  pinMode(copyPCOinPin, OUTPUT);
  pinMode(blueOutPin, OUTPUT);
  pinMode(purpleOutPin, OUTPUT);
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
    flipflopState = 0; // guarantee that color is blue on first frame when acquisition starts.
    currentBlueInternalState = 1;
    currentPurpleInternalState = 0;
    }

  lastAcqLiveState = currentAcqLiveState;

  currentPCOstate = digitalRead(PCOinPin);
  digitalWrite(copyPCOinPin, currentPCOstate);

  if (currentPCOstate == LOW & lastPCOstate == HIGH) {
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
  digitalWrite(blueOutPin, currentBlueInternalState & currentPCOstate & currentAcqLiveState);
  digitalWrite(purpleOutPin, currentPurpleInternalState & currentPCOstate & currentAcqLiveState);

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
