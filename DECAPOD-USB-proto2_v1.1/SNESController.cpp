/*  SNESController.cpp - see SNESController.h for the license header. */

#include "SNESController.h"

#include "Config.h"
#include "SystemDetect.h"

namespace {

// PIND bit per gamepad data line (PD1, PD2, PD3, PD4).
const uint8_t gpBit[GAMEPAD_COUNT_MAX] = { B00000010, B00000100, B00001000, B00010000 };

// 32 possible bits in the SNES report:
//   [0..11]  standard SNES gamepad buttons + directions
//   [12..15] NTT Data Keypad upper button bank
//   [16..31] NTT keypad keys
uint32_t buttons[GAMEPAD_COUNT_MAX]     = { 0, 0, 0, 0 };
uint32_t buttonsPrev[GAMEPAD_COUNT_MAX] = { 0, 0, 0, 0 };

ControllerType controllerType[GAMEPAD_COUNT_MAX] = { NONE, NONE, NONE, NONE };

const uint32_t btnBits[32] = {
  // Standard SNES (B, Y, Select, Start, then UDLR, then A, X, L, R)
  0x10, 0x40, 0x400, 0x800, UP, DOWN, LEFT, RIGHT, 0x20, 0x80, 0x100, 0x200,
  // NTT Data Keypad (NDK10) extension
  0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x1000, 0x2000, 0x4000, 0x8000,
  0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
  0x1000000, 0x2000000, 0x4000000, 0x8000000
};

// Number of clock cycles to read per frame. Drops to 8 (NES) or 12 (SNES)
// once detectControllerTypes() identifies the lightest connected variant.
uint8_t buttonCount = 32;

// Per-gp report mask + NES-remap flag, populated once at end of
// detectControllerTypes(). Lets the poll loop avoid a per-frame branch on
// controllerType[].
uint32_t reportMask[GAMEPAD_COUNT_MAX]    = { 0, 0, 0, 0 };
bool     needsNesRemap[GAMEPAD_COUNT_MAX] = { false, false, false, false };

inline void sendLatch() {
  PORTD |=  B00100000;
  DELAY_CYCLES(CYCLES_LATCH);
  PORTD &= ~B00100000;
  DELAY_CYCLES(CYCLES_PAUSE);
}

inline void sendClock() {
  PORTD |=  B10010000;
  DELAY_CYCLES(CYCLES_CLOCK);
  PORTD &= ~B10010000;
  DELAY_CYCLES(CYCLES_PAUSE);
}

void detectControllerTypes() {
  uint8_t buttonCountNew = 0;

  // Read the controllers a handful of frames to settle the type guess.
  for (uint8_t i = 0; i < 40; i++) {
    sendLatch();

    for (uint8_t btn = 0; btn < buttonCount; btn++) {
      for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
        (PIND & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn]
                           : buttons[gp] |=  btnBits[btn];
      }
      sendClock();
    }

    // Pick the heaviest variant any pad needs and size buttonCount for it.
    for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
      if ((buttons[gp] & 0xF3A0) == 0xF3A0) {  // NES-only signature
        if (controllerType[gp] != SNES && controllerType[gp] != NTT)
          controllerType[gp] = NES;
        if (buttonCountNew < 8) buttonCountNew = 8;
      } else if (buttons[gp] & NTT_CONTROL_BIT) {
        controllerType[gp] = NTT;
        buttonCountNew = 32;
      } else {
        if (controllerType[gp] != NTT) controllerType[gp] = SNES;
        if (buttonCountNew < 12) buttonCountNew = 12;
      }
    }
  }

  buttonCount = buttonCountNew;

  // Freeze the per-gp report mask + remap flag so the poll loop can apply
  // them without re-checking controllerType[].
  for (byte gp = 0; gp < GAMEPAD_COUNT_MAX; gp++) {
    switch (controllerType[gp]) {
      case NES:
        reportMask[gp]    = 0xC3F;
        needsNesRemap[gp] = true;
        break;
      case NTT:
        reportMask[gp]    = 0x3FFFFFF;
        needsNesRemap[gp] = false;
        break;
      case SNES:
        reportMask[gp]    = 0xFFF;
        needsNesRemap[gp] = false;
        break;
      default:
        reportMask[gp]    = 0;
        needsNesRemap[gp] = false;
        break;
    }
  }
}

}  // namespace

void SNESController::run(Gamepad_* Gamepad[], int system) {
  // Latch + clock outputs (PD7, PD5, PD4); data inputs with pull-ups.
  DDRD  |=  B10110000;
  PORTD &= ~B10110000;
  DDRD  &= ~B00000110;
  PORTD |=  B00000110;

  delay(500);

  detectControllerTypes();

  for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
    Gamepad[gp]->reset();
    Gamepad[gp]->send();
  }

  // Loop body always exceeds BUTTON_READ_DELAY (latch+8/12/32 clocks+USB
  // > 20us), so the previous micros() gate was a guaranteed-true check;
  // dropping it saves two micros() calls per poll without changing behaviour.
  while (1) {
    SystemDetect::checkAndReboot(system);

    sendLatch();

    for (uint8_t btn = 0; btn < buttonCount; btn++) {
      for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
        (PIND & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn]
                           : buttons[gp] |=  btnBits[btn];
      }
      sendClock();
    }

    // Apply the precomputed per-gp mask + (if NES) Select/Start remap.
    for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
      if (needsNesRemap[gp]) {
        bitWrite(buttons[gp], 5, bitRead(buttons[gp], 4));
        bitWrite(buttons[gp], 4, bitRead(buttons[gp], 6));
      }
      buttons[gp] &= reportMask[gp];
    }

    for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
      if (buttons[gp] != buttonsPrev[gp]) {
        Gamepad[gp]->_GamepadReport_SNES.buttons = (buttons[gp] >> 4);
        Gamepad[gp]->_GamepadReport_SNES.Y = ((buttons[gp] & DOWN)  >> 1) -  (buttons[gp] & UP);
        Gamepad[gp]->_GamepadReport_SNES.X = ((buttons[gp] & RIGHT) >> 3) - ((buttons[gp] & LEFT) >> 2);
        buttonsPrev[gp] = buttons[gp];
        Gamepad[gp]->send();
      }
    }
  }
}
