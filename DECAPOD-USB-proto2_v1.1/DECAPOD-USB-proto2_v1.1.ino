/*  DECAPOD-USB-proto2_v1.1.ino
 *
 *  DECAPOD USB - main sketch. Detects which retro-controller carrier board
 *  is attached at boot, then dispatches to the matching per-system read
 *  loop. The loop never returns (each runner spins forever).
 *
 *  Originally part of DECAPOD-DaemonBite by Guille Asin Prieto (GAsinPrieto)
 *    https://github.com/GAsinPrieto/DECAPOD-DaemonBite
 *  Based on DaemonBite Retro Controllers USB by Mikael Norrgard (MickGyver)
 *    https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB
 *  Refactor (c) 2026 Timothy Redaelli
 *
 *  GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "Config.h"
#include "Gamepad.h"
#include "SystemDetect.h"
#include "NESController.h"
#include "SNESController.h"
#include "NeoGeoController.h"
#include "GenesisController.h"

// USB serial string reported by the gamepad. SystemDetect::applySerial
// patches the suffix at boot, e.g. "OCTOPOD NES".
char gp_serial[16] = "OCTOPOD ";

// Detected target system, set in setup().
static int SISTEMA = NOT_SELECTED;

void setup() {
  // Wait until USB has actually enumerated. While waiting, force PD/PF as
  // hi-Z so a not-yet-powered carrier board cannot back-feed the MCU.
  while (!(UDADDR & _BV(ADDEN))) {
    DDRD  = B00000000;
    PORTD = B00000000;
    DDRF  = B00000000;
    PORTF = B00000000;
  }

  SISTEMA = SystemDetect::detect();
  SystemDetect::applySerial(SISTEMA, gp_serial);
}

void loop() {
  // Each Gamepad_ ctor self-registers with PluggableUSB by linking itself
  // into a global module list, so we MUST construct each instance directly
  // (no brace-init, which would copy from a temporary and leave a dangling
  // entry in that list). Hence two named instances + a pointer fan-out.
  static Gamepad_ Gamepad0(SISTEMA);
  static Gamepad_ Gamepad1(SISTEMA);
  Gamepad_* Gamepad[GAMEPAD_COUNT] = { &Gamepad0, &Gamepad1 };

  switch (SISTEMA) {
    case NES_:     NESController::run(Gamepad);     break;
    case SNES_:    SNESController::run(Gamepad);    break;
    case NEOGEO_:  NeoGeoController::run(Gamepad);  break;
    case GENESIS_: GenesisController::run(Gamepad); break;
    case NOT_SELECTED:
    default:
      // No carrier detected: idle the report so the host still sees a
      // valid (empty) gamepad.
      for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
        Gamepad[gp]->reset();
        Gamepad[gp]->send();
      }
      break;
  }
}
