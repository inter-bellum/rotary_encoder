
void processMIDI(void) {
  byte type, channel, data1, data2, cable;

  // fetch the MIDI message, defined by these 5 numbers (except SysEX)
  //
  type = usbMIDI.getType();       // which MIDI message, 128-255
  data2 = usbMIDI.getData2();     // second data byte of message, 0-127  
#ifdef DEBUG
  channel = usbMIDI.getChannel(); // which MIDI channel, 1-16
  data1 = usbMIDI.getData1();     // first data byte of message, 0-127
  cable = usbMIDI.getCable();     // which virtual cable with MIDIx8, 0-7
#endif

  // print info about the message
  //
  switch (type) {

    case usbMIDI.ControlChange: // 0xB0
#ifdef DEBUG
      Serial.print("Control Change, ch=");
      Serial.print(channel, DEC);
      Serial.print(", control=");
      Serial.print(data1, DEC);
      Serial.print(", value=");
      Serial.println(data2, DEC);
#endif
      blinkLEDS(data2 * 3);
      break;
  }

}
