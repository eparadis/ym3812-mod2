// largely from https://github.com/a1k0n/opl2/blob/master/arduino/ym3812/ym3812.ino

// see https://github.com/bitbank2/ss_oled/blob/master/src/ss_oled.h
#include <ss_oled.h>

SSOLED ssoled;
//static uint8_t ucBackBuffer[1024]; // not enough RAM as of yet

void setupOLED() {
  oledInit(&ssoled, OLED_128x32, -1, 1, 0, 1, -1, -1, -1, 400000L);
//oledSetBackBuffer(&ssoled, ucBackBuffer);

  oledFill(&ssoled, 0x0, 1);
  oledWriteString(&ssoled,0, 0,0, (char *)"BeepBoop", FONT_STRETCHED, 0, 1);
  oledWriteString(&ssoled,0, 0,2, (char *)"have fun", FONT_STRETCHED, 0, 1);
  delay(400);
  oledFill(&ssoled, 0x0, 1);

  // see https://github.com/bitbank2/ss_oled/blob/01fb9a53388002bbb653c7c05d8e80ca413aa306/src/ss_oled.h#L137 for write string usage
}

byte modal_type = 0;
byte feedback = 1;
byte wave1 = 0, wave2 = 0;
char waveforms[][9] = {"sine    ", "halfsine", "abs sine", "clipsine"};
char fb_label[]   = "val: 0  ";
void cycle_mode() {
  modal_type = ( modal_type + 1) % 3;
  switch( modal_type) {
    case 0:
      oledWriteString(&ssoled,0, 0,0, (char *)"FEEDBACK", FONT_STRETCHED, 0, 1);
      oledWriteString(&ssoled,0, 0,2, fb_label, FONT_STRETCHED, 0, 1);
      break;
    case 1:
      oledWriteString(&ssoled,0, 0,0, (char *)"OP1 WAVE", FONT_STRETCHED, 0, 1);
      oledWriteString(&ssoled,0, 0,2, waveforms[wave1], FONT_STRETCHED, 0, 1);
      break;
    case 2:
      oledWriteString(&ssoled,0, 0,0, (char *)"OP2 WAVE", FONT_STRETCHED, 0, 1);
      oledWriteString(&ssoled,0, 0,2, waveforms[wave2], FONT_STRETCHED, 0, 1);
      break;
  }
}

// bus pins for programming YM3812
#define pin_D0 9 // PB1
#define pin_D1 8 // PB0
#define pin_D2 7 // PD7
#define pin_D3 6 // PD6
#define pin_D4 5 // PD5
#define pin_D5 4 // PD4
#define pin_D6 3 // PD3
#define pin_D7 2 // PD2
#define pin_A0 11 // PB3
#define pin_WR 12 // PB4
#define pin_IC 10 // PB2  ~IC aka reset pin

#define pin_BUTTON A0 // PC0 will be used as a digital input

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

void assertWrite() {
  //digitalWrite(pin_WR, 0);  // PB4
  bitClear(PORTB, PB4);
}

void unassertWrite() {
  //digitalWrite(pin_WR, 1);  // PB4
  bitSet(PORTB, PB4);
}

void assertReset() {
  //digitalWrite(pin_IC, LOW); // PB2
  bitClear(PORTB, PB2);
}

void unassertReset() {
  //digitalWrite(pin_IC, HIGH); // PB2
  bitSet(PORTB, PB2);
}

void ym3812_write1(uint8_t addr, uint8_t data) {
  // write reg
  //digitalWrite(pin_A0, addr);  // PB3
  bitWrite(PORTB, PB3, addr);
  
  // our 8 bit bus is spread out across some random arduino DIO pins
//  digitalWrite(pin_D7, data & 0x80); // PD2
  bitWrite(PORTD, PD2, data & 0x80);

//  digitalWrite(pin_D6, data & 0x40); // PD3
  bitWrite(PORTD, PD3, data & 0x40);

//  digitalWrite(pin_D5, data & 0x20); // PD4
  bitWrite(PORTD, PD4, data & 0x20);

//  digitalWrite(pin_D4, data & 0x10); // PD5
  bitWrite(PORTD, PD5, data & 0x10);

//  digitalWrite(pin_D3, data & 0x08); // PD6
  bitWrite(PORTD, PD6, data & 0x08);

//  digitalWrite(pin_D2, data & 0x04); // PD7
  bitWrite(PORTD, PD7, data & 0x04);

//  digitalWrite(pin_D1, data & 0x02); // PB0
  bitWrite(PORTB, PB0, data & 0x02);

//  digitalWrite(pin_D0, data & 0x01); // PB1
  bitWrite(PORTB, PB1, data & 0x01);

  assertWrite();
  // 100ns
  delayMicroseconds(1);  // FIXME
  unassertWrite();
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
  // button
  pinMode(pin_BUTTON, INPUT_PULLUP); // an analog line is used as a digital input

  const uint8_t output_pins[] = {
    pin_D0, pin_D1, pin_D2, pin_D3, pin_D4, pin_D5, pin_D6, pin_D7,
    pin_A0, pin_WR, pin_IC
  };
  for (uint8_t i = 0; i < sizeof(output_pins); i++) {
    pinMode(output_pins[i], OUTPUT);
  }

  // reset the YM3812
  assertReset();
  delay(10);  // TODO: look up how long you need to hold reset
  unassertReset();

  unassertWrite();

  delay(100);

  ym3812_write(0x01, 0x20);  // test off, wave select enable(!)

  ym3812_write(0x60 + op1, 0xf0);  // ad. decay must be 'long' (0x0) for vco mode
  ym3812_write(0x80 + op1, 0xFF);  // sr
  ym3812_write(0x40 + op1, 0x10);  // ksl / output level
  ym3812_write(0x20 + op1, 0x01);  // multiplier, vibrato, sustain 0x20
  ym3812_write(0xe0 + op1, 0x00);  // waveform (sine)

  ym3812_write(0x60 + op2, 0xf0);  // ad. decay must be 'long' (0x0) for vco mode
  ym3812_write(0x80 + op2, 0xFF);  // sr
  ym3812_write(0x40 + op2, 0x00);  // ksl / output level  
  ym3812_write(0x20 + op2, 0x01);  // multiplier + vibrato etc
  ym3812_write(0xe0 + op2, 0x00);  // waveform (sine)

  ym3812_write(0xc0 + chan, 0x00);  // synthtype + feedback

  create_ratio_table();

  setupOLED();
  cycle_mode();

  // setup ADC
  ADCSRA = bit(ADEN) // Turn ADC on
           | bit(ADPS0) | bit(ADPS1) | bit(ADPS2); // Prescaler of 128
  ADMUX  = bit(REFS0) // AVCC
           | ((A7 - 14) & 0x07); // Arduino Uno to ADC pin
}

struct mult_ratio {
  float ratio;
  byte op1;
  byte op2;
};

mult_ratio ratios[87];
bool not_contained(float rat) {
  bool found = false;
  byte i = 0;
  while( i<87 && !found) {
    if( abs(ratios[i].ratio - rat) < 0.00001 ) {
      // equal
      found = true;
    }
    i += 1;
  }
  return !found;
}

byte last_added = 0;
void add_ratio(byte op1, byte op2, float rat) {
  ratios[last_added].ratio = rat;
  ratios[last_added].op1 = op1;
  ratios[last_added].op2 = op2;
  last_added += 1;
}

// https://rosettacode.org/wiki/Sorting_algorithms/Bubble_sort#C
void bubble_sort (struct mult_ratio *a, int n) {
    byte i, j = n, s = 1;
    float t;
    while (s) {
        s = 0;
        for (i = 1; i < j; i++) {
            if (a[i].ratio < a[i - 1].ratio) {
                t = a[i].ratio;
                a[i].ratio = a[i - 1].ratio;
                a[i - 1].ratio = t;
                s = 1;
            }
        }
        j--;
    }
}

void sort_ratios() {
  bubble_sort(&ratios[0], 87);
}

void create_ratio_table() {
  float r[] = { 0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15 };
  float rat;

  // fill list of ratios with place holders
  for(byte i=0; i<87; i+=1) {
    ratios[i].ratio = 1.0;
    ratios[i].op1 = 1;
    ratios[i].op1 = 1;
  }

  // add every unique ratio
  for( byte op1 = 0; op1 < 13; op1 += 1) {
    for( byte op2 = 0; op2 < 13; op2 += 1) {
      rat = r[op1]/r[op2];
      if( not_contained(rat)) {
        add_ratio(op1, op2, rat);
      }
    }
  }

  sort_ratios();
}

byte isButtonPushed() {
  return !digitalRead(pin_BUTTON); // PC0
//  return bit_is_clear(PINC, PC0);
//  return !(PINC & (1<<PC0));

}

int old_raw_val = 0;
void modal_knob( int raw_val) {
  // detect if the value has actually changed since we switched modes so that cycling the mode doesn't change things
  if(abs(old_raw_val - raw_val) < 8)
    return;
  old_raw_val = raw_val;

  int dummy; // used to make sure we don't update things when we don't need to

  switch(modal_type) {
    case 0:
      dummy = map(raw_val, 0, 1023, 0, 8);
      if( dummy != feedback) {
        feedback = dummy;
        ym3812_write(0xc0 + op1, (feedback << 1) );
        fb_label[5] = '0' + feedback;
        oledWriteString(&ssoled,0, 0,2, fb_label, FONT_STRETCHED, 0, 1);
      }
      break;
    case 1: // waveform for op1
      // 0 = sine, 1 = halfsine, 2 = rectified sine, 3 = half cycle rectified sine
      dummy = map(raw_val, 0, 1023, 0, 4) & 0b00000011;
      if( dummy != wave1) {
        wave1 = dummy;
        ym3812_write(0xe0 + op1, wave1);
        oledWriteString(&ssoled,0, 0,2, waveforms[wave1], FONT_STRETCHED, 0, 1);
      }
      break;
    case 2: // waveform for op2
      // 0 = sine, 1 = halfsine, 2 = rectified sine, 3 = half cycle rectified sine
      dummy = map(raw_val, 0, 1023, 0, 4) & 0b00000011;
      if( dummy != wave2) {
        wave2 = dummy;
        ym3812_write(0xe0 + op2, wave2);
        oledWriteString(&ssoled,0, 0,2, waveforms[wave2], FONT_STRETCHED, 0, 1);
      }
      break;
  }
}

bool adc_conversion_working = false;
byte adcChannel = A7;
void setADCChannel(byte pin) {
  adcChannel = pin;
  ADMUX  = bit(REFS0) // AVCC
    | ((pin - 14) & 0x07); // Arduino Uno to ADC pin
}

void startADC() {
  bitSet(ADCSRA, ADSC);  // Start a conversion
  adc_conversion_working = true;
}

byte adcReady() {
  return bit_is_clear(ADCSRA, ADSC);
}

void setNextADCChannel() {
  if(adcChannel == A7)
    adcChannel = A1;
  else if(adcChannel == A1)
    adcChannel = A2;
  else if(adcChannel == A2)
    adcChannel = A3;
  else
    adcChannel = A7;
  setADCChannel(adcChannel);
}

byte note = 20;
byte modulation = 1;
int ratio;
bool waiting_for_button_release = false;
int dummy;

void loop() {
  if( !adc_conversion_working)
    startADC();

  if( adcReady()) {
    adc_conversion_working = false;
    switch (adcChannel) {
      case A7:
        dummy = map(ADC, 0, 1023, 0, notenum);
        if( dummy != note) {
          note = dummy;
          ym3812_write(0xa0, notetbl[note] & 0x00ff);  // least significant byte of f-num
          ym3812_write(0xb0, 0x20 | ((notetbl[note] >> 8) & 0x00FF) ) ;  // f-num, octave, key on
        }
        break;
      case A1:
        dummy = map(ADC, 0, 1023, 0x1F, 0x0);
        if( dummy != modulation) {
          modulation = dummy;
          ym3812_write(0x40 + op1, modulation);
        }
        break;
      case A2:
          dummy = map(ADC, 0, 1023, 0, 87);
          if( dummy > 86) dummy = 86;
          if( dummy != ratio) {
            ratio = dummy;
            ym3812_write(0x20 + op1, 0x20 | ratios[ratio].op1);
            ym3812_write(0x20 + op2, 0x20 | ratios[ratio].op2);
          }
        break;
      case A3:
        modal_knob(ADC);
        break;
    }
    setNextADCChannel();
  }

  if( isButtonPushed()) {
    waiting_for_button_release = true;
  } else {
    // A0 is low
    if( waiting_for_button_release) {
      waiting_for_button_release = false;
      cycle_mode();
    }
  }
}
