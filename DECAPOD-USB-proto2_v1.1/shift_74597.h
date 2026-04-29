/*  shift_74597.h
 *
 *  Driver for a chain of 74HC597 parallel-in / serial-out shift registers,
 *  used by the DECAPOD NEO GEO carrier to multiplex two 8-button gamepads.
 *
 *  Originally by Daniel J Bailey
 *    https://github.com/danieljabailey/shift_74597
 *  Modified for improved efficiency and ported to DECAPOD-DaemonBite by
 *    Guille Asin Prieto (GAsinPrieto)
 *    https://github.com/GAsinPrieto/DECAPOD-DaemonBite
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

#ifndef SHIFT_74597_INCLUDE
#define SHIFT_74597_INCLUDE

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

// us between consecutive control-line edges. 74HC597 @ 5V needs ~10 ns
// minimum, so 1 us keeps a ~100x margin while cutting NeoGeo poll cost
// from ~480 us to ~50 us.
#define DELAY 1

class shift_74597 {
public:
  shift_74597(int QH) : _QH(QH) {}
  ~shift_74597() {}

  // Configure SCK / SLOAD as outputs and the QH data line as a pulled-up
  // input. Call once before any load()/getByte() calls.
  void init();

  // Pulse RCK / SLOAD to capture the parallel inputs into the register.
  void load();

  // Clock 8 bits MSB-first off QH and return them.
  char getByte();

private:
  int _QH;
};

#endif /*74597_INCLUDE*/
