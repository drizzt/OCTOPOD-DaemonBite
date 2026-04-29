/*  GenesisController.cpp - see GenesisController.h for the license header. */

#include "GenesisController.h"

#include "Config.h"
#include "SegaControllers32U4.h"

namespace {

SegaControllers32U4 controllers;

word lastState[2] = { 1, 1 };

}  // namespace

void GenesisController::run(Gamepad_* Gamepad[]) {
  controllers.setup_controllers();

  for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
    Gamepad[gp]->reset();
    Gamepad[gp]->send();
  }

  while (1) {
    for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
      controllers.readState();

      if (controllers.currentState[gp] != lastState[gp]) {
        // Swap START (bit 10) and MODE (bit 11) so USB B7=MODE, B8=START.
        word state = controllers.currentState[gp];
        state = (state & ~(SC_BTN_START | SC_BTN_MODE))
              | ((state & SC_BTN_START) << 1)
              | ((state & SC_BTN_MODE)  >> 1);
        Gamepad[gp]->_GamepadReport_GENESIS.buttons = state >> 4;
        Gamepad[gp]->_GamepadReport_GENESIS.Y =
            ((controllers.currentState[gp] & SC_BTN_DOWN)  >> SC_BIT_SH_DOWN) -
            ((controllers.currentState[gp] & SC_BTN_UP)    >> SC_BIT_SH_UP);
        Gamepad[gp]->_GamepadReport_GENESIS.X =
            ((controllers.currentState[gp] & SC_BTN_RIGHT) >> SC_BIT_SH_RIGHT) -
            ((controllers.currentState[gp] & SC_BTN_LEFT)  >> SC_BIT_SH_LEFT);
        Gamepad[gp]->send();
        lastState[gp] = controllers.currentState[gp];
      }
    }
  }
}
