
// Written by Dennis Frie - 2011
// Contact: Dennis.frie@gmail.com

// Discussion:
// www.rcgroups.com/forums/showthread.php?t=1473207

// Support DIY hardware and SimpleOSD Open

// Notes on DIY hardware pins:;
// pin 6 = PD6 / AIN0 (Reference voltage)
// Pin 7 = PD7 / AIN1 (Video)
// Pin 11 = MOSI (SPI) used for pixel-generation. 
// Pin 8 = Dimming (Optional)

// GPS should be configured to NMEA with GPGGA and GPRMC strings.

#include "config.h"
#include "output.h"
#include "gps.h"

#include <util/setbaud.h>

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>

extern int line;

void setup() {
	// Set pin-modes:
	pinMode(10,OUTPUT);
	pinMode(11,OUTPUT); 
	pinMode(12,OUTPUT);
	pinMode(13,OUTPUT);

	pinMode(0,INPUT);  
	pinMode(1,INPUT);
	pinMode(2,INPUT);
	pinMode(3,INPUT);
	pinMode(4,INPUT);  
	pinMode(5,INPUT);
	pinMode(6,INPUT); 
	pinMode(7,INPUT);
	pinMode(8,INPUT);

	//Define SimpleOSD X2 dim pin
	#if (CONTROLLER == 2) 
		pinMode(dim_pin,OUTPUT);
	#else
		pinMode(dim_pin,INPUT);
	#endif

	// Init Serial communication. 
	Serial.begin(BAUD);
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	UCSR0A = 0b0000000;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (3<<UCSZ00);

	// Used to set the GPS update-rate to 5 hz, and GPGGA and GPRMC gps-strings (Only for MKT-GPS).

	Serial.print("$PMTK300,200,0,0,0,0*2F");
	Serial.write(13);
	Serial.write(10); 

	Serial.print("$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28");
	Serial.write(13);
	Serial.write(10);
 
	// Enable SPI for pixel generation
	// Set SPI;  
	SPCR = 
		(1<<SPE) | //Enable SPI
		(1<<MSTR)| // Set as master
		(0<<SPR1)| // Max speed
		(0<<SPR0)| // Max speed
		(0<<DORD)| // Year, Try to set this to 1 :D (Will mirror the byte)
		(1<<CPOL)| // Doesnt really matter, its the SCK polarity
		(1<<CPHA); //
  
	SPSR = (1<<SPI2X); // SPI double speed - we want 8 mhz output speed.
	SPDR =0b00000000; // IMPORTANT.. The SPI will idle random at low or high in the beginning. If it idles high you will get black screen = bad idea in FPV.
	// It will always idle at the last bit sent, so always be sure the last bit is 0. The main-loop and end of each line will always send 8 zeros - so it should
	// be pretty safe. 
 
	// Analog read enable and set prescale. You can change prescale if you like.
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (0<<ADPS1) | (1<<ADPS0);
  
	// Disable standard Arduino interrupts used for timer etc. (you can clearly see the text flicker if they are not disabled)
	TIMSK0=0;
	TIMSK1=0;
	TIMSK2=0;
  
	// If Arduino is used
	if (CONTROLLER == 0) {
		// Init analog comparator to register new line and frame
		ADCSRB = 0b00000001; // Set analog comparator mode
  
		ACSR = 0b00001011; //Enable Analog comparator interrupt 

		// Set interrupt on falling output edge.  
		ADCSRA|=(1<<ADSC);    
  
		// Start timer 2 - used to determine if it's a new frame or line. 
		TCCR2B =
			(0<<CS22) | //Prescale 1024
			(0<<CS21) | //Prescale 1024
			(1<<CS20) | //Prescale 1024
			(0<<WGM22); // CTC mode (Clear timer on compare match)
	}

	// If SimpleOSD XL /LM1881
	if (CONTROLLER ==1) {
		attachInterrupt(0,detectline,FALLING);
		attachInterrupt(1,detectframe,RISING);  
		pinMode(5,OUTPUT);
		digitalWrite(5,HIGH); // Turn on the led
	}

	// If SimpleOSD X2/LM1881
	if (CONTROLLER ==2) {
		attachInterrupt(0,detectline,FALLING);
		attachInterrupt(1,detectframe,RISING);  
		pinMode(13,INPUT);
		digitalWrite(13,HIGH); // Turn on the led
		
		// enable pwm rssi input
		#if (digital_rssi==1)
			DDRB &= ~_BV(0);
			PORTB |= _BV(0);
			// nothing needed here
			TCCR1A = 0;
			//  prescaler 8
			 TCCR1B = _BV(ICNC1) | _BV(ICES1) | _BV(CS11);
		#endif
	}
	
	// Button with internal pull-up.  
	pinMode(Buttonpin_,INPUT);
	digitalWrite(Buttonpin_,HIGH);

	// If the controller have not been configured yet. 
	if ((EEPROM.read(0) != 52) || (reset_values == 1)) {
		EEPROM.write(0,52);
  
		// Show_mah_km
		EEPROM.write(1,show_mah_km_);
  
		// Show decimals
		EEPROM.write(2,show_decimals_);
  
		// Altitude offset
		EEPROM.write(3,altitude_offset_on_);
  
		// Align text
		EEPROM.write(4,align_text_);
  
		// Show plane pos
		EEPROM.write(5,show_plane_pos_);  
  
		// Alt alarm
		EEPROM.write(10,(unsigned char) alt_alarm_);
		EEPROM.write(11,(unsigned char) (alt_alarm_>>8));  
  
		EEPROM.write(12,(unsigned char) los_alarm_);
		EEPROM.write(13,(unsigned char) (los_alarm_>>8));  

		EEPROM.write(14,(unsigned char) volt_alarm_);
		EEPROM.write(15,(unsigned char) (volt_alarm_>>8));  

		EEPROM.write(16,(unsigned char) mah_alarm_);
		EEPROM.write(17,(unsigned char) (mah_alarm_>>8));    
	}
}



// ====================================================================================================================================================================================================================
// ====================================================================================================================================================================================================================
// ====================================================================================================================================================================================================================
// ====================================================================================================================================================================================================================
// Analog comparator interrupt:
#if (CONTROLLER == 0)
ISR(ANALOG_COMP_vect) {
	// Reset counter
	TCNT2 =0; 

	// Wait for the sync to go high again;
	while ((ACSR & 0b00100000) == 0b00100000) { }  
      
	// If the counter has exceeded the "new line sync time", it's a new 
	// frame. In that case, we just need to reset the line-counter.  
	// 
	if (TCNT2 > 75) {
		line = 0;
	}  
  
	detectline(); 
}
#endif

// ====================================================================================================================================================================================================================
// ====================================================================================================================================================================================================================
// ====================================================================================================================================================================================================================
// ====================================================================================================================================================================================================================

void loop() {
	gps();
}

