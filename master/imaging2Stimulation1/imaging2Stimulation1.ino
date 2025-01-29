// alternate two imaging light source by each camera frame
// between camera frames, turn on optogen stim and bklight
// 2024-11-17 DS created from LED1LED2AlternateWithFlipperBklightRolling.ino

const bool acqLiveMode = 1; //1 to evoke this code only when acqLive signal is on, 0 to ignore acqLive signal
const int camrate = 2; //frame rate of camera [Hz] (used only in debugMode)
const int duration = 1000 / camrate; //[ms]
int tmod;

// INPUT PINS
const int acqLiveInPin = 13;//5;       // pin for acqLive from Timeline
const int PCObusyPin = 5; //FIXED. if low, cameara is ready for acquiring new trigger. previously known as "FVALPin"

// OUTPUT PINS
const int extraGndPin = 1;       //GND output from arduino
const int LED1OutPin = 8;//9;       // pin for LED1's Gate1
const int LED2OutPin = 9;//4;       // pin for LED2's Gate1
const int poissonPin = 10;          //pin for "flipper"
const int bklightPin = 11; //pin for backlight & laser control 23/4/20
const int laserPin =13; //invert of bklightPin (= PCObusyPin)

const int minPoissonDur = 10; //ms
const int maxPoissonDur = 200; //ms

bool flipflopState = 0;         // current state of the alternation
bool lastAcqLiveState = 0;
bool currentAcqLiveState = 0;
// internal variables
bool currentLED1InternalState = 0;
bool currentLED2InternalState = 0;
// output to LED
bool currentLED1OutState = 0;
bool currentLED2OutState = 0;

unsigned long lastFlipTime = 0;
unsigned long timeNow = 0;
unsigned long poissonStateDur = 0;
int currentPoissonState = 0;
// bool currentTrigInstate = 0;
// bool lastTrigInstate = 0;
bool currentPCObusystate = 0;
bool lastPCObusystate = 0;


void setup() {
  // put your setup code here, to run once:
  pinMode(PCObusyPin, INPUT);
  pinMode(LED1OutPin, OUTPUT);
  pinMode(LED2OutPin, OUTPUT);
  pinMode(acqLiveInPin, INPUT);
  pinMode(bklightPin, OUTPUT);
  digitalWrite(bklightPin, LOW);
  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, HIGH);
  pinMode(extraGndPin, OUTPUT);
  digitalWrite(extraGndPin, LOW);

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
  //      flipflopState = 1; // guarantee that color is LED1 on first frame when acquisition starts.
  //         }

  
  if (lastAcqLiveState == LOW) { //fixed 9/6/20
          flipflopState = 1; // guarantee that color is LED1 on first frame when acquisition starts.
             }
    lastAcqLiveState = currentAcqLiveState;


    //code for LED1/LED2 LEDs
    currentPCObusystate = digitalRead(PCObusyPin);
    if (currentPCObusystate == HIGH & lastPCObusystate == LOW) {
        digitalWrite(bklightPin, LOW);
        digitalWrite(laserPin, HIGH);

      flipflopState = (flipflopState + 1) % 2;

      if (flipflopState == 0) {
        currentLED1InternalState = 1;
        currentLED2InternalState = 0;
      } else {
        currentLED1InternalState = 0;
        currentLED2InternalState = 1;
      }

    } else if (currentPCObusystate == LOW & lastPCObusystate == HIGH) {
    // code for backlight
      digitalWrite(bklightPin, HIGH); //24/4/20
           digitalWrite(laserPin, LOW); //24/4/20
 
    // needs fixing. when camera is not plugged, this condition is not evoked

      // turn off LEDs when screen is ON (not necessary but nicer)
      //currentLED1InternalState = 0;  //16/6/20 commented out
      //currentLED2InternalState = 0; //16/6/20 commented out
    }
    lastPCObusystate = currentPCObusystate;


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
    digitalWrite(LED1OutPin, currentLED1InternalState & currentAcqLiveState);
    digitalWrite(LED2OutPin, currentLED2InternalState & currentAcqLiveState);

    currentLED1OutState = digitalRead(LED1OutPin);
    currentLED2OutState = digitalRead(LED2OutPin);

    delayMicroseconds(50);
  }
