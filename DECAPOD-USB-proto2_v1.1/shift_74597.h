#ifndef SHIFT_74597_INCLUDE
#define SHIFT_74597_INCLUDE

#if (ARDUINO >= 100)
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define DELAY 10

class shift_74597 {
public:
	shift_74597(int QH) : _QH(QH) {}

	~shift_74597() {}

	void init() {
		DDRD |= B00010000;   // PD4-output - SCK
		DDRD &= ~B00000010;  // PD1-input - QH
		PORTD |= B00000010;  // enable internal pull-ups
		DDRD |= B00100000;   // PD5-output - RCK/SLOAD

		PORTD |= B00100000;  // high
		PORTD &= ~B00110000;  // low
	}

	void load() {
		PORTD |= B00100000;  // high
		delayMicroseconds(DELAY);
		PORTD &= ~B00100000;  // low
		delayMicroseconds(DELAY);
		PORTD &= ~B00100000;  // low
		delayMicroseconds(DELAY);
		PORTD |= B00100000;  // high
		delayMicroseconds(DELAY);
	}

	char getByte() {
		char result = 0;
		for (int i = 0; i <= 7; i++) {
			if (digitalRead(_QH) == HIGH) { result |= (1 << (7 - i)); }
			PORTD |= B00010000;  // high
			delayMicroseconds(DELAY);
			PORTD &= ~B00010000;  // low
			delayMicroseconds(DELAY);
		}
		return result;
	}

private:
	int _QH;
};

#endif /*74597_INCLUDE*/
