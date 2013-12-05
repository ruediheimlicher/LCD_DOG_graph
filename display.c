/*----------------------------------------------------------------------------
 Copyright:
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        21.11.2009
 Description:    Description:    EA DOG (M/L)128-6
------------------------------------------------------------------------------*/

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <avr/io.h>
#include <avr/delay.h>
#include "display.h"
#include "font.h"
#include "text.h"

extern volatile uint16_t      laufsekunde;
extern volatile uint8_t       curr_screen;
extern volatile uint8_t       curr_page; // aktuelle page
extern volatile uint8_t       curr_col; // aktuelle colonne
extern volatile uint8_t       curr_model; // aktuelles modell
extern volatile uint8_t       curr_kanal; // aktueller kanal

extern volatile uint8_t       curr_cursorzeile; // aktuelle zeile des cursors
extern volatile uint8_t       curr_cursorspalte; // aktuelle colonne des cursors

extern volatile uint8_t       last_cursorzeile; // letzte zeile des cursors
extern volatile uint8_t       last_cursorspalte; // letzte colonne des cursors



extern volatile uint16_t motorsekunde;
extern volatile uint16_t stopsekunde;
extern volatile uint16_t batteriespannung;

extern volatile uint16_t  posregister[4][8]; // Aktueller screen: werte fuer page und daraufliegende col fuer Menueintraege (hex). geladen aus progmem

extern volatile uint16_t  cursorpos[8][4]; // Aktueller screen: werte fuer page und daraufliegende col fuer cursor (hex). geladen aus progmem

// 
extern volatile uint16_t              updatecounter; // Zaehler fuer Einschalten
/*
 write_dogm(0x40,A0);// Display start line set --> 0
 write_dogm(0xA1,A0);  // ADC set --> reverse
 write_dogm(0xC0,A0);  // Common output mode select --> normal
 write_dogm(0xA6,A0);  // Display --> normal
 write_dogm(0xA2,A0);  // LCD Bias set --> 1/9 (Duty 1/65)
 write_dogm(0x2F,A0);  // Power control set --> Booster, Regulator and Follower on
 write_dogm(0xF8,A0);  // Booster ratio set --> Set internal Booster to 4x
 write_dogm(0x00,A0);  // ...
 write_dogm(0x27,A0);  // Contrast set
 write_dogm(0x81,A0);  // ...
 write_dogm(actContrast,A0);
 write_dogm(0xAC,A0);  // Static indicator set --> no indicator
 write_dogm(0x00,A0);  // ...
 write_dogm(0xAF,A0);  // Display on/off

 */



const volatile char DISPLAY_INIT[] =
{
0x40,// Display start line set --> 0
0xA1, // ADC set --> reverse Horizontal gespiegelt wenn A1
0xC0, // Common output mode select --> normal
0xA6, // Display --> normal  A7 ist reverse
0xA2, // LCD Bias set --> 1/9 (Duty 1/65)
0x2F, // Power control set --> Booster, Regulator and Follower on
0xF8, // Booster ratio set --> Set internal Booster to 4x
0x00, // ...
0x27, //
0x81, // Contrast set
0x18,
0xAC, // Static indicator set --> no indicator
0x00, // ...
0xAF
};  // Display on/off

//const char DISPLAY_INIT[] = {0xC8,0xA1,0xA6,0xA2,0x2F,0x26,0x81,0x25,0xAC,0x00,0xAF};

//char_height_mul 1, 2 or 4
volatile unsigned char char_x=0,char_y=1,char_height_mul=1,char_width_mul=1;



void sethomescreen(void)
{
   posregister[3][0] = 56 | (0x02 << 8); // Einschaltzeit

   posregister[3][1] = 56 | (0x02 << 8); // Einschaltzeit
   
   posregister[3][2] = 56 | (0x03 << 8); // Stoppuhr
   posregister[3][3] = 56 | (0x04 << 8); // Motorzeit
   posregister[3][4] = 44 | (0x07 << 8); // Batt
   posregister[3][5] = 74 | (0x06 << 8); // Spannungsanzeige
  
   // positionen lesen
   // titel setzen
   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[0])));
   char_x=0;
   char_y = 1;
   char_height_mul = 1;
   char_width_mul = 1;
   display_inverse(1);
   //display_write_prop_str(char_y,char_x,0,(unsigned char*)titelbuffer);
   display_write_inv_str(titelbuffer);
   display_inverse(0);
   char_height_mul = 1;
   char_width_mul = 1;
   // Einschaltzeit
   
   char_y= (posregister[3][0] & 0xFF00)>>8;

  // char_y = 4;
   char_x=0;
   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[1])));
   //display_write_str(titelbuffer);
   char_x = posregister[3][0] & 0x00FF;
   //display_write_min_sek(laufsekunde);

   char_x = posregister[3][1] & 0x00FF;
   char_y= (posregister[3][1] & 0xFF00)>>8;

   

   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[2])));
   char_x=0;
   char_y= (posregister[3][2] & 0xFF00)>>8;
   display_write_str(titelbuffer);
   char_x = posregister[3][2] & 0x00FF;

   
   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[3]))); // Motorzeit
   char_x=0;
   char_y= ((posregister[3][3] & 0xFF00)>>8);
   char_height_mul = 2;
   display_write_str(titelbuffer);
   
   char_y= ((posregister[3][3] & 0xFF00)>>8);
   char_height_mul = 2;
   char_width_mul = 2;
   
   char_x = posregister[3][3] & 0x00FF;
   display_write_min_sek(motorsekunde);
   
   char_height_mul = 1;
   char_width_mul = 1;

   // Batteriespannung

   char_y= (posregister[3][4] & 0xFF00)>>8;
   char_x = posregister[3][4] & 0x00FF;
   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[6]))); // Akku
   char_height_mul = 2;
   display_write_str(titelbuffer);
   
   char_height_mul = 1;
   char_width_mul = 1;

  // display_write_char(' ');
 
   char_x=0;
   char_y = curr_page;
   display_write_symbol(pfeilvollrechts);
   char_x += 2;
   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[4])));
   display_write_str(titelbuffer);
}// sethomescreen

void setmenuscreen(void)
{
   posregister[0][0] =  cursortab[0] |    (0x03 << 8); // modellcursor
   posregister[0][1] =  itemtab[0] |    (0x03 << 8); // Modelltext
   posregister[0][2] =  itemtab[1] |    (0x03 << 8); // Modellnummer
   posregister[0][3] =  itemtab[2] |    (0x03 << 8); // Modellname

   
   posregister[1][0] =  cursortab[0] |    (0x05 << 8); // kanalcursor
   posregister[1][1] =  itemtab[0] |    (0x05 << 8); // Kanaltext
   posregister[1][2] =  itemtab[1] |    (0x05 << 8); // kanalnummer
   
   
   posregister[2][0] =  cursortab[0] |    (0x06 << 8); // mixcursor
   posregister[2][1] =  itemtab[0] |    (0x06 << 8); // mixtext
   posregister[2][2] =  itemtab[1] |    (0x06 << 8); // mixnummer
   
   
   
   cursorpos[0][0] =posregister[0][0]; // cursorpos fuer model zeile/colonne
   cursorpos[1][0] =posregister[1][0]; // cursorpos fuer kanal
   cursorpos[2][0] =posregister[2][0]; // cursorpos fuer mix
  
   
   strcpy_P(menubuffer, (PGM_P)pgm_read_word(&(SettingTable[0])));
   char_x=0;
   char_y = 1;
   char_height_mul = 1;
   char_width_mul = 1;
   //display_write_prop_str(char_y,char_x,0,(unsigned char*)titelbuffer);
   display_write_inv_str(menubuffer);
   char_height_mul = 1;
   char_width_mul = 1;
   
   // Modell-Zeile
   strcpy_P(menubuffer, (PGM_P)pgm_read_word(&(SettingTable[1])));
   char_y= (posregister[0][1] & 0xFF00)>>8;
   char_x = posregister[0][1] & 0x00FF;
   //char_x=0;
   display_write_str(menubuffer);
   
   char_y= (posregister[0][2] & 0xFF00)>>8;
   char_x = posregister[0][2] & 0x00FF;
   display_write_int(curr_model);

   strcpy_P(menubuffer, (PGM_P)pgm_read_word(&(ModelTable[curr_model])));
   char_y= (posregister[0][3] & 0xFF00)>>8;
   char_x = posregister[0][3] & 0x00FF;
   display_write_prop_str(char_y,char_x,0,menubuffer);
   
 
   char_y= (posregister[0][0] & 0xFF00)>>8;
   char_x = posregister[0][0] & 0x00FF;
   display_write_symbol(pfeilvollrechts);

   // Kanal-Zeile
   strcpy_P(menubuffer, (PGM_P)pgm_read_word(&(SettingTable[2])));
   char_y= (posregister[1][1] & 0xFF00)>>8;
   char_x = posregister[1][1] & 0x00FF;
   //char_x=0;
   display_write_str(menubuffer);
   
   char_y= (posregister[1][2] & 0xFF00)>>8;
   char_x = posregister[1][2] & 0x00FF;
   display_write_int(curr_kanal);

   
   // Mix-Zeile
   strcpy_P(menubuffer, (PGM_P)pgm_read_word(&(SettingTable[3])));
   char_y= (posregister[2][1] & 0xFF00)>>8;
   char_x = posregister[2][1] & 0x00FF;
   //char_x=0;
   display_write_str(menubuffer);
   
   char_y= (posregister[2][2] & 0xFF00)>>8;
   char_x = posregister[2][2] & 0x00FF;
   display_write_int(curr_kanal);

   char_y= (posregister[1][0] & 0xFF00)>>8;
   char_x = posregister[1][0] & 0x00FF;
   /*
   if (laufsekunde%2)
   {
   display_write_symbol(pfeilvollrechts);
   }
   else
   {
      display_write_symbol(pfeilleerrechts);
   }
   */
   


}// setmenuscreen





uint8_t update_screen(void)
{
   switch (curr_screen)
   {
      case 0: // homescreen
      {
         updatecounter++;
         char_x = posregister[3][0] & 0x00FF;
         char_y= (posregister[3][0] & 0xFF00)>>8;
         //display_go_to(char_x,char_y);
         char_height_mul = 1;
         char_width_mul = 1;
         char_x=90;
         char_y = 1;

         display_write_min_sek(laufsekunde);
 
         // Stopzeit aktualisieren
         char_height_mul = 1;
         char_width_mul = 1;
         
         char_y= (posregister[3][3] & 0xFF00)>>8;
         char_x = posregister[3][3] & 0x00FF;
         display_write_min_sek(motorsekunde);

         
         // Motorzeit aktualisieren
         char_height_mul = 2;
         char_width_mul = 2;

         char_y= (posregister[3][3] & 0xFF00)>>8;
         char_x = posregister[3][3] & 0x00FF;
         display_write_min_sek(motorsekunde);
         
         // Batteriespannung aktualisieren
         char_y= (posregister[3][4] & 0xFF00)>>8;
         char_x = posregister[3][5] & 0x00FF;
         display_write_spannungbis10(batteriespannung);

         
         char_height_mul = 1;
         char_width_mul = 1;

      }break;
         
      case 1: // Menu
      {
         char_x=90;
         char_y = 1;
         display_write_min_sek(laufsekunde);

         char_y= (posregister[0][2] & 0xFF00)>>8;
         char_x = posregister[0][2] & 0x00FF;
         display_write_int(curr_model);
         
         if (!((curr_cursorzeile == last_cursorzeile) && (curr_cursorspalte == last_cursorspalte))) // cursorzeile verschoben
         {
            uint16_t lastcursorposition = cursorpos[last_cursorzeile][last_cursorspalte];
            
            // letzte Cursorposition entfernen
            char_y= (lastcursorposition & 0xFF00)>>8;
            char_x = lastcursorposition & 0x00FF;
            display_write_symbol(pfeilwegrechts);
            // last updaten
            last_cursorzeile = curr_cursorzeile;
            last_cursorspalte = curr_cursorspalte;
         }
         
         uint16_t cursorposition = cursorpos[curr_cursorzeile][curr_cursorspalte];

         char_y= (cursorposition & 0xFF00)>>8;
         char_x = cursorposition & 0x00FF;
         
         display_write_symbol(pfeilvollrechts);
         
         /*
         if (laufsekunde%2)
         {
            display_write_symbol(pfeilvollrechts);
         }
         else
         {
            display_write_symbol(pfeilwegrechts);
         }
          */

      }break;
   }
   return 0;
}

//##############################################################################################
//Writes one byte to data or cmd register
//
//##############################################################################################
void display_back_char (void)
{
	char_x = char_x - (FONT_WIDTH*char_width_mul);
	if (char_x > 128) char_x = 0;
}

//##############################################################################################
//Writes one byte to data or cmd register
//
//##############################################################################################
void display_write_byte(unsigned cmd_data, unsigned char data) 
{
   if(cmd_data == 0)
	{
		A0_HI;
	}
	else
	{
		A0_LO;
	}
   
   
   spi_out(data);
  
   /*
	DOG_PORT &= ~(1<<SPI_SS);
	if(cmd_data == 0)
	{
		PORT_A0 |= (1<<PIN_A0);
	}
	else
	{
		PORT_A0 &= ~(1<<PIN_A0);
	}
   
//   
   
	SPDR = data;
	while(!(SPSR & (1<<SPIF)));
   
   _delay_us(1);
	PORTB |= (1<<SPI_SS);
    */
}

//##############################################################################################
//Init LC-Display
//
//##############################################################################################
void display_init() 
{
   
	//Set TIMER0 (PWM OC2 Pin)
	BRIGHTNESS_PWM_DDR |= (1<<BRIGHTNESS_PWM_PIN);//PWM PORT auf Ausgang (OC2)
	TCCR2A |= (1<<WGM21|1<<WGM20|1<<COM2A1|1<<CS20);
	OCR2A = 50;
	
	//set outputs AO und RESET
	DDR_A0  |= (1<<PIN_A0);             
	DDR_RST |= (1<<PIN_RST);
  
	//Set SPI PORT
	DDRB |= (1<<SPI_Clock)|(1<<SPI_DO)|(1<<SPI_SS);
	PORTB |= (1<<SPI_SS);
	//Enable SPI, SPI in Master Mode
	SPCR = (1<<SPE)|(1<<MSTR); 	
  
    //Reset the Display Controller
	PORT_RST &= ~(1<<PIN_RST);                   
	PORT_RST |= (1<<PIN_RST);
	//asm("nop");

   
   
	_delay_us(10);
	//send 11 init commands to Display
   
	for (unsigned char tmp = 0;tmp < 14;tmp++)
	{
		display_write_byte(CMD,DISPLAY_INIT[tmp]);
      _delay_us(10);
	}
	display_clear();
	
	return;
}

void display_soft_init(void)
{
	//Set TIMER0 (PWM OC2 Pin)
	BRIGHTNESS_PWM_DDR |= (1<<BRIGHTNESS_PWM_PIN);//PWM PORT auf Ausgang (OC2)
	TCCR2A |= (1<<WGM21|1<<WGM20|1<<COM2A1|1<<CS20);
	OCR2A = 50;
	
   SOFT_SPI_DDR |= (1<<DOG_A0);
   SOFT_SPI_PORT |= (1<<DOG_A0);
   
   SOFT_SPI_DDR |= (1<<DOG_RST);
   SOFT_SPI_PORT |= (1<<DOG_RST);
   
   SOFT_SPI_DDR |= (1<<DOG_CS);
   SOFT_SPI_PORT |= (1<<DOG_CS);
   
   SOFT_SPI_DDR |= (1<<DOG_SCL);
   SOFT_SPI_PORT &= ~(1<<DOG_SCL);
   
   SOFT_SPI_DDR |= (1<<DOG_DATA);
   SOFT_SPI_PORT &= ~(1<<DOG_DATA);
   
   //Reset the Display Controller
	SOFT_SPI_DDR &= ~(1<<PIN_RST);
	SOFT_SPI_DDR |= (1<<PIN_RST);

	_delay_us(1);
	//send 11 init commands to Display
   
	for (unsigned char tmp = 0;tmp < 14;tmp++)
	{
		display_write_byte(CMD,DISPLAY_INIT[tmp]);
      _delay_us(1);
	}
	display_clear();
	
	return;
}

//##############################################################################################
//Go to x,y
//
//##############################################################################################

void display_go_to (unsigned char x, unsigned char y)
{
   
	display_write_byte(CMD,DISPLAY_PAGE_ADDRESS | ((y) & 0x0F));
   
	display_write_byte(CMD,DISPLAY_COL_ADDRESS_MSB | ((x>>4) & 0x0F));
  
	display_write_byte(CMD,DISPLAY_COL_ADDRESS_LSB | ((x) & 0x0F));
	return;
}



//##############################################################################################
//Diplay clear
//
//##############################################################################################
void display_clear()
{
	unsigned char page, col;
	
	for(page=0;page<8;page++)
	{
		display_go_to(0,page);
	
		for (col=0;col<128;col++)
		{
         if (col%4)
         {
			display_write_byte(DATA,0x00);
         }
         else
         {
            display_write_byte(0,0x00);
         }
		}
	}
}


//##############################################################################################
//Diplay clear
//
//##############################################################################################
void display_inverse(uint8_t inv)
{
   display_write_byte(1,0xA6+(inv & 0x01));
}

//##############################################################################################
//Diplay Memory (BMP MONO 64x128)
//
//##############################################################################################
void display_mem(PGM_P pointer)
{
	unsigned char col,page;
	
	for(col=DISPLAY_OFFSET;col < 128 + DISPLAY_OFFSET;col++) 
	{
		for (page=8;page!=0;page--)
		{
			display_go_to(col,page-1);
				
			display_write_byte(0,~(pgm_read_byte(pointer++)));
		}
	}
}

//##############################################################################################
//Ausgabe eines Zeichens
//
//##############################################################################################
void display_write_char(unsigned char c)
{
	unsigned char col,page,tmp1,tmp2,tmp3,tmp4,counter;
	PGM_P pointer = font[c];
	
	
	for(col=(char_x+DISPLAY_OFFSET);col<(char_x+(FONT_WIDTH*char_width_mul)+DISPLAY_OFFSET);col=col+char_width_mul)
	{
		for (page=char_y;page<(char_y+((FONT_HEIGHT/8)*char_height_mul));page = page +char_height_mul)
		{
			tmp1 = pgm_read_byte(pointer++);
			
			if (char_height_mul > 1) // schreiben auf mehrere pages
			{
				tmp2 = (tmp1&0xf0)>>4; // HI byte
				tmp1 = tmp1 & 0x0f;     // LO byte
				
				tmp1 = ((tmp1&0x01)*3)+(((tmp1&0x02)<<1)*3)+(((tmp1&0x04)<<2)*3)+(((tmp1&0x08)<<3)*3);
				tmp2 = ((tmp2&0x01)*3)+(((tmp2&0x02)<<1)*3)+(((tmp2&0x04)<<2)*3)+(((tmp2&0x08)<<3)*3);
				
				if (char_height_mul>2)
				{
					tmp3 = tmp2;
					tmp2 = (tmp1&0xf0)>>4;
					tmp1 = tmp1 & 0x0f;
				
					tmp1 = ((tmp1&0x01)*3)+(((tmp1&0x02)<<1)*3)+(((tmp1&0x04)<<2)*3)+(((tmp1&0x08)<<3)*3);
					tmp2 = ((tmp2&0x01)*3)+(((tmp2&0x02)<<1)*3)+(((tmp2&0x04)<<2)*3)+(((tmp2&0x08)<<3)*3);
					
				
					tmp4 = (tmp3&0xf0)>>4;
					tmp3 = tmp3 & 0x0f;
				
					tmp3 = ((tmp3&0x01)*3)+(((tmp3&0x02)<<1)*3)+(((tmp3&0x04)<<2)*3)+(((tmp3&0x08)<<3)*3);
					tmp4 = ((tmp4&0x01)*3)+(((tmp4&0x02)<<1)*3)+(((tmp4&0x04)<<2)*3)+(((tmp4&0x08)<<3)*3);
					
					display_go_to(col,page+1);
					for(counter = 0;counter<char_width_mul;counter++)
					{
						display_write_byte(0,tmp3);
					}
					
					display_go_to(col,page+2);
					for(counter = 0;counter<char_width_mul;counter++)
					{
						display_write_byte(0,tmp4);
					}
				} // end >2

			
				display_go_to(col,page);
				
				for(counter = 0;counter<char_width_mul;counter++)
				{
					display_write_byte(DATA,tmp2);
				}
			}
			
			display_go_to(col,page-1);
			for(counter = 0;counter<char_width_mul;counter++)
			{	
				display_write_byte(DATA,tmp1);
			}
		}
	}
	
	if (char_x < (128 + DISPLAY_OFFSET))
	{
		char_x = char_x + (FONT_WIDTH*char_width_mul);
	}
	return;
}

// inv
void  display_write_inv_char(unsigned char c)
{
	unsigned char col,page,tmp1,tmp2,tmp3,tmp4,counter;
	PGM_P pointer = font[c];
	
	
	for(col=(char_x+DISPLAY_OFFSET);col<(char_x+(FONT_WIDTH*char_width_mul)+DISPLAY_OFFSET);col=col+char_width_mul)
	{
		for (page=char_y;page<(char_y+((FONT_HEIGHT/8)*char_height_mul));page = page +char_height_mul)
		{
			tmp1 = pgm_read_byte(pointer++);
			
			if (char_height_mul > 1) // schreiben auf mehrere pages
			{
				tmp2 = (tmp1&0xf0)>>4;
				tmp1 = tmp1 & 0x0f;
				
				tmp1 = ((tmp1&0x01)*3)+(((tmp1&0x02)<<1)*3)+(((tmp1&0x04)<<2)*3)+(((tmp1&0x08)<<3)*3);
				tmp2 = ((tmp2&0x01)*3)+(((tmp2&0x02)<<1)*3)+(((tmp2&0x04)<<2)*3)+(((tmp2&0x08)<<3)*3);
				
				if (char_height_mul>2)
				{
					tmp3 = tmp2;
					tmp2 = (tmp1&0xf0)>>4;
					tmp1 = tmp1 & 0x0f;
               
					tmp1 = ((tmp1&0x01)*3)+(((tmp1&0x02)<<1)*3)+(((tmp1&0x04)<<2)*3)+(((tmp1&0x08)<<3)*3);
					tmp2 = ((tmp2&0x01)*3)+(((tmp2&0x02)<<1)*3)+(((tmp2&0x04)<<2)*3)+(((tmp2&0x08)<<3)*3);
					
               
					tmp4 = (tmp3&0xf0)>>4;
					tmp3 = tmp3 & 0x0f;
               
					tmp3 = ((tmp3&0x01)*3)+(((tmp3&0x02)<<1)*3)+(((tmp3&0x04)<<2)*3)+(((tmp3&0x08)<<3)*3);
					tmp4 = ((tmp4&0x01)*3)+(((tmp4&0x02)<<1)*3)+(((tmp4&0x04)<<2)*3)+(((tmp4&0x08)<<3)*3);
					
					display_go_to(col,page+1);
					for(counter = 0;counter<char_width_mul;counter++)
					{
						display_write_byte(0,~tmp3);
					}
					
					display_go_to(col,page+2);
					for(counter = 0;counter<char_width_mul;counter++)
					{
						display_write_byte(0,~tmp4);
					}
				}
            
            
				display_go_to(col,page);
				
				for(counter = 0;counter<char_width_mul;counter++)
				{
					display_write_byte(DATA,~tmp2);
				}
			}
			
			display_go_to(col,page-1);
			for(counter = 0;counter<char_width_mul;counter++)
			{
				display_write_byte(DATA,~tmp1);
			}
		}
	}
	
	if (char_x < (128 + DISPLAY_OFFSET))
	{
		char_x = char_x + (FONT_WIDTH*char_width_mul);
	}
	return;
}



//##############################################################################################
// Ausgabe simple char

void display_write_simplechar(unsigned char c)
{
	unsigned char col,tmp1,page;
	PGM_P pointer = font[c];
	
	
	for(col=(char_x+DISPLAY_OFFSET);col<(char_x+(FONT_WIDTH)+DISPLAY_OFFSET);col=col+1)
	{
		{
			tmp1 = pgm_read_byte(pointer++);
			
			display_go_to(col,char_y);
         display_write_byte(DATA,tmp1);
		}
	}
	
	if (char_x < (128 + DISPLAY_OFFSET))
	{
		char_x = char_x + (FONT_WIDTH);
	}
	return;
}

//##############################################################################################
//Ausgabe eines Zeichenkette
//
//##############################################################################################
void display_write_P (const char *Buffer,...)
{
	va_list ap;
	va_start (ap, Buffer);	
	
	int format_flag;
	char str_buffer[10];
	char str_null_buffer[10];
	char move = 0;
	char Base = 0;
	int tmp = 0;
	char by;
	char *ptr;
		
	//Ausgabe der Zeichen
    for(;;)
    {
       by = pgm_read_byte(Buffer++);
       if(by==0) break; // end of format string
       
       if (by == '%')
       {
          by = pgm_read_byte(Buffer++);
          if (isdigit(by)>0)
          {
             
             str_null_buffer[0] = by;
             str_null_buffer[1] = '\0';
             move = atoi(str_null_buffer);
             by = pgm_read_byte(Buffer++);
          }
          
          switch (by)
          {
             case 's':
                ptr = va_arg(ap,char *);
                while(*ptr) { display_write_char(*ptr++); }
                break;
             case 'b':
                Base = 2;
                goto ConversionLoop;
             case 'c':
                //Int to char
                format_flag = va_arg(ap,int);
                display_write_char (format_flag++);
                break;
             case 'i':
                Base = 10;
                goto ConversionLoop;
             case 'o':
                Base = 8;
                goto ConversionLoop;
             case 'x':
                Base = 16;
                //****************************
             ConversionLoop:
                //****************************
                itoa(va_arg(ap,int),str_buffer,Base);
                int b=0;
                while (str_buffer[b++] != 0){};
                b--;
                if (b<move)
                {
                   move -=b;
                   for (tmp = 0;tmp<move;tmp++)
                   {
                      str_null_buffer[tmp] = '0';
                   }
                   //tmp ++;
                   str_null_buffer[tmp] = '\0';
                   strcat(str_null_buffer,str_buffer);
                   strcpy(str_buffer,str_null_buffer);
                }
                display_write_str (str_buffer);
                move =0;
                break;
          }
          
       }	
       else
       {
          display_write_char ( by );	
       }
    }
	va_end(ap);
}

//##############################################################################################
//Ausgabe eines Strings
//
//##############################################################################################
void display_write_str(char *str)
{
	while (*str)
	{
		display_write_char(*str++);
	}
}

void display_write_inv_str(char *str)
{
	while (*str)
	{
		display_write_inv_char(*str++);
	}
}

//##############################################################################################
//Ausgabe einer Zahl
//
//##############################################################################################
void display_write_int(uint8_t zahl)
{
   char zahlbuffer[10];
   itoa(zahl,zahlbuffer,10);
   
	//while (*zahlbuffer)
	{
		display_write_str(zahlbuffer);
      
	}
}

//##############################################################################################
//Ausgabe Spannung
//
//##############################################################################################
void display_write_spannungbis10(uint8_t rawspannung) // eine Dezimale
{
   uint16_t temp = rawspannung*5;
   uint8_t dezimale = temp%10;
   uint8_t wert = temp/10;
   char tempbuffer[6]={};
  
   {
     // tempbuffer[0] =' ';
   }
   tempbuffer[0] =wert%10+'0';
   tempbuffer[1] ='.';
   
   tempbuffer[2] =dezimale+'0';
   tempbuffer[3] ='V';
   tempbuffer[4] = '\0';
   display_write_str(tempbuffer);

   
}


//##############################################################################################
//Ausgabe Minute:Sekunde
//
//##############################################################################################
void display_write_min_sek(uint16_t rawsekunde)
{
   uint8_t minute = rawsekunde/60;
   uint8_t sekunde = rawsekunde%60;
   uint8_t stunde = rawsekunde/3600;
   char tempbuffer[6]={};
   if (stunde)
   {
      char stdbuffer[4]={};
      stdbuffer[0] =stunde/10+'0';
      stdbuffer[1] =stunde%10+'0';
      stdbuffer[2] =':';
      stdbuffer[3] = '\0';
      display_write_str(stdbuffer);
   }
   tempbuffer[0] =minute/10+'0';
   tempbuffer[1] =minute%10+'0';
   tempbuffer[2] =':';
   tempbuffer[3] =sekunde/10+'0';
   tempbuffer[4] =sekunde%10+'0';
   tempbuffer[5] = '\0';
   display_write_str(tempbuffer);
   
}


//##############################################################################################
// Ausgabe Pfeil rechts an pos x,y (page)
//##############################################################################################
void display_pfeilvollrechts(uint8_t col, uint8_t page)
{
   char_x = col;
   char_y = page;
   display_write_symbol(pfeilvollrechts);
   
}

//##############################################################################################
//Ausgabe eines Symbols
//
//##############################################################################################
void display_write_symbol(PGM_P symbol)
{
	unsigned char col,page,tmp1,tmp2,tmp3,tmp4,counter;
	PGM_P pointer = symbol;
	
	
	for(col=(char_x+DISPLAY_OFFSET);col<(char_x+(FONT_WIDTH*char_width_mul)+DISPLAY_OFFSET);col=col+char_width_mul)
	{
		for (page=char_y;page<(char_y+((FONT_HEIGHT/8)*char_height_mul));page = page +char_height_mul)
		{
			tmp1 = pgm_read_byte(pointer++);
			
			if (char_height_mul > 1)
			{
				tmp2 = (tmp1&0xf0)>>4;
				tmp1 = tmp1 & 0x0f;
				
				tmp1 = ((tmp1&0x01)*3)+(((tmp1&0x02)<<1)*3)+(((tmp1&0x04)<<2)*3)+(((tmp1&0x08)<<3)*3);
				tmp2 = ((tmp2&0x01)*3)+(((tmp2&0x02)<<1)*3)+(((tmp2&0x04)<<2)*3)+(((tmp2&0x08)<<3)*3);
				
				if (char_height_mul>2)
				{
					tmp3 = tmp2;
					tmp2 = (tmp1&0xf0)>>4;
					tmp1 = tmp1 & 0x0f;
               
					tmp1 = ((tmp1&0x01)*3)+(((tmp1&0x02)<<1)*3)+(((tmp1&0x04)<<2)*3)+(((tmp1&0x08)<<3)*3);
					tmp2 = ((tmp2&0x01)*3)+(((tmp2&0x02)<<1)*3)+(((tmp2&0x04)<<2)*3)+(((tmp2&0x08)<<3)*3);
					
               
					tmp4 = (tmp3&0xf0)>>4;
					tmp3 = tmp3 & 0x0f;
               
					tmp3 = ((tmp3&0x01)*3)+(((tmp3&0x02)<<1)*3)+(((tmp3&0x04)<<2)*3)+(((tmp3&0x08)<<3)*3);
					tmp4 = ((tmp4&0x01)*3)+(((tmp4&0x02)<<1)*3)+(((tmp4&0x04)<<2)*3)+(((tmp4&0x08)<<3)*3);
					
					display_go_to(col,page+1);
					for(counter = 0;counter<char_width_mul;counter++)
					{
						display_write_byte(0,tmp3);
					}
					
					display_go_to(col,page+2);
					for(counter = 0;counter<char_width_mul;counter++)
					{
						display_write_byte(0,tmp4);
					}
				}
            
            
				display_go_to(col,page);
				
				for(counter = 0;counter<char_width_mul;counter++)
				{
					display_write_byte(DATA,tmp2);
				}
			}
			
			display_go_to(col,page-1);
			for(counter = 0;counter<char_width_mul;counter++)
			{
				display_write_byte(DATA,tmp1);
			}
		}
	}
	
	if (char_x < (128 + DISPLAY_OFFSET))
	{
		char_x = char_x + (FONT_WIDTH*char_width_mul);
	}
	return;
}


/*
 http://www.mikrocontroller.net/attachment/5130/spi.c
 void write_spi (unsigned char data_out){       //msb first
 unsigned char loop, mask;
 for (loop=0,mask=0x80;loop<8;loop++, mask=mask>>1)
 {sclk=0;
 if (data_out & mask) mosi=1;
 else mosi=0;
 sclk=1;
 }
 sclk=0;
 }

 */


/*
 * File:        dogl.c
 * Project:     Mini Anzeige Modul
 * Author:      Nicolas Meyertˆns
 * Version:     siehe setup.h
 * Web:         www.PIC-Projekte.de

 * Diese Funktion gibt eine Zeichenkette auf dem Bildschirm aus.
 * Mit einer Angabe der Adresse, kann dies an jedem beliebigen Ort auf
 * dem Display stattfinden. Des Weiteren kann zwischen normaler und
 * invertierter Darstellung gew‰hlt werden.
 *
 * page:        Adresse - Zeile (0..7)
 * column:      Adresse - Spalte (0..127)
 * inverse:     Invertierte Darstellung wenn true sonst normal
 * *pChain:     Die Zeichnkette ansich, welche geschrieben werden soll
 */

/*
 * Auswahl einer Adresse. Diese Funktion wird in der Regel nicht vom
 * Anwender selbst aufgerufen, sondern nur durch die Funktionen dieser
 * C-Datei.
 *
 * page:    Adresse - Zeile (0..7)
 * column:  Adresse - Spalte (0..127)
 */
void setAddrDOGL(uint8_t page, uint8_t column)
{
   if( page<8 && column<128 )
   {
      
      display_write_byte(CMD,0xB0 + page);
      display_write_byte(CMD,0x10 + ((column&0xF0)>>4) );
      display_write_byte(CMD,0x00 +  (column&0x0F) );
      
   }
}

void display_write_prop_str(uint8_t page, uint8_t column, uint8_t inverse, const uint8_t *pChain)
{
   uint8_t l=0,k;
   setAddrDOGL(page-1,column);
   if(inverse)
   {
      display_write_byte(DATA,~propfont[0][1]);
      column++;
   }
   else
   {
      display_write_byte(DATA,propfont[0][1]);
      column++;
   }
   
   while(*pChain)
   {
      PGM_P pointer = propfont[*pChain-32];
      uint8_t  fontbreite =   pgm_read_byte(pointer++);
      unsigned char tmp1=0;
      for(k=1; k <= fontbreite; k++)
      {
         if( column > 127)
            break;
         
         display_go_to(column,page-1);
         tmp1 = pgm_read_byte(pointer++);
         column++;
         if(inverse)
         {
            display_write_byte(DATA,~tmp1);
         }
         else
         {
            display_write_byte(DATA,tmp1);
         }
      }
      if( column > 127)
      {
         return 0;
      }
      if(inverse)
      {
         display_write_byte(DATA,~propfont[0][1]);
         column++;
      }
      else
      {
         display_write_byte(DATA,propfont[0][1]);
         column++;
      }
      
      /*
       * Ab dem (126-32). Eintrag in der Font Library folgen die Zeichen
       * des MiniAnzeigeModuls. Diese Eintr‰ge kommen NICHT als String in
       * diese Funktion und haben somit keinen Terminator! Es muss beim
       * Zeichnen eines Zeichens also darauf geachtet werden, dass der
       * Pointer nicht einen Schritt weiter geht sondern direkt die
       * Schleife beendet. Das geschieht hier:
       */
      if( *pChain > 126 )
         break;
      pChain++;
   } // while *char
   
   return l;
}


//##############################################################################################

void r_uitoa8(int8_t zahl, char* string)
{
   uint8_t i;
   
   string[3]='\0';                  // String Terminator
   for(i=3; i>=0; i--)
   {
      string[i]=(zahl % 10) +'0';     // Modulo rechnen, dann den ASCII-Code von '0' addieren
      zahl /= 10;
   }
}
//##############################################################################################


uint8_t spi_out(uint8_t dataout)
{
   cli();
  // OSZI_B_LO;
   CS_LO; // Chip enable
   uint8_t datain=0xFF;
   uint8_t pos=0;
   SCL_LO; // SCL LO
   uint8_t tempdata=dataout;

   for (pos=8;pos>0;pos--)
   {
      
      if (tempdata & 0x80)
      {
         //DATA_HI;
         SOFT_SPI_PORT |= (1<<DOG_DATA);

      }
      else
      {
         //DATA_LO;
         SOFT_SPI_PORT &= ~(1<<DOG_DATA);
         //DOG_PORT |= (1<<DOG_DATA);
      }
      tempdata<<= 1;
     // _delay_us(10);
      //SCL_HI;
      SOFT_SPI_PORT |= (1<<DOG_SCL);
      //_delay_us(10);
      //SCL_LO;
      SOFT_SPI_PORT &= ~(1<<DOG_SCL);
      
   }
 //  OSZI_B_HI;
   
   CS_HI;// Chip disable
   
   sei();
   return datain;
}

