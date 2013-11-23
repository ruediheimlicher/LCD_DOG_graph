//
//  navigation_old.c
//  Tastenblinky
//
//  Created by Ruedi Heimlicher on 30.10.2013.
//
//

#include <stdio.h>
#include <avr/pgmspace.h>

//#include "menu.c"

//#include "dogm.c"
//extern void write_byte_lcd(byte x, byte y, byte z);
//extern volatile uint8_t  navigation_old[8][4][8];
//extern volatile uint8_t  navigation[8];
void initnav(void)
{
   uint8_t i=0, k=0, l=0;
   for (i=0;i<8;i++)
   {
      for (k=0;k<8;k++)
      {
         for (l=0;l<8;l++)
         {
           // cursor[i][k][l] = 0xFF;
         }

      }

   }
   
   // page 0 // fixpage
   //zeile 0: Titelzeile
   /* col
    Titel: 0
    laufminute: 12
    : 13
    laufsekunde:15
    */
   
 

}