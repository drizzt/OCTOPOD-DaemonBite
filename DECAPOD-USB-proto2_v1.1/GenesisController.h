/*  GenesisController.h
 *
 *  DECAPOD USB - Mega Drive / Genesis (3 / 6 button), Master System and
 *  Atari controller read loop. Wraps SegaControllers32U4.
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

namespace GenesisController {

// Initialises both DB9 ports and enters an infinite poll loop. Never returns.
void run(Gamepad_* Gamepad[]);

}  // namespace GenesisController
