/*  Gamepad.cpp
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
#include "Gamepad.h"

//GENERIC
static const uint8_t _hidReportDescriptor[] PROGMEM = {
  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,  // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,  // COLLECTION (Application)
  0xa1, 0x00,  // COLLECTION (Physical)

  0x05, 0x09,  // USAGE_PAGE (Button)
  0x19, 0x01,  // USAGE_MINIMUM (Button 1)
  0x29, 0x09,  // USAGE_MAXIMUM (Button 9)
  0x15, 0x00,  // LOGICAL_MINIMUM (0)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x09,  // REPORT_COUNT (9)
  0x75, 0x01,  // REPORT_SIZE (1)
  0x81, 0x02,  // INPUT (Data,Var,Abs)

  0x95, 0x01,  // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
  0x75, 0x07,  // REPORT_SIZE (7)
  0x81, 0x03,  // INPUT (Const,Var,Abs)

  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x01,  // USAGE (pointer)
  0xa1, 0x00,  // COLLECTION (Physical)
  0x09, 0x30,  // USAGE (X)
  0x09, 0x31,  // USAGE (Y)
  0x15, 0xff,  // LOGICAL_MINIMUM (-1)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x02,  // REPORT_COUNT (2)
  0x75, 0x08,  // REPORT_SIZE (8)
  0x81, 0x02,  // INPUT (Data,Var,Abs)
  0xc0,        // END_COLLECTION

  0xc0,  // END_COLLECTION
  0xc0,  // END_COLLECTION
};

//NES
static const uint8_t _hidReportDescriptorNES[] PROGMEM = {
  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,  // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,  // COLLECTION (Application)
  0xa1, 0x00,  // COLLECTION (Physical)

  0x05, 0x09,  // USAGE_PAGE (Button)
  0x19, 0x01,  // USAGE_MINIMUM (Button 1)
  0x29, 0x04,  // USAGE_MAXIMUM (Button 4)
  0x15, 0x00,  // LOGICAL_MINIMUM (0)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x04,  // REPORT_COUNT (4)
  0x75, 0x01,  // REPORT_SIZE (1)
  0x81, 0x02,  // INPUT (Data,Var,Abs)

  0x95, 0x01,  // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
  0x75, 0x04,  // REPORT_SIZE (4)
  0x81, 0x03,  // INPUT (Const,Var,Abs)

  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x01,  // USAGE (pointer)
  0xa1, 0x00,  // COLLECTION (Physical)
  0x09, 0x30,  // USAGE (X)
  0x09, 0x31,  // USAGE (Y)
  0x15, 0xff,  // LOGICAL_MINIMUM (-1)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x02,  // REPORT_COUNT (2)
  0x75, 0x08,  // REPORT_SIZE (8)
  0x81, 0x02,  // INPUT (Data,Var,Abs)
  0xc0,        // END_COLLECTION

  0xc0,  // END_COLLECTION
  0xc0,  // END_COLLECTION
};

//SNES
static const uint8_t _hidReportDescriptorSNES[] PROGMEM = {
  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,  // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,  // COLLECTION (Application)
  0xa1, 0x00,  // COLLECTION (Physical)

  0x05, 0x09,  // USAGE_PAGE (Button)
  0x19, 0x01,  // USAGE_MINIMUM (Button 1)
  0x29, 0x18,  // USAGE_MAXIMUM (Button 24)
  0x15, 0x00,  // LOGICAL_MINIMUM (0)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x18,  // REPORT_COUNT (24)
  0x75, 0x01,  // REPORT_SIZE (1)
  0x81, 0x02,  // INPUT (Data,Var,Abs)

  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x01,  // USAGE (pointer)
  0xa1, 0x00,  // COLLECTION (Physical)
  0x09, 0x30,  // USAGE (X)
  0x09, 0x31,  // USAGE (Y)
  0x15, 0xff,  // LOGICAL_MINIMUM (-1)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x02,  // REPORT_COUNT (2)
  0x75, 0x08,  // REPORT_SIZE (8)
  0x81, 0x02,  // INPUT (Data,Var,Abs)
  0xc0,        // END_COLLECTION

  0xc0,  // END_COLLECTION
  0xc0,  // END_COLLECTION
};

//NEOGEO
static const uint8_t _hidReportDescriptorNG[] PROGMEM = {
  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,  // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,  // COLLECTION (Application)
  0xa1, 0x00,  // COLLECTION (Physical)
  0x05, 0x09,  // USAGE_PAGE (Button)
  0x19, 0x01,  // USAGE_MINIMUM (Button 1)
  0x29, 0x08,  // USAGE_MAXIMUM (Button 8)
  0x15, 0x00,  // LOGICAL_MINIMUM (0)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x08,  // REPORT_COUNT (8)
  0x75, 0x01,  // REPORT_SIZE (1)
  0x81, 0x02,  // INPUT (Data,Var,Abs)

  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x01,  // USAGE (pointer)
  0xa1, 0x00,  // COLLECTION (Physical)
  0x09, 0x30,  // USAGE (X)
  0x09, 0x31,  // USAGE (Y)
  0x15, 0xff,  // LOGICAL_MINIMUM (-1)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x02,  // REPORT_COUNT (2)
  0x75, 0x08,  // REPORT_SIZE (8)
  0x81, 0x02,  // INPUT (Data,Var,Abs)
  0xc0,        // END_COLLECTION

  0xc0,  // END_COLLECTION
  0xc0,  // END_COLLECTION
};

//GENESIS
static const uint8_t _hidReportDescriptorGEN[] PROGMEM = {
  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,  // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,  // COLLECTION (Application)
  0xa1, 0x00,  // COLLECTION (Physical)

  0x05, 0x09,  // USAGE_PAGE (Button)
  0x19, 0x01,  // USAGE_MINIMUM (Button 1)
  0x29, 0x09,  // USAGE_MAXIMUM (Button 9)
  0x15, 0x00,  // LOGICAL_MINIMUM (0)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x09,  // REPORT_COUNT (9)
  0x75, 0x01,  // REPORT_SIZE (1)
  0x81, 0x02,  // INPUT (Data,Var,Abs)

  0x95, 0x01,  // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
  0x75, 0x07,  // REPORT_SIZE (7)
  0x81, 0x03,  // INPUT (Const,Var,Abs)

  0x05, 0x01,  // USAGE_PAGE (Generic Desktop)
  0x09, 0x01,  // USAGE (pointer)
  0xa1, 0x00,  // COLLECTION (Physical)
  0x09, 0x30,  // USAGE (X)
  0x09, 0x31,  // USAGE (Y)
  0x15, 0xff,  // LOGICAL_MINIMUM (-1)
  0x25, 0x01,  // LOGICAL_MAXIMUM (1)
  0x95, 0x02,  // REPORT_COUNT (2)
  0x75, 0x08,  // REPORT_SIZE (8)
  0x81, 0x02,  // INPUT (Data,Var,Abs)
  0xc0,        // END_COLLECTION

  0xc0,  // END_COLLECTION
  0xc0,  // END_COLLECTION
};

Gamepad_::Gamepad_(int SYSTEM)
  : PluggableUSBModule(1, 1, epType), protocol(HID_REPORT_PROTOCOL), idle(1) {
  SISTEMAgp = SYSTEM;
  epType[0] = EP_TYPE_INTERRUPT_IN;

  // Cache report buffer ptr + size so send()/reset() are switch-free.
  switch (SYSTEM) {
    case NES_:
      _reportPtr  = &_GamepadReport_NES;
      _reportSize = sizeof(GamepadReport_NES);
      break;
    case SNES_:
      _reportPtr  = &_GamepadReport_SNES;
      _reportSize = sizeof(GamepadReport_SNES);
      break;
    case NEOGEO_:
      _reportPtr  = &_GamepadReport_NEOGEO;
      _reportSize = sizeof(GamepadReport_NEOGEO);
      break;
    case GENESIS_:
      _reportPtr  = &_GamepadReport_GENESIS;
      _reportSize = sizeof(GamepadReport_GENESIS);
      break;
    default:
      _reportPtr  = &_GamepadReport;
      _reportSize = sizeof(GamepadReport);
      break;
  }

  PluggableUSB().plug(this);
}

int Gamepad_::getInterface(uint8_t* interfaceCount) {
  *interfaceCount += 1;  // uses 1
  switch (SISTEMAgp) {
    case NES_:
      {
        HIDDescriptor hidInterface = {
          D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
          D_HIDREPORT(sizeof(_hidReportDescriptorNES)),
          D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
        };
        return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
      }

    case SNES_:
      {
        HIDDescriptor hidInterface = {
          D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
          D_HIDREPORT(sizeof(_hidReportDescriptorSNES)),
          D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
        };
        return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
      }

    case NEOGEO_:
      {
        HIDDescriptor hidInterface = {
          D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
          D_HIDREPORT(sizeof(_hidReportDescriptorNG)),
          D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
        };
        return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
      }

    case GENESIS_:
      {
        HIDDescriptor hidInterface = {
          D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
          D_HIDREPORT(sizeof(_hidReportDescriptorGEN)),
          D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
        };
        return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
      }

    default:
      {
        HIDDescriptor hidInterface = {
          D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
          D_HIDREPORT(sizeof(_hidReportDescriptor)),
          D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
        };
        return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
      }
  }
}

int Gamepad_::getDescriptor(USBSetup& setup) {
  // Check if this is a HID Class Descriptor request
  if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
  if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

  // In a HID Class Descriptor wIndex cointains the interface number
  if (setup.wIndex != pluggedInterface) { return 0; }

  // Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
  // due to the USB specs, but Windows and Linux just assumes its in report mode.
  protocol = HID_REPORT_PROTOCOL;

  switch (SISTEMAgp) {
    case NES_:
      return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorNES, sizeof(_hidReportDescriptorNES));

    case SNES_:
      return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorSNES, sizeof(_hidReportDescriptorSNES));

    case NEOGEO_:
      return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorNG, sizeof(_hidReportDescriptorNG));

    case GENESIS_:
      return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorGEN, sizeof(_hidReportDescriptorGEN));

    default:
      return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor, sizeof(_hidReportDescriptor));
  }
}

bool Gamepad_::setup(USBSetup& setup) {
  if (pluggedInterface != setup.wIndex) {
    return false;
  }

  uint8_t request = setup.bRequest;
  uint8_t requestType = setup.bmRequestType;

  if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
    if (request == HID_GET_REPORT) {
      // TODO: HID_GetReport();
      return true;
    }
    if (request == HID_GET_PROTOCOL) {
      // TODO: Send8(protocol);
      return true;
    }
  }

  if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
    if (request == HID_SET_PROTOCOL) {
      protocol = setup.wValueL;
      return true;
    }
    if (request == HID_SET_IDLE) {
      idle = setup.wValueL;
      return true;
    }
    if (request == HID_SET_REPORT) {
    }
  }

  return false;
}

uint8_t Gamepad_::getShortName(char* name) {
  if (!next) {
    strcpy(name, gp_serial);
    return strlen(name);
  }
  return 0;
}
