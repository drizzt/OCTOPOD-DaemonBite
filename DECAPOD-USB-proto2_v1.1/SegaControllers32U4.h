//
// SegaControllers32U4.h
//
// Authors:
//       Jon Thysell <thysell@gmail.com>
//       Mikael Norrgård <mick@daemonbite.com>
//
// Copyright (c) 2017 Jon Thysell <http://jonthysell.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef SegaController32U4_h
#define SegaController32U4_h

#include <Arduino.h>

// Bitmasks for the SC_BTN_* set inside currentState[gp].
enum {
  SC_BTN_UP    = 1,
  SC_BTN_DOWN  = 2,
  SC_BTN_LEFT  = 4,
  SC_BTN_RIGHT = 8,
  SC_BTN_A     = 16,
  SC_BTN_B     = 32,
  SC_BTN_C     = 64,
  SC_BTN_X     = 128,
  SC_BTN_Y     = 256,
  SC_BTN_Z     = 512,
  SC_BTN_START = 1024,
  SC_BTN_MODE  = 2048,
  SC_BTN_HOME  = 4096,

  // Shift counts to extract direction bits as 0/1 for axis math.
  SC_BIT_SH_UP    = 0,
  SC_BIT_SH_DOWN  = 1,
  SC_BIT_SH_LEFT  = 2,
  SC_BIT_SH_RIGHT = 3,

  // Original DaemonBite pin map (kept as reference for the alternate map below).
  SC_PIN1_BIT = 0,
  SC_PIN2_BIT = 1,
  SC_PIN3_BIT = 2,
  SC_PIN4_BIT = 3,
  SC_PIN6_BIT = 4,
  SC_PIN9_BIT = 5,

  // DECAPOD wiring: DB9 pins land on PD bits as below for controller 1...
  DB9_PIN1_BIT1 = 1,  // 7
  DB9_PIN2_BIT1 = 3,  // 6
  DB9_PIN3_BIT1 = 4,  // 5
  DB9_PIN4_BIT1 = 6,  // 4
  DB9_PIN6_BIT1 = 2,  // 3
  DB9_PIN9_BIT1 = 7,
  // ...and as below for controller 2.
  DB9_PIN1_BIT2 = 3,
  DB9_PIN2_BIT2 = 2,
  DB9_PIN3_BIT2 = 1,
  DB9_PIN4_BIT2 = 0,
  DB9_PIN6_BIT2 = 4,
  DB9_PIN9_BIT2 = 7
};

// Delay (us) between asserting the select pin and reading the data pins.
const byte SC_CYCLE_DELAY = 10;

class SegaControllers32U4 {
public:
  SegaControllers32U4(void);

  // Pull a fresh sample for both controllers into currentState[].
  // Cycles the select line and runs the 8-state Mega Drive read sequence.
  void readState();

  // Configure DDR/PORT registers for the DB9 pins. Call once at startup.
  void setup_controllers();

  // Latest button bitfield per port. Bit layout: see SC_BTN_* above.
  word currentState[2];

private:
  // "Normal" Six button controller reading routine, done a bit differently in this project
  // Cycle  TH out  TR in  TL in  D3 in  D2 in  D1 in  D0 in
  // 0      LO      Start  A      0      0      Down   Up
  // 1      HI      C      B      Right  Left   Down   Up
  // 2      LO      Start  A      0      0      Down   Up      (Check connected and read Start and A in this cycle)
  // 3      HI      C      B      Right  Left   Down   Up      (Read B, C and directions in this cycle)
  // 4      LO      Start  A      0      0      0      0       (Check for six button controller in this cycle)
  // 5      HI      C      B      Mode   X      Y      Z       (Read X,Y,Z and Mode in this cycle)
  // 6      LO      ---    ---    ---    ---    ---    Home    (Home only for 8bitdo wireless gamepads)
  // 7      HI      ---    ---    ---    ---    ---    ---
  void readPort1();
  void readPort2();

  boolean _pinSelect1;
  boolean _pinSelect2;

  byte _ignoreCycles[2];

  boolean _connected[2];
  boolean _sixButtonMode[2];

  byte _inputReg1;
  byte _inputReg2;
};

#endif
