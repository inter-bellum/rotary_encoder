#include <avr/io.h>
#include <avr/interrupt.h>

#define CW 1
#define CCW 2

//#define DEBUG

#define BUILT_IN_LED_ON()    (digitalWrite(13, HIGH))
#define BUILT_IN_LED_OFF()   (digitalWrite(13, LOW))



long cycleCount = 0;

elapsedMillis sinceTouched; //keep track of when the wheel was last touched

//forward declare functions
//encoder interrupt function
void encoderIRQ(void);

//variables that hold the encoder pin values
volatile uint8_t encoderPinValues = 0, _encoderPinValues = 0;

//flag for detecting a new value
volatile bool newVal = false;

//encoder direction table. translates the value of the two encoder pins into a single directional value
static int table[] = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};


uint8_t scalingTableLength = 128;
//table for scaling LED brightness values parabolically, giving the lower range smaller steps
const float scalingTable[] = {
  0.,       0.000689, 0.002756, 0.0062,   0.011022, 0.017222, 0.0248,   0.033756, 0.044089, 0.0558,   0.068889, 0.083356, 0.0992,   0.102362, 0.110236, 0.11811,
  0.125984, 0.133858, 0.141732, 0.149606, 0.15748,  0.165354, 0.173228, 0.181102, 0.188976, 0.19685,  0.204724, 0.212598, 0.220472, 0.228346, 0.23622,  0.244094,
  0.251969, 0.259843, 0.267717, 0.275591, 0.283465, 0.291339, 0.299213, 0.307087, 0.314961, 0.322835, 0.330709, 0.338583, 0.346457, 0.354331, 0.362205, 0.370079,
  0.377953, 0.385827, 0.393701, 0.401575, 0.409449, 0.417323, 0.425197, 0.433071, 0.440945, 0.448819, 0.456693, 0.464567, 0.472441, 0.480315, 0.488189, 0.496063,
  0.503937, 0.511811, 0.519685, 0.527559, 0.535433, 0.543307, 0.551181, 0.559055, 0.566929, 0.574803, 0.582677, 0.590551, 0.598425, 0.606299, 0.614173, 0.622047,
  0.629921, 0.637795, 0.645669, 0.653543, 0.661417, 0.669291, 0.677165, 0.685039, 0.692913, 0.700787, 0.708661, 0.716535, 0.724409, 0.732283, 0.740157, 0.748031,
  0.755906, 0.76378,  0.771654, 0.779528, 0.787402, 0.795276, 0.80315,  0.811024, 0.818898, 0.826772, 0.834646, 0.84252,  0.850394, 0.858268, 0.866142, 0.874016,
  0.88189,  0.889764, 0.897638, 0.905512, 0.913386, 0.92126,  0.929134, 0.937008, 0.944882, 0.952756, 0.96063,  0.968504, 0.976378, 0.984252, 0.992126, 1.
};

//running index for the smoothing table
uint8_t smoothingIndex = 0;
const uint8_t smoothingSampleCount = 70;
uint16_t history[smoothingSampleCount];


const uint8_t midiSmoothCount = 15;
uint8_t midiSmooth[midiSmoothCount];
uint8_t midiSmoothIndex = 0;
uint8_t midiOut = 0, _midiOut = 0;

volatile int count = 0;

const byte inputPin1 = 2;
const byte inputPin2 = 3;

void setup() {
  NVIC_ENABLE_IRQ(IRQ_PORTD);
  pinMode(inputPin1, INPUT);
  attachInterrupt(digitalPinToInterrupt(inputPin1), encoderIRQ, CHANGE);
  pinMode(inputPin2, INPUT);
  attachInterrupt(digitalPinToInterrupt(inputPin2), encoderIRQ, CHANGE);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  delay(250);
  initLEDs();

#ifdef DEBUG
  Serial.begin(115200);
#endif
}

void loop() {
  uint16_t valForAvg = 0;

  //if the encoder rotated
  if (encoderPinValues != _encoderPinValues) {
    //add 1 to the running average, regardless of direction
    valForAvg = 1;
    //save the new value
    _encoderPinValues = encoderPinValues;
    BUILT_IN_LED_ON();
  } else {
    BUILT_IN_LED_OFF();
  }


#ifdef DEBUG
    Serial.print(0);
    Serial.print(",");
    Serial.print(127);
    Serial.print(",");
    Serial.println(encoderPinValues);
#endif

  //add the value to the array of values for averaging
  history[smoothingIndex] = valForAvg;
  //increment index into the array of values for averaging
  smoothingIndex = (smoothingIndex + 1) % smoothingSampleCount;

  //sum the values
  float sum = 0;
  for (int i = 0; i < smoothingSampleCount; i++) {
    sum += history[i];
  }

  //calculate index in the scalingTable by calculating the average between 0 and 1, then multiplying by the size of the array - 1
  uint8_t indexIntoScalingTable = (uint8_t) (floor(sum / smoothingSampleCount * (scalingTableLength - 1)));
  
  //the scaling table again returns a number between 0 and 1, so multiply by 127 to get the max midi value
  midiOut = floor(scalingTable[indexIntoScalingTable] * 127);
  
  //add the value to the midi smoothing array
  midiSmooth[midiSmoothIndex] = midiOut;

  //increment midi smooth index
  midiSmoothIndex = (midiSmoothIndex + 1) % midiSmoothCount;

  float midiSum = 0;

  //calculate the sum of the midi values
  for (int i = 0; i < midiSmoothCount; i++) {
    midiSum += midiSmooth[i];
  }

  //average te value
  uint8_t midiSmoothedOut = floor(midiSum / midiSmoothCount);

#ifdef DEBUG
    Serial.println(midiSmoothedOut);
#endif

  //if it's different
  if (midiOut != _midiOut) {
    sinceTouched = 0;
    //send the value
    usbMIDI.sendControlChange(80, midiOut, 1);
    //save the new value
    _midiOut = midiOut;
  }

  //do some LED magic
  setLEDs(midiOut);

  //process midi if new data is available
  if (usbMIDI.read()) {
    processMIDI();
  }

  //why?
  delayMicroseconds(700);

  //check for idle
  checkIdle();
}

void encoderIRQ(void) {
  //get the encoder pin values by shifting the (P)in (IN)put register for pin port (D) (digital pins 0-7) to the right by the first pin number (we're reading pin 2 and 3)
  //then mask the result with 0b11 to get rid of noise/values on the other pins
  encoderPinValues = (PIND >> 2) & 0x3;

  //calculate the table index by some magic where the difference between the previous and current reading is used as an array index
  int tableIndex = (_encoderPinValues << 2 | encoderPinValues);
  
  //add the (add this point directional) value to the step count
  count += table[tableIndex];
}
