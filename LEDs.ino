
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN    10

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT 26

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void initLEDs() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(100); // Set BRIGHTNESS to about 1/5 (max = 255)
  for (int i = 0; i < LED_COUNT; i ++) {
    strip.setPixelColor(i, strip.Color(0,100,255));
    strip.show();
    delay(100);
  }
  delay(100);
  strip.clear();
  strip.show();


}


void setLEDs(int midiOut) {
  float ledRange = int((LED_COUNT * 1) * (float(midiOut) / 127.));
  //  Serial.println("LED range " + String(ledRange));
  for (int i = 0; i < LED_COUNT; i ++) {
    if (i < ledRange) {
      strip.setPixelColor(i, strip.Color(0, 100, 255));
    } else {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }

  }
  strip.show();
}

void blinkLEDS(int vol) {
  for (int i = 0; i < LED_COUNT; i ++) {
    strip.setPixelColor(i, strip.Color(vol * 0.3, vol * 0.5, vol));
  }
  strip.show();
}
