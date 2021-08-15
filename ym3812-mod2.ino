// largely from https://github.com/a1k0n/opl2/blob/master/arduino/ym3812/ym3812.ino

static const byte D2 = 2;
static const byte D3 = 3;
static const byte D4 = 4;
static const byte D5 = 5;
static const byte D6 = 6;
static const byte D7 = 7;
static const byte D8 = 8;
static const byte D9 = 9;
static const byte D10 = 10;
static const byte D11 = 11;
static const byte D12 = 12;

// bus pins for programming YM3812
static const int pin_a0 = D11;
static const int pin_wr = D12;
//static const int pin_cs = D10;

// /IC aka reset pin
static const byte pin_ic = D10; // A0;

uint16_t notetbl[] = {
  0x0157, 0x016b, 0x0181, 0x0198, 0x01b0, 0x01ca, 0x01e5, 0x0202, 0x0220,
  0x0241, 0x0263, 0x0287, 0x0557, 0x056b, 0x0581, 0x0598, 0x05b0, 0x05ca,
  0x05e5, 0x0602, 0x0620, 0x0641, 0x0663, 0x0687, 0x0957, 0x096b, 0x0981,
  0x0998, 0x09b0, 0x09ca, 0x09e5, 0x0a02, 0x0a20, 0x0a41, 0x0a63, 0x0a87,
  0x0d57, 0x0d6b, 0x0d81, 0x0d98, 0x0db0, 0x0dca, 0x0de5, 0x0e02, 0x0e20,
  0x0e41, 0x0e63, 0x0e87, 0x1157, 0x116b, 0x1181, 0x1198, 0x11b0, 0x11ca,
  0x11e5, 0x1202, 0x1220, 0x1241, 0x1263, 0x1287, 0x1557, 0x156b, 0x1581,
  0x1598, 0x15b0, 0x15ca, 0x15e5, 0x1602, 0x1620, 0x1641, 0x1663, 0x1687,
  0x1957, 0x196b, 0x1981, 0x1998, 0x19b0, 0x19ca, 0x19e5, 0x1a02, 0x1a20,
  0x1a41, 0x1a63, 0x1a87, 0x1d57, 0x1d6b, 0x1d81, 0x1d98, 0x1db0, 0x1dca,
  0x1de5, 0x1e02, 0x1e20, 0x1e41, 0x1e63, 0x1e87};

uint8_t notenum = 40;

void ym3812_write1(uint8_t addr, uint8_t data) {
  // write reg
  digitalWrite(pin_a0, addr);  // set a0
  //digitalWrite(pin_cs, 0);  // assert /CS

  // our 8 bit bus is spread out across some random arduino DIO pins
  digitalWrite(D2, (data >> 7 ) & 0x01);
  digitalWrite(D3, (data >> 6 ) & 0x01);
  digitalWrite(D4, (data >> 5 ) & 0x01);
  digitalWrite(D5, (data >> 4 ) & 0x01);
  digitalWrite(D6, (data >> 3 ) & 0x01);
  digitalWrite(D7, (data >> 2 ) & 0x01);
  digitalWrite(D8, (data >> 1 ) & 0x01);
  digitalWrite(D9, (data >> 0 ) & 0x01);
  
  digitalWrite(pin_wr, 0);  // assert /WR
  // 100ns
  delayMicroseconds(0);  // FIXME
  digitalWrite(pin_wr, 1);  // unassert /WR
  //digitalWrite(pin_cs, 1);  // unassert /CS
}

void ym3812_write(uint8_t reg, uint8_t val) {
  ym3812_write1(0, reg);
  delayMicroseconds(5);  // orig 4
  ym3812_write1(1, val);
  delayMicroseconds(24); // orig 23
}

// this page http://www.shipbrook.net/jeff/sb.html seems to indicate
// that the 'op1' and 'op2' are switched
const uint8_t op1 = 0;
const uint8_t op2 = 3;
const uint8_t chan = 0;

void setup() {
  static const uint8_t output_pins[] = {
    D2, D3, D4, D5, D6, D7, D8, D9,   // D0..D7
    pin_a0, pin_wr, /*pin_cs,*/ pin_ic
  };
  for (uint8_t i = 0; i < sizeof(output_pins); i++) {
    pinMode(output_pins[i], OUTPUT);
  }

  // reset the YM3812
  digitalWrite(pin_ic, LOW);
  delay(10);  // TODO: look up how long you need to hold reset
  digitalWrite(pin_ic, HIGH);
  
  digitalWrite(pin_wr, 1);  // de-assert /WR

  delay(100);

  ym3812_write(0x60 + op1, 0xf0);  // ad. decay must be 'long' (0x0) for vco mode
  ym3812_write(0x80 + op1, 0xFF);  // sr
  ym3812_write(0x40 + op1, 0x10);  // ksl / output level
  ym3812_write(0x20 + op1, 0x01);  // multiplier, vibrato, sustain 0x20
  ym3812_write(0xe0 + op1, 0x02);  // waveform (half sine)

  ym3812_write(0x60 + op2, 0xf0);  // ad. decay must be 'long' (0x0) for vco mode
  ym3812_write(0x80 + op2, 0xFF);  // sr
  ym3812_write(0x40 + op2, 0x00);  // ksl / output level  
  ym3812_write(0x20 + op2, 0x01);  // multiplier + vibrato etc
  ym3812_write(0xe0 + op2, 0x00);  // waveform (sine)
  
  ym3812_write(0xc0 + chan, 0x00);  // synthtype + feedback

}

byte note = 20;
byte modulation = 1;
byte feedback = 1;
byte mult = 1;
int knob0, knob1, knob2;
byte mult2 = 1;

void loop() {
  //demo_loop();
  vco_loop();
}

void vco_loop() {
  knob0 = analogRead(A0);
  knob1 = analogRead(A1);
  knob2 = analogRead(A2);

  note = map(knob0, 0, 1023, 0, notenum);
  ym3812_write(0xa0, notetbl[note] & 0x00ff);  // least significant byte of f-num
  ym3812_write(0xb0, 0x20 | ((notetbl[note] >> 8) & 0x00FF) ) ;  // f-num, octave, key on
  
  mult= map(knob1, 0, 1023, 0, 0x0F);
  ym3812_write(0x20 + op1, 0x20 | mult);

  //modulation = map(knob2, 0, 1023, 0x1F, 0x0);
  modulation = 0x1D;
  ym3812_write(0x40 + op1, modulation);

  //feedback = map(knob2, 0, 1023, 1, 7);
  feedback = 0;
  ym3812_write(0xc0 + op1, (feedback << 1) );
  
  mult2 = map(knob2, 0, 1023, 0, 0x0F);
  ym3812_write(0x20 + op2, 0x20 | mult2);
  
  //digitalWrite(13, true);
  //delay(50);
  //digitalWrite(13, false);
}

void demo_loop() {
  // from http://www.shipbrook.net/jeff/sb.html
                            // Reg Val
  ym3812_write(0x20, 0x01); // 20  01  Set the modulator's multiple to 1
  ym3812_write(0x40, 0x10); // 40  10  Set the modulator's level to about 40 dB
  ym3812_write(0x60, 0xF0); // 60  F0  Modulator attack: quick; decay: long
  ym3812_write(0x80, 0x77); // 80  77  Modulator sustain: medium; release: medium
  ym3812_write(0xa0, 0x98); // A0  98  Set voice frequency's LSB (it'll be a D#)
  ym3812_write(0x23, 0x01); // 23  01  Set the carrier's multiple to 1
  ym3812_write(0x43, 0x00); // 43  00  Set the carrier to maximum volume (about 47 dB)
  ym3812_write(0x63, 0xF0); // 63  F0  Carrier attack: quick; decay: long
  ym3812_write(0x83, 0x77); // 83  77  Carrier sustain: medium; release: medium
  ym3812_write(0xb0, 0x31); // B0  31  Turn the voice on; set the octave and freq MSB
  digitalWrite(13, true);
  delay(200);
  
  ym3812_write(0xb0, 0x11);  // To turn the voice off, set register B0h to 11h
  digitalWrite(13, false);
  delay(200);
}
