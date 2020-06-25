#include <SoftwareSerial.h>

#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// define the pins used
#define VS1053_RX  2 // This is the pin that connects to the RX pin on VS1053

#define VS1053_RESET 9 // This is the pin that connects to the RESET pin on VS1053
// If you have the Music Maker shield, you don't need to connect the RESET pin!

// If you're using the VS1053 breakout:
// Don't forget to connect the GPIO #0 to GROUND and GPIO #1 pin to 3.3V
// If you're using the Music Maker shield:
// Don't forget to connect the GPIO #1 pin to 3.3V and the RX pin to digital #2

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 31
#define VS1053_BANK_DEFAULT 0x00
#define VS1053_BANK_DRUMS1 0x78
#define VS1053_BANK_DRUMS2 0x7F
#define VS1053_BANK_MELODY 0x79

// See http://www.vlsi.fi/fileadmin/datasheets/vs1053.pdf Pg 32 for more!
#define VS1053_GM1_OCARINA 80
#define GOBLINS 102

#define MIDI_NOTE_ON  0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_CHAN_MSG 0xB0
#define MIDI_CHAN_BANK 0x00
#define MIDI_CHAN_VOLUME 0x07
#define MIDI_CHAN_PROGRAM 0xC0


SoftwareSerial VS1053_MIDI(0, 2); // TX only, do not use the 'rx' side
// on a Mega/Leonardo you may have to change the pin to one that
// software serial support uses OR use a hardware serial port!
int old_a12, old_a11, old_a10, old_a9, old_a8;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }

  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while (1);
  }
  // power
  Serial.println(F("VL53L0X API Simple Ranging example\n\n"));

  VS1053_MIDI.begin(31250); // MIDI uses a 'strange baud rate'

  pinMode(VS1053_RESET, OUTPUT);
  digitalWrite(VS1053_RESET, LOW);
  delay(10);
  digitalWrite(VS1053_RESET, HIGH);
  delay(10);

  midiSetChannelBank(0, VS1053_BANK_MELODY);
  midiSetInstrument(0, 104);
  midiSetChannelVolume(0, 127);


}

void loop() {
  // put your main code here, to run repeatedly:
  int a_12 = analogRead(A12);
  int a_11 = analogRead(A11);
  int a_10 = analogRead(A10);
  int a_9 = analogRead(A9);
  int a_8 = analogRead(A8);
  //Serial.println(analogRead(A8));
  //  Serial.print(a_12);
  //  Serial.print(' ');
  //  Serial.print(a_11);
  //  Serial.print(' ');
  //  Serial.print(a_10);
  //  Serial.print(' ');
  //  Serial.print(a_9);
  //  Serial.print(' ');
  //  Serial.print(a_8);
  //  Serial.println(' ');

  VL53L0X_RangingMeasurementData_t measure;

  //Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    //Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
  } else {
   // Serial.println(" out of range ");
  }
  
   int KV = map(measure.RangeMilliMeter,0,500,127,0);
   Serial.print("KV: ");
   Serial.println(KV);
  delay(10);




  if (a_12 < 600) {
    midiNoteOff(0, old_a12, KV);
    old_a12 =  'A' + (600 - a_12) / 4;
    midiNoteOn(0, old_a12, KV);
  } else {
    midiNoteOff(0, old_a12, KV);
  }

  if (a_11 < 550) {
    midiNoteOff(0, old_a11, KV);
    old_a11 = 'C' + (550 - a_11) / 4;
    midiNoteOn(0, old_a11, KV);
  } else {
    midiNoteOff(0, old_a11, KV);
  }

  if (a_10 < 550) {
    midiNoteOff(0, old_a10, KV);
    old_a10 = 'E' + (600 - a_10) / 8;
    midiNoteOn(0, old_a10, KV);
  } else {
    midiNoteOff(0, old_a10, KV);
  }

  if (a_9 < 550) {
    midiNoteOn(0, 'H', KV);
  } else {
    midiNoteOff(0, 'H', KV);
  }

  if (a_8 < 550) {
    midiNoteOn(0, 'J', KV);
  } else {
    midiNoteOff(0, 'J', KV);
  }
}
void midiSetInstrument(uint8_t chan, uint8_t inst) {
  if (chan > 15) return;
  inst --; // page 32 has instruments starting with 1 not 0 :(
  if (inst > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_PROGRAM | chan);
  VS1053_MIDI.write(inst);
}


void midiSetChannelVolume(uint8_t chan, uint8_t vol) {
  if (chan > 15) return;
  if (vol > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write(MIDI_CHAN_VOLUME);
  VS1053_MIDI.write(vol);
}

void midiSetChannelBank(uint8_t chan, uint8_t bank) {
  if (chan > 15) return;
  if (bank > 127) return;

  VS1053_MIDI.write(MIDI_CHAN_MSG | chan);
  VS1053_MIDI.write((uint8_t)MIDI_CHAN_BANK);
  VS1053_MIDI.write(bank);
}

void midiNoteOn(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_ON | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}

void midiNoteOff(uint8_t chan, uint8_t n, uint8_t vel) {
  if (chan > 15) return;
  if (n > 127) return;
  if (vel > 127) return;

  VS1053_MIDI.write(MIDI_NOTE_OFF | chan);
  VS1053_MIDI.write(n);
  VS1053_MIDI.write(vel);
}
