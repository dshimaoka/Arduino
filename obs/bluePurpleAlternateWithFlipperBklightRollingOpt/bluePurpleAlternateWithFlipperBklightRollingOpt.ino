const bool acqLiveMode = 1; //1 to evoke this code only when acqLive signal is on, 0 to ignore acqLive signal
const int camrate = 2; //frame rate of camera [Hz] (used only in debugMode)
const int duration = 1000 / camrate; //[ms]
int tmod;

const int FVALPin = 5; //FIXED "Frame Valid" in TSI-IOBOB2
const int TrigInPin = 3; //FIXED "Trig In" in TSI-IOBOB2 or "tlExposeClock" from NIDAQ
const int acqLiveInPin = 13;//5;       // pin for acqLive from Timeline
//const int extraGndPin = 1;  
const int vsInPin = 1; //1/10/20
const int camExposureInPin = 4; //1/10/20 Fixed "Strobe" in TSI-IOBOB2

const int blueOutPin = 8;//9;       // pin for blue's Gate1
const int purpleOutPin = 9;//4;       // pin for purple's Gate1
const int poissonPin = 10;          //pin for "flipper"
const int bklightPin = 11; //pin for backlight control 23/4/20
const int vsOutPin = 12;    // pin for flipped blue + purple 1/10/20

const int minPoissonDur = 10; //ms
const int maxPoissonDur = 200; //ms

bool flipflopState = 0;         // current state of the alternation
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
bool currentVsInState = 0; //1/10/20
bool camExposureState = 0; //1/10/20

void setup() {
  // put your setup code here, to run once:
  pinMode(FVALPin, INPUT);
  pinMode(TrigInPin, INPUT); //NOT OUTPUT
  //pinMode(PCOinPin, INPUT);
  pinMode(blueOutPin, OUTPUT);
  pinMode(purpleOutPin, OUTPUT);
  pinMode(vsOutPin, OUTPUT); //1/10/20
  pinMode(acqLiveInPin, INPUT);
  pinMode(bklightPin, OUTPUT);
  digitalWrite(bklightPin, LOW);
  //pinMode(extraGndPin, OUTPUT);
  //digitalWrite(extraGndPin, LOW);
  pinMode(vsInPin, INPUT); //1/10/20

  pinMode(poissonPin, OUTPUT);
  digitalWrite(poissonPin, LOW);
  currentPoissonState = LOW;
  poissonStateDur = random(minPoissonDur, maxPoissonDur);
  lastFlipTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:

  currentAcqLiveState = digitalRead(acqLiveInPin);
  timeNow = millis();

  //    if (currentAcqLiveState == HIGH & lastAcqLiveState == LOW) {
  //      flipflopState = 1; // guarantee that color is blue on first frame when acquisition starts.
  //         }

  
  if (lastAcqLiveState == LOW) { //fixed 9/6/20
          flipflopState = 1; // guarantee that color is blue on first frame when acquisition starts.
             }
    lastAcqLiveState = currentAcqLiveState;


    //code for blue/purple LEDs
    currentTrigInstate = digitalRead(TrigInPin);
    if (currentTrigInstate == HIGH & lastTrigInstate == LOW) {
      digitalWrite(bklightPin, LOW);
      flipflopState = (flipflopState + 1) % 2;

      if (flipflopState == 0) {
        currentBlueInternalState = 1;
        currentPurpleInternalState = 0;
      } else {
        currentBlueInternalState = 0;
        currentPurpleInternalState = 1;
      }
    }
    lastTrigInstate = currentTrigInstate;

  currentVsInState = digitalRead(vsInPin); //1/10/20
  camExposureState = digitalRead(camExposureInPin); //1/10/20

    // code for backlight
    currentFVALstate = digitalRead(FVALPin);
    if (currentFVALstate == LOW & lastFVALstate == HIGH) {
      digitalWrite(bklightPin, HIGH); //24/4/20
    // needs fixing. when camera is not plugged, this condition is not evoked

      // turn off LEDs when screen is ON (not necessary but nicer)
      //currentBlueInternalState = 0;  //16/6/20 commented out
      //currentPurpleInternalState = 0; //16/6/20 commented out
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


    // turn LED only when liveAcq is ON
    digitalWrite(blueOutPin, currentBlueInternalState & currentAcqLiveState);
    digitalWrite(purpleOutPin, currentPurpleInternalState & currentAcqLiveState);
    //digitalWrite(vsOutPin, 1); //ok
    digitalWrite(vsOutPin, (1-camExposureState) * currentVsInState); //1/10/20
    //digitalWrite(vsOutPin, (1-camExposureState) & currentAcqLiveState & currentVsInState); //1/10/20

    currentBlueOutState = digitalRead(blueOutPin);
    currentPurpleOutState = digitalRead(purpleOutPin);

    delayMicroseconds(50);
  }
