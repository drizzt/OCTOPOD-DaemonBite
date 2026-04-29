/*  SNESController.h
 *
 *  DECAPOD USB - SNES controller read loop with auto-detect of plain NES,
 *  SNES, and SNES NTT Data Keypad (NDK10) variants.
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

#include "Gamepad.h"

namespace SNESController {

// Sets up data/clock/latch lines, probes the controllers a few frames to
// distinguish NES / SNES / NTT, then enters an infinite read loop.
// Returns only via watchdog reset on carrier swap. `system` is the
// currently-bound system enum value, used by the periodic detect probe.
void run(Gamepad_* Gamepad[], int system);

}  // namespace SNESController
