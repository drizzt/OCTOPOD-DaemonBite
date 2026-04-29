/*  NESController.h
 *
 *  DECAPOD USB - NES (Famicom) controller read loop.
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

namespace NESController {

// Sets up data/clock/latch lines, then enters an infinite read loop that
// emits a USB HID report on every change. Returns only via watchdog reset
// when SystemDetect::checkAndReboot detects a carrier swap. `system` is
// the currently-bound system enum value, used by the periodic detect probe.
void run(Gamepad_* Gamepad[], int system);

}  // namespace NESController
