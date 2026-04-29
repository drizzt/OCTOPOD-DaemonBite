/*  Gamepad.h
 *   
 *  Based on the advanced HID library for Arduino: 
 *  https://github.com/NicoHood/HID
 *  Copyright (c) 2014-2015 NicoHood
 * 
 *  Copyright (c) 2020 Mikael Norrgård <http://daemonbite.com>
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
 *  
 */

#pragma once

#include <Arduino.h>
#include <HID.h>

#include "Config.h"

extern char gp_serial[16];

typedef struct {
  uint8_t buttons : 4;

  int8_t X;
  int8_t Y;
} GamepadReport_NES;

typedef struct {
  uint32_t buttons : 24;

  int8_t X;
  int8_t Y;
} GamepadReport_SNES;

typedef struct {
  uint8_t buttons : 8;

  int8_t X;
  int8_t Y;
} GamepadReport_NEOGEO;

typedef struct {
  uint16_t buttons : 9;

  int8_t X;
  int8_t Y;
} GamepadReport_GENESIS;

typedef struct {
  uint16_t buttons : 9;

  int8_t X;
  int8_t Y;
} GamepadReport;

class Gamepad_ : public PluggableUSBModule {
protected:
  int getInterface(uint8_t* interfaceCount);
  int getDescriptor(USBSetup& setup);
  uint8_t getShortName(char* name);
  bool setup(USBSetup& setup);

  uint8_t epType[1];
  uint8_t protocol;
  uint8_t idle;

  int SISTEMAgp = NOT_SELECTED;

  // Report buffer + length picked once at ctor based on SYSTEM, so
  // send()/reset() avoid a per-frame switch on SISTEMAgp.
  void*   _reportPtr  = nullptr;
  uint8_t _reportSize = 0;

public:
  GamepadReport_NES _GamepadReport_NES;
  GamepadReport_SNES _GamepadReport_SNES;
  GamepadReport_NEOGEO _GamepadReport_NEOGEO;
  GamepadReport_GENESIS _GamepadReport_GENESIS;
  GamepadReport _GamepadReport;
  Gamepad_(int SYSTEM);
  void reset() {
    memset(_reportPtr, 0, _reportSize);
    this->send();
  }

  void send() {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, _reportPtr, _reportSize);
  }
};
