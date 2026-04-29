/*  SystemDetect.cpp - see SystemDetect.h for the license header. */

#include "SystemDetect.h"

#include "Config.h"

namespace SystemDetect {

int detect() {
  // PD5 is the shared "did this carrier respond?" sense line. Pull it up
  // through the internal resistor and let each candidate analogWrite drive
  // its own detect pin to 255 in turn.
  DDRD  &= ~B00100000;  // PD5 input
  PORTD |=  B00100000;  // internal pull-up

  pinMode(pinSNES,    OUTPUT);
  pinMode(pinNES,     OUTPUT);
  pinMode(pinNEOGEO,  OUTPUT);
  pinMode(pinGENESIS, OUTPUT);

  int system = NOT_SELECTED;

  analogWrite(pinSNES, 255);
  if (((PIND & B00100000) >> 5) == 1) {
    system = SNES_;
  } else {
    analogWrite(pinSNES, 0);
    analogWrite(pinNES, 255);
    if (((PIND & B00100000) >> 5) == 1) {
      system = NES_;
    } else {
      analogWrite(pinNES, 0);
      analogWrite(pinNEOGEO, 255);
      if (((PIND & B00100000) >> 5) == 1) {
        system = NEOGEO_;
      } else {
        analogWrite(pinNEOGEO, 0);
        analogWrite(pinGENESIS, 255);
        if (((PIND & B00100000) >> 5) == 1) {
          system = GENESIS_;
        } else {
          analogWrite(pinGENESIS, 0);
          system = NOT_SELECTED;
        }
      }
    }
  }

  // Release detect pins so per-system code can reuse them as inputs.
  pinMode(pinSNES,    INPUT);
  pinMode(pinNES,     INPUT);
  pinMode(pinNEOGEO,  INPUT);
  pinMode(pinGENESIS, INPUT);

  return system;
}

void applySerial(int system, char* gp_serial) {
  // gp_serial layout: "OCTOPOD <SYS>\0", suffix written from offset 8.
  switch (system) {
    case NES_:     memcpy(&gp_serial[8], "NES",     4); break;
    case SNES_:    memcpy(&gp_serial[8], "SNES",    5); break;
    case NEOGEO_:  memcpy(&gp_serial[8], "NEOGEO",  7); break;
    case GENESIS_: memcpy(&gp_serial[8], "GENESIS", 8); break;
  }
}

}  // namespace SystemDetect
