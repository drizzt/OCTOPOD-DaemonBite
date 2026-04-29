/*  NeoGeoController.cpp - see NeoGeoController.h for the license header. */

#include "NeoGeoController.h"

#include "Config.h"
#include "shift_74597.h"

namespace {

// QH (serial out) of the 74HC597 chain is hardwired to PD1; the driver
// reads PIND directly, so no pin number needs to be passed in.
shift_74597 shifter;

uint8_t  axes[2]        = { 0x0f, 0x0f };
uint8_t  axesPrev[2]    = { 0x0f, 0x0f };

uint16_t buttons[2]     = { 0, 0 };
uint16_t buttonsPrev[2] = { 0, 0 };

bool usbUpdate = false;

// Mirror the low 4 bits of `n` into the low 4 bits of the result. Both
// call sites only ever pass a 4-bit value and shift the result by 4, so
// reversing one nibble (LUT, ~4 cycles) replaces the prior 8-if reverse.
const uint8_t kNibbleRev[16] = {
  0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
  0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF
};

}  // namespace

void NeoGeoController::run(Gamepad_* Gamepad[]) {
  shifter.init();

  buttons[0] = 0;
  buttons[1] = 0;

  for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
    Gamepad[gp]->reset();
    Gamepad[gp]->send();
  }

  while (1) {
    shifter.load();
    char in0 = shifter.getByte();
    char in1 = shifter.getByte();
    char in2 = shifter.getByte();

    // Three bytes pack two controllers' axes + 8 buttons each.
    // Bitwise NOT yields 1 = pressed. Axes use a 4-bit mirror via LUT.
    axes[0]    = ~(kNibbleRev[in0 & 0x0F] << 4);
    buttons[0] = ~((in0 & B11110000) >> 4
                 | (in1 & B00001100) << 2
                 | (in1 & B00000011) << 6
                 | (B11111111 << 8));

    axes[1]    = ~(kNibbleRev[(in1 >> 4) & 0x0F] << 4);
    buttons[1] = ~((in2 & B00001111)
                 | (in2 & B00110000) << 2
                 | (in2 & B11000000) >> 2
                 | (B11110000 << 4));

    for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
      if (axes[gp] != axesPrev[gp]) {
        // SOCD policy: UP+DOWN -> NEUTRAL, LEFT+RIGHT -> NEUTRAL.
        Gamepad[gp]->_GamepadReport_NEOGEO.Y = ((axes[gp] & B01000000) >> 6) - ((axes[gp] & B10000000) >> 7);
        Gamepad[gp]->_GamepadReport_NEOGEO.X = ((axes[gp] & B00010000) >> 4) - ((axes[gp] & B00100000) >> 5);
        axesPrev[gp] = axes[gp];
        usbUpdate = true;
      }

      if (buttons[gp] != buttonsPrev[gp]) {
        // Swap USB B7/B8 (bits 6/7) so SELECT=B7, START=B8.
        uint16_t out = buttons[gp];
        out = (out & ~0xC0u) | ((out & 0x40u) << 1) | ((out & 0x80u) >> 1);
        Gamepad[gp]->_GamepadReport_NEOGEO.buttons = out;
        buttonsPrev[gp] = buttons[gp];
        usbUpdate = true;
      }

      if (usbUpdate) {
        Gamepad[gp]->send();
        usbUpdate = false;
      }
    }
  }
}
