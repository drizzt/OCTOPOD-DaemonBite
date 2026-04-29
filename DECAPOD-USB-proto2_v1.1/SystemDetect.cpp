/*  SystemDetect.cpp - see SystemDetect.h for the license header. */

#include "SystemDetect.h"

#include "Config.h"

namespace SystemDetect {

int detect() {
  // PD5 is the shared "did this carrier respond?" sense line. Pull it up
  // through the internal resistor and let each candidate digitalWrite drive
  // its own detect pin HIGH in turn (a steady high is enough; analogWrite
  // would needlessly configure a PWM Timer).
  DDRD  &= ~B00100000;  // PD5 input
  PORTD |=  B00100000;  // internal pull-up

  pinMode(pinSNES,    OUTPUT);
  pinMode(pinNES,     OUTPUT);
  pinMode(pinNEOGEO,  OUTPUT);
  pinMode(pinGENESIS, OUTPUT);

  int system = NOT_SELECTED;

  digitalWrite(pinSNES, HIGH);
  if (((PIND & B00100000) >> 5) == 1) {
    system = SNES_;
  } else {
    digitalWrite(pinSNES, LOW);
    digitalWrite(pinNES, HIGH);
    if (((PIND & B00100000) >> 5) == 1) {
      system = NES_;
    } else {
      digitalWrite(pinNES, LOW);
      digitalWrite(pinNEOGEO, HIGH);
      if (((PIND & B00100000) >> 5) == 1) {
        system = NEOGEO_;
      } else {
        digitalWrite(pinNEOGEO, LOW);
        digitalWrite(pinGENESIS, HIGH);
        if (((PIND & B00100000) >> 5) == 1) {
          system = GENESIS_;
        } else {
          digitalWrite(pinGENESIS, LOW);
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
