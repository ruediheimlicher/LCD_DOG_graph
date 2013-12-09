//
//  dog_spi.c
//  DOG_LCD
//
//  Created by Ruedi Heimlicher on 22.11.2013.
//
//

#include <stdio.h>
#include "def.h"
#include <avr/pgmspace.h>
#include "display.c"
#include "text.h"

volatile uint8_t screen_x=0;
volatile uint8_t screen_y=1;

void sethomescreen(void)
{
   // titel setzen
   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[0])));
   screen_x=0;
   screen_y = 1;
//   char_height_mul = 2;
//   char_width_mul = 2;
   //display_write_prop_str(char_y,char_x,0,(unsigned char*)titelbuffer);
   
   display_write_str(titelbuffer,1);

}