/*  SystemDetect.cpp - see SystemDetect.h for the license header. */

#include "SystemDetect.h"

#include <avr/wdt.h>

#include "Config.h"

namespace SystemDetect {

int detect() {
  // PD5 is the shared "did this carrier respond?" sense line. Pull it up
  // through the internal resistor and let each candidate digitalWrite drive
  // its own detect pin HIGH in turn (a steady high is enough; analogWrite
  // would needlessly configure a PWM Timer).
  DDRD  &= ~B00100000;  // PD5 input
  PORTD |=  B00100000;  // internal pull-up

  pinMode(pinSNES,    OUTPUT);
  pinMode(pinNES,     OUTPUT);
  pinMode(pinNEOGEO,  OUTPUT);
  pinMode(pinGENESIS, OUTPUT);

  int system = NOT_SELECTED;

  digitalWrite(pinSNES, HIGH);
  if (((PIND & B00100000) >> 5) == 1) {
    system = SNES_;
  } else {
    digitalWrite(pinSNES, LOW);
    digitalWrite(pinNES, HIGH);
    if (((PIND & B00100000) >> 5) == 1) {
      system = NES_;
    } else {
      digitalWrite(pinNES, LOW);
      digitalWrite(pinNEOGEO, HIGH);
      if (((PIND & B00100000) >> 5) == 1) {
        system = NEOGEO_;
      } else {
        digitalWrite(pinNEOGEO, LOW);
        digitalWrite(pinGENESIS, HIGH);
        if (((PIND & B00100000) >> 5) == 1) {
          system = GENESIS_;
        } else {
          digitalWrite(pinGENESIS, LOW);
          system = NOT_SELECTED;
        }
      }
    }
  }

  // Release detect pins so per-system code can reuse them as inputs.
  pinMode(pinSNES,    INPUT);
  pinMode(pinNES,     INPUT);
  pinMode(pinNEOGEO,  INPUT);
  pinMode(pinGENESIS, INPUT);

  return system;
}

void checkAndReboot(int currentSystem) {
  // Cadence: ~100 ms. Cheap millis() gate keeps the per-frame cost at one
  // 32-bit subtract+compare; the actual probe runs ~10x/second.
  static uint32_t lastCheck = 0;
  uint32_t now = millis();
  if ((uint32_t)(now - lastCheck) < 100) return;
  lastCheck = now;

  // Snapshot the runner's pin map. detect() drives PD5 + A0..A3 (PORTF),
  // and leaves all four detect pins as INPUT no-pullup on exit, which
  // would corrupt the runner's latch/clock/data setup if not restored.
  uint8_t savePORTD = PORTD;
  uint8_t saveDDRD  = DDRD;
  uint8_t savePORTF = PORTF;
  uint8_t saveDDRF  = DDRF;

  int detected = detect();

  // Restore PORT before DDR: a brief INPUT_PULLUP is harmless, but a brief
  // OUTPUT_HIGH on PD5 would look like a spurious latch pulse to the
  // attached carrier.
  PORTD = savePORTD;
  DDRD  = saveDDRD;
  PORTF = savePORTF;
  DDRF  = saveDDRF;

  if (detected != currentSystem) {
    // Different carrier (or carrier removed). Reboot via watchdog so
    // setup() re-runs detect() and Gamepad_ binds the right HID descriptor
    // for the new device on USB re-enumeration.
    wdt_enable(WDTO_15MS);
    while (1) {}
  }
}

void applySerial(int system, char* gp_serial) {
  // gp_serial layout: "OCTOPOD <SYS>\0", suffix written from offset 8.
  switch (system) {
    case NES_:     memcpy(&gp_serial[8], "NES",     4); break;
    case SNES_:    memcpy(&gp_serial[8], "SNES",    5); break;
    case NEOGEO_:  memcpy(&gp_serial[8], "NEOGEO",  7); break;
    case GENESIS_: memcpy(&gp_serial[8], "GENESIS", 8); break;
  }
}

}  // namespace SystemDetect
