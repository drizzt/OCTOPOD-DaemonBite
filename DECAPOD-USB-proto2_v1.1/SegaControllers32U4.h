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

enum {
  SC_BTN_UP = 1,
  SC_BTN_DOWN = 2,
  SC_BTN_LEFT = 4,
  SC_BTN_RIGHT = 8,
  SC_BTN_A = 16,
  SC_BTN_B = 32,
  SC_BTN_C = 64,
  SC_BTN_X = 128,
  SC_BTN_Y = 256,
  SC_BTN_Z = 512,
  SC_BTN_START = 1024,
  SC_BTN_MODE = 2048,
  SC_BTN_HOME = 4096,
  SC_BIT_SH_UP = 0,
  SC_BIT_SH_DOWN = 1,
  SC_BIT_SH_LEFT = 2,
  SC_BIT_SH_RIGHT = 3,
  SC_PIN1_BIT = 0,
  SC_PIN2_BIT = 1,
  SC_PIN3_BIT = 2,
  SC_PIN4_BIT = 3,
  SC_PIN6_BIT = 4,
  SC_PIN9_BIT = 5,
  DB9_PIN1_BIT1 = 1,  //7,
  DB9_PIN2_BIT1 = 3,  //6,
  DB9_PIN3_BIT1 = 4,  //5,
  DB9_PIN4_BIT1 = 6,  //4,
  DB9_PIN6_BIT1 = 2,  //3,
  DB9_PIN9_BIT1 = 7,
  DB9_PIN1_BIT2 = 3,
  DB9_PIN2_BIT2 = 2,
  DB9_PIN3_BIT2 = 1,
  DB9_PIN4_BIT2 = 0,
  DB9_PIN6_BIT2 = 4,
  DB9_PIN9_BIT2 = 7
};

const byte SC_CYCLE_DELAY = 10;  // Delay (µs) between setting the select pin and reading the button pins

class SegaControllers32U4 {
public:
  SegaControllers32U4(void) {
    // Setup input pins (A0,A1,A2,A3,14,15 or PF7,PF6,PF5,PF4,PB3,PB1)
    _pinSelect1 = true;
    _pinSelect2 = true;

    for (byte i = 0; i <= 1; i++) {
      currentState[i] = 0;
      _connected[i] = 0;
      _sixButtonMode[i] = false;
      _ignoreCycles[i] = 0;
    }
  }

  void readState() {
    PORTD &= B11111110;  // Controller select pin low - controller 1
    delayMicroseconds(2);
    // Set the select pins low/high
    _pinSelect1 = !_pinSelect1;
    if (!_pinSelect1) {
      PORTD &= ~B00100000;
    } else {
      PORTD |= B00100000;
    }

    delayMicroseconds(2);

    PORTD |= B00000001;  // Controller select pin high - controller 2

    delayMicroseconds(2);

    // Set the select pins low/high
    _pinSelect2 = !_pinSelect2;
    if (!_pinSelect2) {
      PORTD &= ~B00100000;
    } else {
      PORTD |= B00100000;
    }

    delayMicroseconds(2);

    PORTD &= B11111110;  // Controller select pin low - controller 1

    // Short delay to stabilise outputs in controller
    delayMicroseconds(SC_CYCLE_DELAY - 8);

    // Read all input registers
    _inputReg1 = PIND;

    PORTD |= B00000001;  // Controller select pin high - controller 2

    delayMicroseconds(6);

    _inputReg2 = PIND;

    readPort1();
    readPort2();
  }

  void setup_controllers() {
    // Setup input pins (TXO,RXI,2,3,4,6 or PD3,PD2,PD1,PD0,PD4,PD7)
    DDRD &= ~B11011110;  // input
    PORTD |= B11011110;  // high to enable internal pull-up

    DDRE |= B01000000;
    DDRD |= B00100001;   // Select pins as output
    PORTD |= B00100000;  // Select pins high
    PORTE |= B01000000;
  }

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
  void readPort1() {
    if (_ignoreCycles[0] <= 0) {
      if (_pinSelect1)  // Select pin is HIGH
      {
        if (_connected[0]) {
          // Check if six button mode is active
          if (_sixButtonMode[0]) {
            // Read input pins for X, Y, Z, Mode  //PD6 y PD4, PD3, PD1 en lugar de PF4-7
            (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW) ? currentState[0] |= SC_BTN_Z : currentState[0] &= ~SC_BTN_Z;        //UP
            (bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW) ? currentState[0] |= SC_BTN_Y : currentState[0] &= ~SC_BTN_Y;        //DW
            (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW) ? currentState[0] |= SC_BTN_X : currentState[0] &= ~SC_BTN_X;        //L
            (bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW) ? currentState[0] |= SC_BTN_MODE : currentState[0] &= ~SC_BTN_MODE;  //R
            _sixButtonMode[0] = false;
            _ignoreCycles[0] = 2;  // Ignore the two next cycles (cycles 6 and 7 in table above)
          } else {
            // Read input pins for Up, Down, Left, Right, B, C
            (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW) ? currentState[0] |= SC_BTN_UP : currentState[0] &= ~SC_BTN_UP;
            (bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW) ? currentState[0] |= SC_BTN_DOWN : currentState[0] &= ~SC_BTN_DOWN;
            (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW) ? currentState[0] |= SC_BTN_LEFT : currentState[0] &= ~SC_BTN_LEFT;
            (bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW) ? currentState[0] |= SC_BTN_RIGHT : currentState[0] &= ~SC_BTN_RIGHT;
            (bitRead(_inputReg1, DB9_PIN6_BIT1) == LOW) ? currentState[0] |= SC_BTN_B : currentState[0] &= ~SC_BTN_B;  //A
            (bitRead(_inputReg1, DB9_PIN9_BIT1) == LOW) ? currentState[0] |= SC_BTN_C : currentState[0] &= ~SC_BTN_C;  //START
          }
        } else  // No Mega Drive controller is connected, use SMS/Atari mode
        {
          // Clear current state
          currentState[0] = 0;

          // Read input pins for Up, Down, Left, Right, Fire1, Fire2
          if (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW) { currentState[0] |= SC_BTN_UP; }
          if (bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW) { currentState[0] |= SC_BTN_DOWN; }
          if (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW) { currentState[0] |= SC_BTN_LEFT; }
          if (bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW) { currentState[0] |= SC_BTN_RIGHT; }
          if (bitRead(_inputReg1, DB9_PIN6_BIT1) == LOW) { currentState[0] |= SC_BTN_A; }
          if (bitRead(_inputReg1, DB9_PIN9_BIT1) == LOW) { currentState[0] |= SC_BTN_B; }
        }
      } else  // Select pin is LOW
      {
        // Check if a controller is connected
        _connected[0] = (bitRead(_inputReg1, DB9_PIN3_BIT1) == LOW && bitRead(_inputReg1, DB9_PIN4_BIT1) == LOW);

        // Check for six button mode
        _sixButtonMode[0] = (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW && bitRead(_inputReg1, DB9_PIN2_BIT1) == LOW);

        // Read input pins for A and Start
        if (_connected[0]) {
          if (!_sixButtonMode[0]) {
            (bitRead(_inputReg1, DB9_PIN6_BIT1) == LOW) ? currentState[0] |= SC_BTN_A : currentState[0] &= ~SC_BTN_A;
            (bitRead(_inputReg1, DB9_PIN9_BIT1) == LOW) ? currentState[0] |= SC_BTN_START : currentState[0] &= ~SC_BTN_START;
          }
        }
      }
    } else {
      if (_ignoreCycles[0]-- == 2)  // Decrease the ignore cycles counter and read 8bitdo home in first "ignored" cycle, this cycle is unused on normal 6-button controllers
      {
        (bitRead(_inputReg1, DB9_PIN1_BIT1) == LOW) ? currentState[0] |= SC_BTN_HOME : currentState[0] &= ~SC_BTN_HOME;
      }
    }
  }

  void readPort2() {
    if (_ignoreCycles[1] <= 0) {
      if (_pinSelect2)  // Select pin is HIGH
      {
        if (_connected[1]) {
          // Check if six button mode is active
          if (_sixButtonMode[1]) {
            // Read input pins for X, Y, Z, Mode
            (bitRead(_inputReg2, DB9_PIN1_BIT1) == LOW) ? currentState[1] |= SC_BTN_Z : currentState[1] &= ~SC_BTN_Z;        //UP
            (bitRead(_inputReg2, DB9_PIN2_BIT1) == LOW) ? currentState[1] |= SC_BTN_Y : currentState[1] &= ~SC_BTN_Y;        //DW
            (bitRead(_inputReg2, DB9_PIN3_BIT1) == LOW) ? currentState[1] |= SC_BTN_X : currentState[1] &= ~SC_BTN_X;        //L
            (bitRead(_inputReg2, DB9_PIN4_BIT1) == LOW) ? currentState[1] |= SC_BTN_MODE : currentState[1] &= ~SC_BTN_MODE;  //R
            _sixButtonMode[1] = false;
            _ignoreCycles[1] = 2;  // Ignore the two next cycles (cycles 6 and 7 in table above)
          } else {
            // Read input pins for Up, Down, Left, Right, B, C
            (bitRead(_inputReg2, DB9_PIN1_BIT1) == LOW) ? currentState[1] |= SC_BTN_UP : currentState[1] &= ~SC_BTN_UP;
            (bitRead(_inputReg2, DB9_PIN2_BIT1) == LOW) ? currentState[1] |= SC_BTN_DOWN : currentState[1] &= ~SC_BTN_DOWN;
            (bitRead(_inputReg2, DB9_PIN3_BIT1) == LOW) ? currentState[1] |= SC_BTN_LEFT : currentState[1] &= ~SC_BTN_LEFT;
            (bitRead(_inputReg2, DB9_PIN4_BIT1) == LOW) ? currentState[1] |= SC_BTN_RIGHT : currentState[1] &= ~SC_BTN_RIGHT;
            (bitRead(_inputReg2, DB9_PIN6_BIT1) == LOW) ? currentState[1] |= SC_BTN_B : currentState[1] &= ~SC_BTN_B;  //A
            (bitRead(_inputReg2, DB9_PIN9_BIT1) == LOW) ? currentState[1] |= SC_BTN_C : currentState[1] &= ~SC_BTN_C;  //START
          }
        } else  // No Mega Drive controller is connected, use SMS/Atari mode
        {
          // Clear current state
          currentState[1] = 0;

          // Read input pins for Up, Down, Left, Right, Fire1, Fire2
          if (bitRead(_inputReg2, DB9_PIN1_BIT1) == LOW) { currentState[1] |= SC_BTN_UP; }
          if (bitRead(_inputReg2, DB9_PIN2_BIT1) == LOW) { currentState[1] |= SC_BTN_DOWN; }
          if (bitRead(_inputReg2, DB9_PIN3_BIT1) == LOW) { currentState[1] |= SC_BTN_LEFT; }
          if (bitRead(_inputReg2, DB9_PIN4_BIT1) == LOW) { currentState[1] |= SC_BTN_RIGHT; }
          if (bitRead(_inputReg2, DB9_PIN6_BIT1) == LOW) { currentState[1] |= SC_BTN_A; }
          if (bitRead(_inputReg2, DB9_PIN9_BIT1) == LOW) { currentState[1] |= SC_BTN_B; }
        }
      } else  // Select pin is LOW
      {
        // Check if a controller is connected
        _connected[1] = (bitRead(_inputReg2, DB9_PIN3_BIT1) == LOW && bitRead(_inputReg2, DB9_PIN4_BIT1) == LOW);

        // Check for six button mode
        _sixButtonMode[1] = (bitRead(_inputReg2, DB9_PIN1_BIT1) == LOW && bitRead(_inputReg2, DB9_PIN2_BIT1) == LOW);

        // Read input pins for A and Start
        if (_connected[1]) {
          if (!_sixButtonMode[1]) {
            (bitRead(_inputReg2, DB9_PIN6_BIT1) == LOW) ? currentState[1] |= SC_BTN_A : currentState[1] &= ~SC_BTN_A;
            (bitRead(_inputReg2, DB9_PIN9_BIT1) == LOW) ? currentState[1] |= SC_BTN_START : currentState[1] &= ~SC_BTN_START;
          }
        }
      }
    } else {
      if (_ignoreCycles[1]-- == 2)  // Decrease the ignore cycles counter and read 8bitdo home in first "ignored" cycle, this cycle is unused on normal 6-button controllers
      {
        (bitRead(_inputReg2, DB9_PIN1_BIT1) == LOW) ? currentState[1] |= SC_BTN_HOME : currentState[1] &= ~SC_BTN_HOME;
      }
    }
  }

  boolean _pinSelect1;
  boolean _pinSelect2;

  byte _ignoreCycles[2];

  boolean _connected[2];
  boolean _sixButtonMode[2];

  byte _inputReg1;
  byte _inputReg2;
};

#endif
