
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    10

elapsedMillis sinceIdleLEDUpdate;

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 26

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool isIdle = false; //are we idle?
bool _isIdle = false; //were we idle?
const int timeOutThreshold = 5000; //this is about 5 minutes in ms

int brights[LED_COUNT];//keep track of the brights of every idle LED

int hues[LED_COUNT]; //keep track of all hues

void initLEDs() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
  for (int i = 0; i < LED_COUNT; i ++) {
    strip.setPixelColor(i, strip.Color(0, 100, 255));
    strip.show();
    delay(50);
  }
  delay(100);
  strip.clear();
  strip.show();
}


void setLEDs(int midiOut) {
  float ledRange = int((LED_COUNT * 1) * (float(midiOut) / 128.));
  int colorIntensity = midiOut * 2; //map the midi to 0 - 255 to make it work with the LEDs
  for (int i = 0; i < LED_COUNT; i ++) {
    if (i < ledRange) {
      int r = constrain((colorIntensity - 220) * 7, 0, 255);
      Serial.println("r = " + String(r));
      int g = 100;
      int b = constrain(colorIntensity, 0, 100);
      if (colorIntensity > 150) {
        b = 10;
      }
      strip.setPixelColor(i, r, b , g );
    } else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }

  }
  strip.show();
}

void blinkLEDS(int vol) {
  //blink the LEDs at the level of the received vol
  if (LED_COUNT > 0) {
    for (int i = 0; i < LED_COUNT; i ++) {
      strip.setPixelColor(i, strip.Color(vol * 0.3, vol * 0.5, vol));
    }
    strip.show();
  }
}



void idleLEDs() {
  //this function animates some idle sparkly stuff when the wheel is not touched for a while
  for (int i = 0; i < LED_COUNT; i++ ) {
    //increment all brightnesses a little bit with a preference to going up
    brights[i] += int(random(-1.99, 2));
    //make sure it never overflows
    brights[i] = constrain(brights[i], 0, 255);

    //adjust the hue a little bit every time
    hues[i] += int(random(-100, 100));

    //make sure it stays within a blue to red range (max is  65535)
    if (hues[i] < 32768) {
      hues[i] = 65535;
    } else if (hues[i] > 65535) {
      hues[i] = 32768;
    }
    strip.setPixelColor(i, strip.ColorHSV(hues[i], 255, int(scalingTable[int(brights[i] / 2.)] * 255.)));
  }

  strip.show();

  //reset the idle update timer
  sinceIdleLEDUpdate = 0;
}

bool checkIdle() {
  //checks if the wheel has been touched in a while

  //  Serial.println("Checking if wheel is idle");
  bool isIdle = false;
  //Did we not touch it for a while?
  if (sinceTouched > timeOutThreshold) {
    //    Serial.println("havent been touched in a while");
    isIdle = true; //then we are idle

    //only update about every 25 millis
    if (sinceIdleLEDUpdate > 25) {
      //animate the LEDs when idle
      idleLEDs();
    }

    if (isIdle != _isIdle) { //when it changed from not idle to idle
      //then reset all hues and brightnesses
      restartIdle();
    }
  }

  if (isIdle != _isIdle ) {
    if (isIdle) {
      //If we changed from idle to being active again
      strip.setBrightness(10);
    } else {
      strip.setBrightness(50);
    }
  }

  //keep track of older idle bool
  _isIdle = isIdle;
  return isIdle;
}

void restartIdle() {
  Serial.println("Restarting idle ");
  for (int i = 0; i < LED_COUNT; i++) {
    brights[i] = 0; //set all brightnesses to zero
    hues[i] = int(random(32768, 65535)); //fill everything with random hues between blue and red
    Serial.println(String(hues[i]) + ", " + String(brights[i]) + " - ");
  }
  Serial.println("");
}
