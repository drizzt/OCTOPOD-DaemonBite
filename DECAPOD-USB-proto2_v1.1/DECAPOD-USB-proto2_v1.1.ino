#include "Gamepad.h"
#include "SegaControllers32U4.h"
#include "shift_74597.h"

//NEO GEO
#define QH 2 //PD1 IN

shift_74597 myShifter = shift_74597(QH);

bool usbUpdate = false;     // Should gamepad data be sent to USB?

uint8_t  axesDirect[2] = {0x0f, 0x0f};
uint8_t  axes[2] = {0x0f, 0x0f};
uint8_t  axesPrev[2] = {0x0f, 0x0f};
uint8_t  axesBits[4] = {0x10, 0x20, 0x40, 0x80};

uint16_t buttonsDirect[2] = {0, 0};
uint16_t buttons_NG[2] = {0, 0};
uint16_t buttonsPrev_NG[2] = {0, 0};
uint16_t buttonsBits[12] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800};

char myInput0;
char myInput1;
char myInput2;

uint8_t reverse(uint8_t in)
{
  uint8_t out;
  out = 0;
  if (in & 0x01) out |= 0x80;
  if (in & 0x02) out |= 0x40;
  if (in & 0x04) out |= 0x20;
  if (in & 0x08) out |= 0x10;
  if (in & 0x10) out |= 0x08;
  if (in & 0x20) out |= 0x04;
  if (in & 0x40) out |= 0x02;
  if (in & 0x80) out |= 0x01;

  return (out);
}







//NES
#define GAMEPAD_COUNT 2		// NOTE: No more than TWO gamepads are possible at the moment due to a USB HID issue.
#define GAMEPAD_COUNT_MAX 4  // NOTE: For some reason, can't have more than two gamepads without serial breaking. Can someone figure out why?
//       (It has something to do with how Arduino handles HID devices)
#define BUTTON_COUNT       8 // Standard NES controller has four buttons and four axes, totalling 8
#define BUTTON_READ_DELAY 20 // Delay between button reads in µs
#define MICROS_LATCH_NES       8 // 12µs according to specs (8 seems to work fine)
#define MICROS_CLOCK_NES       4 //  6µs according to specs (4 seems to work fine)
#define MICROS_PAUSE_NES       4 //  6µs according to specs (4 seems to work fine)

//SNES;
#define MICROS_LATCH_SNES      10 // 12µs according to specs (8 seems to work fine)
#define MICROS_CLOCK_SNES       5 //  6µs according to specs (4 seems to work fine)
#define MICROS_PAUSE_SNES       5 //  6µs according to specs (4 seems to work fine)

#define CYCLES_LATCH     128 // 12µs according to specs (8 seems to work fine) (1 cycle @ 16MHz takes 62.5ns so 62.5ns * 128 = 8000ns = 8µs)
#define CYCLES_CLOCK      64 //  6µs according to specs (4 seems to work fine)
#define CYCLES_PAUSE      64 //  6µs according to specs (4 seems to work fine)


enum ControllerType {
  NONE,
  NES,
  SNES,
  NTT
};

//GENESIS
SegaControllers32U4 controllers;

// Controller previous states
word lastState[2] = {1, 1};




#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08

#define NTT_CONTROL_BIT 0x20000000

#define DELAY_CYCLES(n) __builtin_avr_delay_cycles(n)

// Controllers
uint8_t buttons[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
uint8_t buttonsPrev[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
//uint8_t gpBit[GAMEPAD_COUNT_MAX] = {B10000000,B01000000,B00100000,B00010000};
uint8_t gpBit[GAMEPAD_COUNT_MAX] = {B00000010, B00000100, B00001000, B00010000};


//SNES
uint32_t buttons_SNES[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
uint32_t buttonsPrev_SNES[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
ControllerType controllerType[GAMEPAD_COUNT_MAX] = {NONE, NONE, NONE, NONE};
uint32_t btnBits_SNES[32] = {0x10, 0x40, 0x400, 0x800, UP, DOWN, LEFT, RIGHT, 0x20, 0x80, 0x100, 0x200, // Standard SNES controller
                             0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x1000, 0x2000, 0x4000, 0x8000, // NTT Data Keypad (NDK10)
                             0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
                             0x1000000, 0x2000000, 0x4000000, 0x8000000
                            };
uint8_t buttonCount = 32;

uint8_t btnBits_NES[BUTTON_COUNT] = {0x20, 0x10, 0x40, 0x80, UP, DOWN, LEFT, RIGHT};








// Timing
uint32_t microsButtons = 0;










const int pinSNES = A0;
const int pinNES = A2;
const int pinNEOGEO = A1;
const int pinGENESIS = A3;


int outputValueSNES = 0;        // value output to the PWM (analog out)
int outputValueNES = 0;        // value output to the PWM (analog out)
int outputValueNEOGEO = 0;        // value output to the PWM (analog out)
int outputValueGENESIS = 0;        // value output to the PWM (analog out)
int SISTEMA = NOT_SELECTED;

char gp_serial[16] = "OCTOPOD ";


void setup() {
  while(!(UDADDR & _BV(ADDEN))){ //check USB connection
    //SerialNotInit=true;
    DDRD  = B00000000;
    PORTD = B00000000;
    DDRF  = B00000000;
    PORTF = B00000000;  
  }
  
  pinMode(pinSNES, OUTPUT);
  pinMode(pinNES, OUTPUT);
  pinMode(pinNEOGEO, OUTPUT);
  pinMode(pinGENESIS, OUTPUT);
  DDRD  &= ~B00100000; // inputs
  PORTD |=  B00100000; // enable internal pull-ups

  analogWrite(pinSNES, 255);
  if ((PIND & B00100000) >> 5 == 1) SISTEMA = SNES_; //Serial.println("SNES");
  else {
    analogWrite(pinSNES, 0);
    analogWrite(pinNES, 255);
    if ((PIND & B00100000) >> 5 == 1) SISTEMA = NES_; //Serial.println("NES");
    else {
      analogWrite(pinNES, 0);
      analogWrite(pinNEOGEO, 255);
      if ((PIND & B00100000) >> 5 == 1) SISTEMA = NEOGEO_; //Serial.println("NEOGEO");
      else {
        analogWrite(pinNEOGEO, 0);
        analogWrite(pinGENESIS, 255);
        if ((PIND & B00100000) >> 5 == 1) SISTEMA = GENESIS_; //Serial.println("GENESIS");
        else {
          analogWrite(pinGENESIS, 0);
          SISTEMA = NOT_SELECTED; //Serial.println("PCE");
        }
      }
    }
  }

  pinMode(pinSNES, INPUT);
  pinMode(pinNES, INPUT);
  pinMode(pinNEOGEO, INPUT);
  pinMode(pinGENESIS, INPUT);  

  switch (SISTEMA) {
    case NES_:
      memcpy(&gp_serial[8], "NES", 4);
      break;

    case SNES_:
      memcpy(&gp_serial[8], "SNES", 5);
      break;

    case NEOGEO_:
      memcpy(&gp_serial[8], "NEOGEO", 7);
      break;

    case GENESIS_:
      memcpy(&gp_serial[8], "GENESIS", 8);
      break;
  }
}

void loop() {
  Gamepad_ Gamepad[GAMEPAD_COUNT](SISTEMA);

  if (SISTEMA == NES_ || SISTEMA == SNES_) {
    DDRD  |=  B10110000; // output  
    PORTD &= ~B10110000; // low

    // Setup data pins (A0-A3 or PF7-PF4)
    DDRD  &= ~B00000110; // inputs
    PORTD |=  B00000110; // enable internal pull-ups

    delay(500);

    //SNES
    if (SISTEMA == SNES_) detectControllerTypes();
  }

	for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
		Gamepad[gp].reset();
    Gamepad[gp].send();
  }

  switch (SISTEMA) {
    case NOT_SELECTED:
      break;

    case NES_:
      while (1)
      {
        //Serial.println("NES");
        // See if enough time has passed since last button read
        if ((micros() - microsButtons) > BUTTON_READ_DELAY)
        {
          // Pulse latch
          sendLatch();
          for (uint8_t btn = 0; btn < BUTTON_COUNT; btn++)
          {
            for (byte gp = 0; gp < GAMEPAD_COUNT; gp++)
              //(PINF & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn] : buttons[gp] |= btnBits[btn];
              (PIND & gpBit[gp]) ? buttons[gp] &= ~btnBits_NES[btn] : buttons[gp] |= btnBits_NES[btn];
            sendClock();
          }

          for (byte gp = 0; gp < GAMEPAD_COUNT; gp++)
          {
            // Has any buttons changed state?
            if (buttons[gp] != buttonsPrev[gp])
            {
              Gamepad[gp]._GamepadReport_NES.buttons = (buttons[gp] >> 4); // First 4 bits are the axes
              Gamepad[gp]._GamepadReport_NES.Y = ((buttons[gp] & DOWN) >> 1) - (buttons[gp] & UP);
              Gamepad[gp]._GamepadReport_NES.X = ((buttons[gp] & RIGHT) >> 3) - ((buttons[gp] & LEFT) >> 2);
              buttonsPrev[gp] = buttons[gp];
              Gamepad[gp].send();
            }
          }

          microsButtons = micros();
        }
      }
      break;

    case SNES_:
      while (1)
      {
        //Serial.println("SNES");
        // See if enough time has passed since last button read
        if ((micros() - microsButtons) > BUTTON_READ_DELAY)
        { // Pulse latch
          sendLatch();

          for (uint8_t btn = 0; btn < buttonCount; btn++)
          {
            for (byte gp = 0; gp < GAMEPAD_COUNT; gp++)
              //(PINF & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn] : buttons[gp] |= btnBits[btn];
              (PIND & gpBit[gp]) ? buttons_SNES[gp] &= ~btnBits_SNES[btn] : buttons_SNES[gp] |= btnBits_SNES[btn];
            
            sendClock();
          }

          // Check gamepad type

          for (byte gp = 0; gp < GAMEPAD_COUNT; gp++)
          {
            if (controllerType[gp] == NES) {   // NES
              bitWrite(buttons_SNES[gp], 5, bitRead(buttons_SNES[gp], 4));
              bitWrite(buttons_SNES[gp], 4, bitRead(buttons_SNES[gp], 6));
              buttons_SNES[gp] &= 0xC3F;
              //Serial.println(buttons_SNES[gp]);
            }
            else if (controllerType[gp] == NTT) // SNES NTT Data Keypad
              buttons_SNES[gp] &= 0x3FFFFFF;
            else                               // SNES Gamepad
              buttons_SNES[gp] &= 0xFFF;
          }

          for (byte gp = 0; gp < GAMEPAD_COUNT; gp++)
          {
            // Has any buttons changed state?
            if (buttons_SNES[gp] != buttonsPrev_SNES[gp])
            {
              Gamepad[gp]._GamepadReport_SNES.buttons = (buttons_SNES[gp] >> 4); // First 4 bits are the axes
              Gamepad[gp]._GamepadReport_SNES.Y = ((buttons_SNES[gp] & DOWN) >> 1) - (buttons_SNES[gp] & UP);
              Gamepad[gp]._GamepadReport_SNES.X = ((buttons_SNES[gp] & RIGHT) >> 3) - ((buttons_SNES[gp] & LEFT) >> 2);
              buttonsPrev_SNES[gp] = buttons_SNES[gp];
              Gamepad[gp].send();
            }
          }

          microsButtons = micros();
        }
      }

      break;

    case NEOGEO_:
      myShifter.init();

      buttonsDirect[0] = 0;
      buttonsDirect[1] = 0;

      while (1)
      {
        //Serial.println("NEOGEO");

        myShifter.load();
        myInput0 = myShifter.getByte();
        myInput1 = myShifter.getByte();
        myInput2 = myShifter.getByte();

        // Read axis and button inputs (bitwise NOT results in a 1 when button/axis pressed)
        axesDirect[0] = ~(reverse(myInput0 & B00001111));//~(PINF & B11110000);
        buttonsDirect[0] = ~((myInput0 & B11110000) >> 4 | (myInput1 & B00001111) << 4 | (B11111111 << 8)); //~((PIND & B00011111) | ((PIND & B10000000) << 4) | ((PINB & B01111110) << 4));

        axesDirect[1] = ~((reverse(myInput1 & B11110000)) << 4); //~(PINF & B11110000);
        buttonsDirect[1] = ~((myInput2) | (B11110000 << 4));//~((PIND & B00011111) | ((PIND & B10000000) << 4) | ((PINB & B01111110) << 4));

        for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
          axes[gp] = axesDirect[gp];
          buttons_NG[gp] = buttonsDirect[gp];

          // Has axis inputs changed?
          if (axes[gp] != axesPrev[gp])
          {

            // UP + DOWN = UP, SOCD (Simultaneous Opposite Cardinal Directions) Cleaner
            // if (axes[gp] & B10000000)
            //   Gamepad[gp]._GamepadReport_NEOGEO.Y = -1;
            // else if (axes[gp] & B01000000)
            //   Gamepad[gp]._GamepadReport_NEOGEO.Y = 1;
            // else
            //   Gamepad[gp]._GamepadReport_NEOGEO.Y = 0;
            // UP + DOWN = NEUTRAL
            Gamepad[gp]._GamepadReport_NEOGEO.Y = ((axes[gp] & B01000000)>>6) - ((axes[gp] & B10000000)>>7);
            // LEFT + RIGHT = NEUTRAL
            Gamepad[gp]._GamepadReport_NEOGEO.X = ((axes[gp] & B00010000) >> 4) - ((axes[gp] & B00100000) >> 5);
            axesPrev[gp] = axes[gp];
            usbUpdate = true;
          }

          // Has button inputs changed?
          if (buttons_NG[gp] != buttonsPrev_NG[gp])
          {

            Gamepad[gp]._GamepadReport_NEOGEO.buttons = buttons_NG[gp];
            buttonsPrev_NG[gp] = buttons_NG[gp];
            usbUpdate = true;
          }

          // Should gamepad data be sent to USB?
          if (usbUpdate)
          {
            Gamepad[gp].send();
            usbUpdate = false;
          }
        }
      }
      break;

    case GENESIS_:
      controllers.setup_controllers();

      while (1)
      {
        for (byte gp = 0; gp < GAMEPAD_COUNT; gp++) {
          controllers.readState();
          
          if (controllers.currentState[gp] != lastState[gp])
          {
            Gamepad[gp]._GamepadReport_GENESIS.buttons = controllers.currentState[gp] >> 4;
            Gamepad[gp]._GamepadReport_GENESIS.Y = ((controllers.currentState[gp] & SC_BTN_DOWN) >> SC_BIT_SH_DOWN) - ((controllers.currentState[gp] & SC_BTN_UP) >> SC_BIT_SH_UP);
            Gamepad[gp]._GamepadReport_GENESIS.X = ((controllers.currentState[gp] & SC_BTN_RIGHT) >> SC_BIT_SH_RIGHT) - ((controllers.currentState[gp] & SC_BTN_LEFT) >> SC_BIT_SH_LEFT);
            Gamepad[gp].send();
            lastState[gp] = controllers.currentState[gp];
          }
        }
      }
      delay(10);
      break;
  }
}

void sendLatch()
{
  // Send a latch pulse to the NES controller(s)
  PORTD |=  B00100000; // Set HIGH
  if (SISTEMA == NES) delayMicroseconds(MICROS_LATCH_NES);
  else if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_LATCH);
  PORTD &= ~B00100000; // Set LOW
  if (SISTEMA == NES) delayMicroseconds(MICROS_PAUSE_NES);
  if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_PAUSE);
}

void sendClock()
{
  // Send a clock pulse to the NES controller(s)
  PORTD |=  B10010000; // Set HIGH
  if (SISTEMA == NES) delayMicroseconds(MICROS_CLOCK_NES);
  if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_CLOCK);
  PORTD &= ~B10010000; // Set LOW
  if (SISTEMA == NES) delayMicroseconds(MICROS_PAUSE_NES);
  if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_PAUSE);
}


//SNES
void detectControllerTypes()
{
  uint8_t buttonCountNew = 0;

  // Read the controllers a few times to detect controller type
  for (uint8_t i = 0; i < 40; i++)
  {
    // Pulse latch
    sendLatch();

    // Read all buttons
    for (uint8_t btn = 0; btn < buttonCount; btn++)
    {
      for (byte gp = 0; gp < GAMEPAD_COUNT; gp++)
        //(PINF & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn] : buttons[gp] |= btnBits[btn];
        (PIND & gpBit[gp]) ? buttons_SNES[gp] &= ~btnBits_SNES[btn] : buttons_SNES[gp] |= btnBits_SNES[btn];
      sendClock();
    }

    // Check controller types and set buttonCount to max needed
    for (byte gp = 0; gp < GAMEPAD_COUNT; gp++)
    {
      if ((buttons_SNES[gp] & 0xF3A0) == 0xF3A0) {  // NES
        if (controllerType[gp] != SNES && controllerType[gp] != NTT)
          controllerType[gp] = NES;
        if (buttonCountNew < 8)
          buttonCountNew = 8;
      }
      else if (buttons_SNES[gp] & NTT_CONTROL_BIT) { // SNES NTT Data Keypad
        controllerType[gp] = NTT;
        buttonCountNew = 32;
      }
      else {                                   // SNES Gamepad
        if (controllerType[gp] != NTT)
          controllerType[gp] = SNES;
        if (buttonCountNew < 12)
          buttonCountNew = 12;
      }
    }
  }

  // Set updated button count to avoid unneccesary button reads (for simpler controller types)
  buttonCount = buttonCountNew;
}
