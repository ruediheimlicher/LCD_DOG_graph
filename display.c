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

void display_soft_init()
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
			display_write_byte(0,0x00);
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
//##############################################################################################


uint8_t spi_out(uint8_t dataout)
{
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
      _delay_us(10);
      //SCL_HI;
      SOFT_SPI_PORT |= (1<<DOG_SCL);
      _delay_us(10);
      //SCL_LO;
      SOFT_SPI_PORT &= ~(1<<DOG_SCL);
      
   }
 //  OSZI_B_HI;
   
   CS_HI;// Chip disable
   
   
   return datain;
}

