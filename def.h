//
//  def.h
//  Tastenblinky
//
//  Created by Ruedi Heimlicher on 01.11.2013.
//
//

#include <stdio.h>

#define CLOCK_DIV 110 // timer0 1 Hz

#define TASTE1		19
#define TASTE2		29
#define TASTE3		44
#define TASTE4		67
#define TASTE5		94
#define TASTE6		122
#define TASTE7		155
#define TASTE8		186
#define TASTE9		212
#define TASTE_L	234
#define TASTE0		248
#define TASTE_R	255


#define TASTATURPORT PORTC
#define TASTATURPIN		3

#define MANUELL_PORT		PORTD
#define MANUELL_DDR		DDRD
#define MANUELL_PIN		PIND

#define MANUELL			7	// Bit 7 von Status
#define MANUELLPIN		3	// Pin 6 von PORT D fuer Anzeige Manuell
//#define MANUELLNEU		7	// Pin 7 von Status. Gesetzt wenn neue Schalterposition eingestellt
#define MANUELLTIMEOUT	100 // Loopled-counts bis Manuell zurueckgesetzt wird. 02FF: ca. 100 s
#define MINWAIT         3 // Anzahl loops von loopcount1 bis einschalten

// ADC
#define ADC_PORT            PORTC   //    PORTF
#define ADC_DDR             DDRC    //    DDRF
#define ADC_AKKUPIN         0


#define LOOPLED_PORT	PORTD
#define LOOPLED_DDR	DDRD
#define LOOPLED_PIN	4


#define DOGM_PORT	PORTD
#define DOGM_DDR	DDRD

#define DOGM_MOSI_PIN	0
#define DOGM_SCL_PIN	1
#define DOGM_CS_PIN   2
#define DOGM_CMD_PIN	3

#define MOTOR_ON     1




#define OSZIPORT           PORTC
#define OSZIPORTDDR        DDRC
#define OSZI_PULS_A        0
#define OSZI_PULS_B        1


#define OSZI_A_LO OSZIPORT &= ~(1<<OSZI_PULS_A)
#define OSZI_A_HI OSZIPORT |= (1<<OSZI_PULS_A)
#define OSZI_A_TOGG OSZIPORT ^= (1<<OSZI_PULS_A)


#define OSZI_B_LO OSZIPORT &= ~(1<<OSZI_PULS_B)
#define OSZI_B_HI OSZIPORT |= (1<<OSZI_PULS_B)
#define OSZI_B_TOGG OSZIPORT ^= (1<<OSZI_PULS_B)


