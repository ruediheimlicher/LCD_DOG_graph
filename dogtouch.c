//
//  dogl.c
//  DOG_LCD
//
//  Created by Ruedi Heimlicher on 15.11.2013.
//
//

#include <stdio.h>








#define TOUCH_TOP_RIGHT_PORT            PORTB
#define TOUCH_TOP_RIGHT_DDR             DDRB

#define TOUCH_TOP_PIN             0
#define TOUCH_RIGHT_PIN           1

#define TOUCH_BOT_LEFT_PORT            PORTC
#define TOUCH_BOT_LEFT_DDR             DDRC

#define TOUCH_BOT_PIN             4
#define TOUCH_LEFT_PIN             5


volatile uint16_t touchx=0;
volatile uint16_t touchy=0;


uint16_t read_x(void)
{
	TOUCH_TOP_RIGHT_DDR |= (1<<TOUCH_RIGHT_PIN); // RIGHT Ausgang
   TOUCH_TOP_RIGHT_PORT |= (1<<TOUCH_RIGHT_PIN); // HI
   
   TOUCH_BOT_LEFT_DDR |= (1<<TOUCH_LEFT_PIN); // LEFT Ausgang
   TOUCH_BOT_LEFT_PORT &= ~(1<<TOUCH_LEFT_PIN); // LO
   
   TOUCH_BOT_LEFT_DDR &= ~(1<<TOUCH_BOT_PIN); // BOT Eingang
   
   TOUCH_BOT_LEFT_DDR &= ~(1<<TOUCH_TOP_PIN); // TOP float
	
	_delay_ms(1);
	
	ADMUX = (1 << MUX2);//ADC4
	ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<<ADPS2)|(1<<ADPS1);
	
	while(ADCSRA & (1 << ADSC));
	//l = ADCL;
	//h = ADCH & 0x03;
	//h = h << 8;
	//h = h + l;
   TOUCH_TOP_RIGHT_PORT &= ~(1<<TOUCH_RIGHT_PIN); // RIGHT > LO

   return  ADCL + ((ADCH & 0x03)<<8);

   //touchx = ADCL + ((ADCH & 0x03)<<8);
}

uint16_t read_y(void)
{
	TOUCH_TOP_RIGHT_DDR |= (1<<TOUCH_TOP_PIN); // TOP Ausgang
   TOUCH_TOP_RIGHT_PORT |= (1<<TOUCH_TOP_PIN); // HI
   
   TOUCH_BOT_LEFT_DDR |= (1<<TOUCH_BOT_PIN); // BOT Ausgang
   TOUCH_BOT_LEFT_PORT &= ~(1<<TOUCH_BOT_PIN); // LO
   
   TOUCH_BOT_LEFT_DDR &= ~(1<<TOUCH_LEFT_PIN); // LEFT Eingang
   TOUCH_BOT_LEFT_PORT &= ~(1<<TOUCH_LEFT_PIN); // LEFT LO
  
   TOUCH_BOT_LEFT_DDR &= ~(1<<TOUCH_RIGHT_PIN); // RIGHT float
	
	_delay_ms(1);
	
	ADMUX = (1 << MUX0)|(1 << MUX2);//ADC5
	ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<<ADPS2)|(1<<ADPS1);
	
	while(ADCSRA & (1 << ADSC));
	//l1 = ADCL;
	//h1 = ADCH & 0x03;
	//h1 = h1 << 8;
	//h1 = h1 + l1;
   TOUCH_TOP_RIGHT_PORT &= ~(1<<TOUCH_TOP_PIN); // TOP > LO

   return ADCL + ((ADCH & 0x03)<<8);
   //touchy = ADCL + ((ADCH & 0x03)<<8);
}
