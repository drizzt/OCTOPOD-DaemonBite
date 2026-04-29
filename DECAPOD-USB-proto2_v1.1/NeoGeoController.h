/*  NeoGeoController.h
 *
 *  DECAPOD USB - NEO GEO controller read loop. Inputs are clocked out of
 *  three 74HC597 shift registers via the shift_74597 helper.
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

namespace NeoGeoController {

// Initialises the shift register chain and enters an infinite poll loop.
// Never returns.
void run(Gamepad_* Gamepad[]);

}  // namespace NeoGeoController
