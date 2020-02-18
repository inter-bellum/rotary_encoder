#include <MIDI.h>

#define CW 1
#define CCW 2

MIDI_CREATE_DEFAULT_INSTANCE();

static int table[]={0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0};
int* avg;
int index = 0;
int smooth = 50;
byte _val = 0;

byte midiOut = 0, _midiOut = 0;

int count = 0;

//MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
//  Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL_OMNI);
//  MIDI.sendNoteOn(42, 127
  avg = (int *) calloc(smooth, sizeof(int));
}

void loop() {
  int val = (PIND >> 2) & 0x3;
  int valForAvg = 0;
  
  if (val != _val){
    int tableIndex = (_val << 2 | val);

    count += table[tableIndex];
    _val = val;
    valForAvg = 1;
  }
  avg[index] = valForAvg;
  index = (index + 1) % smooth;

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
