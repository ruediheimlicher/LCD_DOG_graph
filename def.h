//
//  def.h
//  Tastenblinky
//
//  Created by Ruedi Heimlicher on 01.11.2013.
//
//

#include <stdio.h>

#define TASTE1		19
#define TASTE2		29
#define TASTE3		44
#define TASTE4		67
#define TASTE5		94
#define TASTE6		122
#define TASTE7		155
#define TASTE8		186
#define TASTE9		212
#define TASTE_L	230
#define TASTE0		248
#define TASTE_R	255

#define TASTATURPORT PORTC
#define TASTATURPIN		3

#define MANUELL_PORT		PORTD
#define MANUELL_DDR		DDRD
#define MANUELL_PIN		PIND

#define DOGM_PORT	PORTD
#define DOGM_DDR	DDRD

#define DOGM_MOSI_PIN	0
#define DOGM_SCL_PIN	1
#define DOGM_CS_PIN   2
#define DOGM_CMD_PIN	3

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

