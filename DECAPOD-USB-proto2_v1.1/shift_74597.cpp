/*  shift_74597.cpp - see shift_74597.h for the license header. */

#include "shift_74597.h"

void shift_74597::init() {
  DDRD  |=  B00010000;  // PD4 output - SCK
  DDRD  &= ~B00000010;  // PD1 input  - QH
  PORTD |=  B00000010;  // enable internal pull-up on QH
  DDRD  |=  B00100000;  // PD5 output - RCK / SLOAD

  PORTD |=  B00100000;
  PORTD &= ~B00110000;
}

void shift_74597::load() {
  PORTD |=  B00100000;
  delayMicroseconds(DELAY);
  PORTD &= ~B00100000;
  delayMicroseconds(DELAY);
  PORTD &= ~B00100000;
  delayMicroseconds(DELAY);
  PORTD |=  B00100000;
  delayMicroseconds(DELAY);
}

char shift_74597::getByte() {
  // QH is hardwired to PD1 (see init()), so a direct PIND read avoids the
  // pin->port lookup that digitalRead() does on every bit.
  char result = 0;
  for (int i = 0; i <= 7; i++) {
    if (PIND & B00000010) { result |= (1 << (7 - i)); }
    PORTD |=  B00010000;
    delayMicroseconds(DELAY);
    PORTD &= ~B00010000;
    delayMicroseconds(DELAY);
  }
  return result;
}
