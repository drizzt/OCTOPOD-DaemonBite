/*  SystemDetect.h
 *
 *  DECAPOD USB - boot-time auto-detection of which carrier board is
 *  attached (SNES / NES / NEOGEO / GENESIS), driven by a 1-of-N analog
 *  pull-up probe on PD5.
 *
 *  Originally part of DECAPOD-DaemonBite by Guille Asin Prieto (GAsinPrieto)
 *    https://github.com/GAsinPrieto/DECAPOD-DaemonBite
 *  Based on DaemonBite Retro Controllers USB by Mikael Norrgard (MickGyver)
 *    https://github.com/MickGyver/DaemonBite-Retro-Controllers-USB
 *  Refactor (c) 2026 Timothy Redaelli
 *
 *  GNU GENERAL PUBLIC LICENSE Version 3 - see LICENSE in project root.
 */

#pragma once

#include <Arduino.h>

namespace SystemDetect {

// Probes pinSNES/pinNES/pinNEOGEO/pinGENESIS in turn; the first one whose
// drive makes PD5 read HIGH wins. Returns NES_ / SNES_ / NEOGEO_ / GENESIS_
// or NOT_SELECTED if no carrier responds.
int detect();

// Patch the suffix of a 16-char gp_serial buffer with the detected system
// name (e.g. "OCTOPOD NES").
void applySerial(int system, char* gp_serial);

// Periodic carrier-change probe. Throttled to ~100 ms via an internal
// millis() gate, saves/restores PORTD/DDRD/PORTF/DDRF around detect() so
// the active runner's pin map is preserved. If the probe reports a system
// different from currentSystem (including NOT_SELECTED on carrier removal),
// triggers a watchdog reset so setup() re-binds the right HID descriptor.
// Never returns in the reboot case.
void checkAndReboot(int currentSystem);

}  // namespace SystemDetect
