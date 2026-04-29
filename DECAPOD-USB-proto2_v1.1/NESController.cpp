/*  NESController.cpp - see NESController.h for the license header. */

#include "NESController.h"

#include "Config.h"

namespace {

// Standard NES controller has 4 buttons and 4 directional inputs.
constexpr uint8_t BUTTON_COUNT = 8;

// Per-pad current/previous button bitfields.
uint8_t  buttons[GAMEPAD_COUNT_MAX]     = { 0, 0, 0, 0 };
uint8_t  buttonsPrev[GAMEPAD_COUNT_MAX] = { 0, 0, 0, 0 };

// PIND bit per gamepad data line (PD1, PD2, PD3, PD4).
const uint8_t gpBit[GAMEPAD_COUNT_MAX] = { B00000010, B00000100, B00001000, B00010000 };

// Bit assignments inside the NES report: A, B, Select, Start, then dirs.
const uint8_t btnBits[BUTTON_COUNT] = { 0x20, 0x10, 0x40, 0x80, UP, DOWN, LEFT, RIGHT };

inline void sendLatch() {
  PORTD |=  B00100000;
  delayMicroseconds(MICROS_LATCH_NES);
  PORTD &= ~B00100000;
  delayMicroseconds(MICROS_PAUSE_NES);
}

inline void sendClock() {
  PORTD |=  B10010000;
  delayMicroseconds(MICROS_CLOCK_NES);
  PORTD &= ~B10010000;
  delayMicroseconds(MICROS_PAUSE_NES);
}

}  // namespace

void NESController::run(Gamepad_* Gamepad[]) {
  // Latch + clock lines as outputs (PD7, PD5, PD4); data pins (PD1, PD2)
  // as inputs with pull-ups so an absent controller floats high (=released).
  DDRD  |=  B10110000;
  PORTD &= ~B10110000;
  DDRD  &= ~B00000110;
  PORTD |=  B00000110;

  delay(500);  // Settle USB enumeration before first report burst.

  for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
    Gamepad[gp]->reset();
    Gamepad[gp]->send();
  }

  // Loop body always exceeds BUTTON_READ_DELAY (latch+clocks+USB > 20us),
  // so the previous micros() gate was a guaranteed-true check; dropping it
  // saves two micros() calls per poll without changing behaviour.
  while (1) {
    sendLatch();
    for (uint8_t btn = 0; btn < BUTTON_COUNT; btn++) {
      for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
        (PIND & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn]
                           : buttons[gp] |=  btnBits[btn];
      }
      sendClock();
    }

    for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
      if (buttons[gp] != buttonsPrev[gp]) {
        // Top 4 bits = action buttons, bottom 4 = directions.
        Gamepad[gp]->_GamepadReport_NES.buttons = (buttons[gp] >> 4);
        Gamepad[gp]->_GamepadReport_NES.Y = ((buttons[gp] & DOWN)  >> 1) -  (buttons[gp] & UP);
        Gamepad[gp]->_GamepadReport_NES.X = ((buttons[gp] & RIGHT) >> 3) - ((buttons[gp] & LEFT) >> 2);
        buttonsPrev[gp] = buttons[gp];
        Gamepad[gp]->send();
      }
    }
  }
}
