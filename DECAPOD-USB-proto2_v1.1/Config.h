/*  Config.h
 *
 *  DECAPOD USB - shared configuration: pin map, system enum,
 *  controller types, and timing constants used across modules.
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

#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Detected target system. Set once at boot by SystemDetect, then read by the
// per-system controller modules and by Gamepad_ to pick the HID descriptor.
// ---------------------------------------------------------------------------
#define NOT_SELECTED 0
#define NES_         1
#define SNES_        2
#define NEOGEO_      3
#define GENESIS_     4

// ---------------------------------------------------------------------------
// Analog probe pins used during boot to identify which carrier board is
// attached. Each pin pulls a different system's detect line; whichever one
// makes PD5 read HIGH wins. See SystemDetect::detect().
// ---------------------------------------------------------------------------
static const int pinSNES    = A0;
static const int pinNES     = A2;
static const int pinNEOGEO  = A1;
static const int pinGENESIS = A3;

// ---------------------------------------------------------------------------
// USB HID gamepad count.
// GAMEPAD_COUNT is the active number of controllers reported on USB.
// GAMEPAD_COUNT_MAX is the storage upper bound. The Arduino HID stack
// breaks (serial gets confused) past two devices, so GAMEPAD_COUNT stays 2.
// ---------------------------------------------------------------------------
#define GAMEPAD_COUNT      2
#define GAMEPAD_COUNT_MAX  4

// ---------------------------------------------------------------------------
// NES / SNES shift-register protocol timing.
// Values are slightly tighter than the official spec but proven reliable.
// CYCLES_* are 16 MHz CPU cycles consumed by __builtin_avr_delay_cycles().
//   1 cycle @ 16 MHz = 62.5 ns, so 128 cycles = 8 us.
// ---------------------------------------------------------------------------
#define BUTTON_READ_DELAY  20   // us between consecutive button reads
#define MICROS_LATCH_NES    8   // 12 us per spec
#define MICROS_CLOCK_NES    4   //  6 us per spec
#define MICROS_PAUSE_NES    4   //  6 us per spec
#define CYCLES_LATCH      128   // ~8 us SNES latch pulse
#define CYCLES_CLOCK       64   // ~4 us SNES clock high/low
#define CYCLES_PAUSE       64   // ~4 us SNES inter-pulse spacing

#define DELAY_CYCLES(n)  __builtin_avr_delay_cycles(n)

// ---------------------------------------------------------------------------
// Direction bits packed into the per-frame buttons word, shared by every
// controller module so the X/Y axis math is uniform.
// ---------------------------------------------------------------------------
#define UP     0x01
#define DOWN   0x02
#define LEFT   0x04
#define RIGHT  0x08

// SNES NTT Data Keypad: bit 29 set = NTT extension is present.
#define NTT_CONTROL_BIT 0x20000000

// SNES auto-detect can identify three sub-protocols on the same connector.
enum ControllerType {
  NONE,
  NES,
  SNES,
  NTT
};
