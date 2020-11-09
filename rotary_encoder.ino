#include <MIDI.h>

#define CW 1
#define CCW 2

MIDI_CREATE_DEFAULT_INSTANCE();

static int table[]={0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
int* avg;
//int index = 0;
int smooth = 50;
byte _val = 0;

byte midiOut = 0, _midiOut = 0;

int count = 0;

void setup() {
  pinMode(13,OUTPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  digitalWrite(13,HIGH);
  delay(1000);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  avg = (int *) calloc(smooth, sizeof(int));
  Serial.begin(9600);
}

void loop() {
  //read values from pin 2 & 3 (PIND = pins 0-7)
  //shift pin state right: 10101100 -> 101011
  //mask state of pins with 3 (11 in binary): 101011 -> 000011
  int val = (PIND >> 2) & 0x3;
  int valForAvg = 0;

  if (digitalRead(2) || digitalRead(3)) { 
    digitalWrite(13,HIGH);
  } else {
    digitalWrite(13,LOW);
  }
  
  if (val != _val){
    int tableIndex = (_val << 2 | val);
    Serial.println("Change! val:" + String(val));
    count += table[tableIndex];
    _val = val;
    valForAvg = 1;
  }
  
//  avg[index] = valForAvg;
//  index = (index + 1) % smooth;

  float sum = 0;
  for (int i = 0; i < smooth; i++){
    sum += avg[i];
  }

  midiOut = floor((sum / smooth) * 127);

  if (midiOut != _midiOut){
    MIDI.sendControlChange(80, midiOut, 1);
    _midiOut = midiOut;
  }

  
}
