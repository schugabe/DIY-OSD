
#include "Arduino.h"
#include <avr/interrupt.h>

volatile uint8_t rising;
#define catchRisingEdge()  {TCCR1B |= (1<<ICES1); TIFR1 |= (1<<ICF1); rising = 1;}
#define catchFallingEdge() {TCCR1B &= ~(1<<ICES1); TIFR1 |= (1<<ICF1); rising = 0;}

volatile uint16_t start_time, stop_time;

ISR(TIMER1_CAPT_vect) {        
        if (rising) {
          uint16_t period, width;
          period = ICR1 - start_time;
          width = stop_time - start_time;
          start_time = ICR1;
          
          Serial.print(period); 
	  Serial.write(",");
          Serial.print(width);
          Serial.println();
          catchFallingEdge();
        }
        else {
          stop_time = ICR1;
          catchRisingEdge();
        }
}

void initTimer(void) {
	TCCR1A = 0;
	TCCR1B = (0<<ICNC1) | (0<<ICES1) | (1<<CS11);
	TCCR1C = 0;

	catchRisingEdge();

	TIFR1 = (1<<ICF1);
	TIMSK1 = (1<<ICIE1);

	pinMode(8, INPUT);
	digitalWrite(8, 1);
}


void setup(void) {
	pinMode(13, OUTPUT);
	digitalWrite(13,HIGH);

	Serial.begin(57600);
	initTimer();
}

void loop(void) {
	delay(500);
        digitalWrite(13, !digitalRead(13));
}

