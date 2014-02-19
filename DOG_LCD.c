//
//  Tastenblinky.c
//  Tastenblinky
//
//  Created by Sysadmin on 03.10.07.
//  Copyright Ruedi Heimlihcer 2007. All rights reserved.
//



#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <inttypes.h>
#include <avr/delay.h>
#include "lcd.c"
#include "adc.c"
#include "def.h"

//#include "screen.c"



#include "font.h"
#include "display.c"

#include "text.h"





uint16_t loopCount0=0;
volatile uint16_t loopcount1=0;
uint16_t loopCount2=0;


/*
#define TASTE1		19
#define TASTE2		29
#define TASTE3		44
#define TASTE4		67
#define TASTE5		94
#define TASTE6		122
#define TASTE7		155
#define TASTE8		186
#define TASTE9		205
#define TASTE_L		223
#define TASTE0		236
#define TASTE_R		248
*/

volatile uint8_t                 default_settingarray[8][2]=
{
   {0x12,0x21},
   {0x00,0x00},
   {0x00,0x00},
   {0x00,0x00},
   {0x00,0x00},
   {0x00,0x00},
   {0x00,0x00},
   {0x00,0x00}
};

volatile uint8_t                 default_levelarray[8]=
{
   0x12,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00,
   0x00
};


volatile uint8_t                 default_expoarray[8]=
{
   0x21,
   0x00,
   0x00,
   0x00,
   0x04, // Schieber
   0x04,
   0x08, // Schalter
   0x08
};

volatile uint8_t                 default_mixarray[8]=
{
   // index gerade  :  mixa(parallel) mit (0x70)<<4, mixb(opposite) mit 0x07
   // index ungerade: typ mit 0x03
   0x10,
   0x01,
   0x23,
   0x02,
   0x00,
   0x00,
   0x00,
   0x00
};

volatile uint8_t                 default_funktionarray[8]=
{
   //
   // bit 0-2: Steuerfunktion bit 4-6: Kanal von Steuerfunktion
   0x00,
   0x11,
   0x22,
   0x33,
   0x44,
   0x55,
   0x66,
   0x77
};

volatile uint8_t                 default_devicearray[8]=
{
   //
   // bit 0-2: Steuerfunktion bit 4-6: Kanal von Steuerfunktion
   0x00,
   0x01,
   0x02,
   0x03,
   0x04,
   0x05,
   0x06,
   0x07
};

volatile uint8_t                 default_ausgangarray[8]=
{
   //
   // bit 0-2: Kanal bit 4-6: 
   0x00,
   0x01,
   0x02,
   0x03,
   0x04,
   0x05,
   0x06,
   0x07
};





//volatile uint8_t             curr_settingarray[8][2]={}; // pro Kanal 3 bytes: level, expo, mix
volatile uint8_t              curr_levelarray[8];
volatile uint8_t              curr_expoarray[8];
volatile uint8_t              curr_mixarray[8];
volatile uint8_t              curr_funktionarray[8];
volatile uint8_t              curr_devicearray[8] = {};
volatile uint8_t              curr_ausgangarray[8];

volatile uint16_t                updatecounter; // Zaehler fuer Update des screens

volatile uint8_t                 curr_screen=0; // aktueller screen

volatile uint8_t                 curr_page=7; // aktuelle page
volatile uint8_t                 curr_col=0; // aktuelle colonne

volatile uint8_t                 curr_cursorzeile=0; // aktuelle zeile des cursors
volatile uint8_t                 curr_cursorspalte=0; // aktuelle colonne des cursors
volatile uint8_t                 last_cursorzeile=0; // letzte zeile des cursors
volatile uint8_t                 last_cursorspalte=0; // letzte colonne des cursors


volatile uint8_t                 curr_model=0; // aktuelles modell
volatile uint8_t                 curr_kanal=0; // aktueller kanal

volatile uint8_t                 curr_impuls=0; // aktueller Impuls im Ausgangscreen

volatile uint8_t                 curr_richtung=0; // aktuelle richtung


volatile uint8_t                 curr_setting=0; // aktuelles Setting fuer Modell

volatile uint16_t                posregister[8][8]={}; // Aktueller screen: werte fuer page und daraufliegende col fuer Menueintraege (hex). geladen aus progmem

volatile uint16_t                cursorpos[8][8]={}; // Aktueller screen: werte fuer page und darauf liegende col fuer den cursor




volatile uint8_t                 navcounter=0; // Zaehler fuer Einschalten
volatile uint16_t                navfix=0; // Stand des loopcounters fuer Einschalten


volatile uint8_t                 navscreen=0; // aktuelle seite
volatile uint8_t                 navcol=0; // aktuelle kolonne
volatile uint8_t                 navline=0; // aktuelle linie

volatile uint8_t                 blinkline=0xFF;
volatile uint8_t                 blinkcol=0xFF;
volatile char                    blinkzeichen = ' ';

volatile uint16_t                 blink_cursorpos=0xFFFF;


volatile uint8_t                 modelnummer=1;
volatile uint8_t                 Taste=0;

volatile uint8_t                 programmstatus=0x00;

volatile uint8_t                 senderstatus=0x00;

volatile uint8_t                 Tastenwert=0;
volatile uint16_t                TastaturCount=0;
volatile uint16_t                manuellcounter=0; // Countr fuer Timeout
volatile uint8_t                 startcounter=0; // timeout-counter beim Start von Settings, schneller als manuellcounter. Ermoeglicht Dreifachklick auf Taste 5
volatile uint8_t                 settingstartcounter=0; // Counter fuer Klicks auf Taste 5
volatile uint16_t                mscounter=0; // Counter fuer ms in timer-ISR



uint16_t TastenStatus=0;
uint16_t Tastencount=0;
uint16_t Tastenprellen=0x01F;

volatile uint8_t  Menu_Ebene=0;
volatile uint8_t  Kanal_Thema =0;

volatile uint8_t  Anzeigekanal =0;


volatile uint16_t laufsekunde=0;
volatile uint16_t laufminute=0;

volatile uint16_t motorsekunde=0;
volatile uint16_t stopsekunde=0;

volatile uint16_t batteriespannung =0;

volatile char homepage0  = 0;



volatile uint8_t levelwert=0x32;
volatile uint8_t levelb=0x12;

volatile uint8_t expowert=0;
volatile uint8_t expob=0;



uint8_t EEMEM titel[] = "RC Settings";

uint8_t EEMEM EEjahr = 13;


// Code 1_wire end

void delay_ms(unsigned int ms)
/* delay for a minimum of <ms> */
{
	// we use a calibrated macro. This is more
	// accurate and not so much compiler dependent
	// as self made code.
	while(ms){
		_delay_ms(0.96);
		ms--;
	}
}

#pragma mark TIMER2
void timer0(void)
{
   TIMSK0 |= (1 << TOIE0);    //Set the ISR COMPA vect
   
   TCCR0B |= (1 << CS02);//
  // TCCR0B |= (1 << CS00);

}


ISR (TIMER0_OVF_vect)
{
   
   mscounter++;
   
   if (mscounter > CLOCK_DIV) // 0.5s
   {
      programmstatus ^= (1<<MS_DIV);
      mscounter=0;
      
      if (programmstatus & (1<<SETTINGWAIT))
      {
         startcounter++;
      }
      
      if (programmstatus & (1<<MS_DIV))
      {
         
         laufsekunde++;
         manuellcounter++;
         
         if (senderstatus & (1<<MOTOR_ON))
         {
            motorsekunde++;
            if (motorsekunde >= 3600)
            {
               motorsekunde = 0;
            }
         }
         
         if (senderstatus & (1<<STOP_ON))
         {
            stopsekunde++;
            if (stopsekunde >= 3600)
            {
               stopsekunde=0;
            }
         }
      }
   }
   

   
}


uint8_t Tastenwahl(uint8_t Tastaturwert)
{
   /*
    // Atmega168
    
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
   
   
   // Atmega328
#define TASTE1		17
#define TASTE2		29
#define TASTE3		44
#define TASTE4		67
#define TASTE5		94
#define TASTE6		122
#define TASTE7		155
#define TASTE8		166
#define TASTE9		214
#define TASTE_L	234
#define TASTE0		252
#define TASTE_R	255
*/
//lcd_gotoxy(0,0);
//lcd_putint(Tastaturwert);
   if (Tastaturwert < TASTE1)
      return 1;
   if (Tastaturwert < TASTE2)
      return 2;
   if (Tastaturwert < TASTE3)
      return 3;
   if (Tastaturwert < TASTE4)
      return 4;
   if (Tastaturwert < TASTE5)
      return 5;
   if (Tastaturwert < TASTE6)
      return 6;
   if (Tastaturwert < TASTE7)
      return 7;
   if (Tastaturwert < TASTE8)
      return 8;
   if (Tastaturwert < TASTE9)
      return 9;
   if (Tastaturwert < TASTE_L)
      return 10;
   if (Tastaturwert < TASTE0)
      return 0;
   if (Tastaturwert <= TASTE_R)
      return 12;
   
   return -1;
}

void resetcursorpos(void)
{
   // cursorpositionen init. Wert ist 0xFF wenn nicht verwendet
   uint8_t i=0,k=0;
   for (i=0;i<8;i++)
   {
      for (k=0;k<4;k++)
      {
         cursorpos[i][k]=0xFFFF;
        
      }
   }

}


void slaveinit(void)
{
	//MANUELL_DDR |= (1<<MANUELLPIN);		//Pin 3 von PORT D als Ausgang fuer Manuell
	//MANUELL_PORT &= ~(1<<MANUELLPIN);
 	//DDRD |= (1<<CONTROL_A);	//Pin 6 von PORT D als Ausgang fuer Servo-Enable
	//DDRD |= (1<<CONTROL_B);	//Pin 7 von PORT D als Ausgang fuer Servo-Impuls
	LOOPLED_DDR |= (1<<LOOPLED_PIN);
	//PORTD &= ~(1<<CONTROL_B);
	//PORTD &= ~(1<<CONTROL_A);

	//DDRB &= ~(1<<PORTB2);	//Bit 2 von PORT B als Eingang fuer Brennerpin
	//PORTB |= (1<<PORTB2);	//HI
	
//	DDRB |= (1<<PORTB2);	//Bit 2 von PORT B als Ausgang fuer PWM
//	PORTB &= ~(1<<PORTB2);	//LO

//	DDRB |= (1<<PORTB1);	//Bit 1 von PORT B als Ausgang fuer PWM
//	PORTB &= ~(1<<PORTB1);	//LO
	

	//LCD
	LCD_DDR |= (1<<LCD_RSDS_PIN);	//Pin 5 von PORT B als Ausgang fuer LCD
 	LCD_DDR |= (1<<LCD_ENABLE_PIN);	//Pin 6 von PORT B als Ausgang fuer LCD
	LCD_DDR |= (1<<LCD_CLOCK_PIN);	//Pin 7 von PORT B als Ausgang fuer LCD

//ADC
   ADC_DDR &= ~(1<<ADC_AKKUPIN);
   ADC_DDR &= ~(1<<TASTATURPIN);
   
   
	

//#define DOG_PORT        PORTD
  /*
   DOGM_DDR |= 1<< DOGM_MOSI_PIN;
   DOGM_DDR |= 1<< DOGM_SCL_PIN;
   DOGM_DDR |= 1<< DOGM_CS_PIN;
   DOGM_DDR |= 1<< DOGM_CMD_PIN;
*/
   /*
   //Pin 0 von   als Ausgang fuer OSZI
	OSZIPORTDDR |= (1<<OSZI_PULS_A);	//Pin 0 von  als Ausgang fuer LED TWI
   OSZIPORT |= (1<<OSZI_PULS_A);		// HI
	
   OSZIPORTDDR |= (1<<OSZI_PULS_B);		//Pin 1 von  als Ausgang fuer LED TWI
   OSZIPORT |= (1<<OSZI_PULS_B);		//Pin   von   als Ausgang fuer OSZI
*/
   
   resetcursorpos();
}


void lcd_putnav(void)
{
   lcd_gotoxy(0,0);
   lcd_putc('P');
   lcd_putc(':');
   lcd_putint1(navscreen);
   lcd_putc(' ');
   lcd_putc('L');
   lcd_putc(':');
   lcd_putint1(navline);
   lcd_putc(' ');
   lcd_putc('C');
   lcd_putc(':');
   lcd_putint1(navcol);
   lcd_putc(' ');
   lcd_putc('c');
   lcd_putint1(navcol);
   lcd_putc(' ');

   //lcd_puthex(navigation_old[navscreen][navline][navcol]);

}

void setdefaultsetting(void)
{
   uint8_t i=0,k=0;
   
   for (i=0;i<8;i++)
   {
      for (k=0;k<2;k++)
      {
         //curr_settingarray[i][k] = default_settingarray[i][k];
      }  
         curr_levelarray[i] = default_levelarray[i];
         curr_expoarray[i] = default_expoarray[i];
         curr_mixarray[i] = default_mixarray[i];
         curr_funktionarray[i] = default_funktionarray[i];
         curr_devicearray[i] = default_devicearray[i];
         curr_ausgangarray[i] = default_ausgangarray[i];

     
   }
}

int main (void)
{
	
	slaveinit();
	
	lcd_initialize(LCD_FUNCTION_8x2, LCD_CMD_ENTRY_INC, LCD_CMD_ON);
	lcd_puts("Guten Tag\0");
	delay_ms(1000);
	lcd_cls();
	lcd_puts("READY\0");
	
	
	

	delay_ms(1000);
	uint8_t i=0;
   
     
   lcd_gotoxy(0,0);
   //lcd_putint16( posregister[3][1]);
   
  //
   display_soft_init();
	
	//Display wird gelöscht
	display_clear();
	
   _delay_us(50);
   
   sethomescreen();
   char_height_mul = 1;
   char_width_mul = 1;

   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[5])));
   
   char_x=0;
   char_y = 3;
   //display_write_symbol(pfeilvollrechts);
   char_x += FONT_WIDTH;
   //display_write_str(titelbuffer,1);
   
   char_x=0;
   char_y = 4;

  // display_write_prop_str(char_y,char_x,0,(unsigned char*)titelbuffer);
   //display_write_str((unsigned char*)titelbuffer,1);
   
   char_x=0;
   char_y = 5;
   
   initADC(TASTATURPIN);
   delay_ms(1000);

   timer0();
   //delay_ms(1000);
 
   sei();
   //delay_ms(100);
	lcd_gotoxy(1,2);
	lcd_puts("graphstarter\0");
   delay_ms(1000);
   lcd_cls();
   
   setdefaultsetting();
   
   //levelwert = curr_settingarray[curr_kanal][0];
   //expowert = curr_settingarray[curr_kanal][1];


#pragma mark while
	while (1)
	{
		loopCount0 ++;
		//_delay_ms(2);
     // if (loopCount0 % 0x2ff == 1)
      
      uint16_t touchx = 0;
      uint16_t touchy = 0;
      
      if (loopCount0%64 == 1)
      {

      }
       if (loopCount0%8 == 4)
       {
          //display_clear();
       }
		
		if (loopCount0 >=0x08FF)
		{
         if (loopcount1 % 64==0)
         {
            /*
            switch (curr_screen)
            {
            case 0:
               {
            lcd_gotoxy(10,0);
            lcd_putint(motorsekunde);
            lcd_putc(' ');
            lcd_putint(stopsekunde);
            //display_write_symbol(pfeilvollrechts);
            
               }break;
               
            case 1: // Setting
               {
                  lcd_gotoxy(0,0);
                  lcd_puthex(curr_cursorzeile);
                  lcd_putc(' ');
                  lcd_puthex(curr_cursorspalte);
                  lcd_putc(' ');
                  lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                  lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
                  
                  if (curr_cursorzeile <8 && (cursorpos[curr_cursorzeile+1][curr_cursorspalte] < 0xFFFF))
                  {
                     lcd_putc('+');
                  }
                  else
                  {
                     lcd_putc('-');
                  }
                  
                  lcd_putc(' ');
                  lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                  lcd_putc(' ');
                  lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
               }break;
            } // switch curr_screen
             */
         }
         
         
         
			LOOPLED_PORT ^= (1<<LOOPLED_PIN);
			loopcount1++;
         if (programmstatus &(1<<MANUELL))
         {
            //manuellcounter++; // timeout von MANUELL incr.
         }
         
         if (loopcount1 % 16==0)
         {
            //batteriespannung = adc_read(0)/0x40; // ca. 6V
            batteriespannung = adc_read(0); // ca. 6V
            //lcd_gotoxy(0,1);
            //lcd_putint(updatecounter);
            
            //lcd_gotoxy(0,0);
            //lcd_putint12(batteriespannung);
            //uint16_t balkenhoehe =(batteriespannung-MINSPANNUNG)*64/(MAXSPANNUNG-MINSPANNUNG);
            //lcd_put_spannung(batteriespannung);

            // laufsekunde++;
            
            if (laufsekunde == 60)
            {
               laufminute++;
               //laufsekunde=0;

            }
            update_screen();
            
         }
         
         
         
         //lcd_gotoxy(0,1);
         //lcd_putint(laufsekunde);
         //lcd_putint(updatecounter);
         //lcd_putint(motorsekunde);
 
         
        // write_zahl_lcd(1,2,eeprom_read_byte(&EEjahr),2);
         //write_zahl_lcd(5,0,17,2);
         //write_zahl_lcd(12,0,manuellcounter,3);
         //write_zahl_lcd(15,0,3,3);
         
			
			if ((manuellcounter > MANUELLTIMEOUT) )
			{
				{
               programmstatus &= ~(1<<MANUELL);
               manuellcounter=0;
					MANUELL_PORT &= ~(1<<MANUELLPIN);
               
               if (curr_screen) // nicht homescreen
               {
                  display_clear();
                  curr_screen=0;
                  curr_cursorspalte=0;
                  curr_cursorzeile=0;
                  last_cursorspalte=0;
                  last_cursorzeile=0;
                  sethomescreen();
               }

				}
				//
			}
         
         if (programmstatus &(1<<MANUELL)) // Rueckwaertszaehlen fuer delay bis pagewechsel
         {
            if (navcounter)
            {
               navcounter--;
               //lcd_gotoxy(5,1);
               //lcd_putint(navcounter);
            }
         }
			
			loopCount0 =0;
		}
 		
#pragma mark Tastatur 
		/* ******************** */
		//initADC(TASTATURPIN);
		//Tastenwert=(readKanal(TASTATURPIN)>>2);
		if (loopCount0%128==1)
      {
         
         Tastenwert=adc_read(TASTATURPIN)>>2;
         //lcd_gotoxy(4,1);
         //lcd_putint(Tastenwert);
         //Tastenwert=0;
         if (Tastenwert>5)
         {
            /*
             0:											1	2	3
             1:											4	5	6
             2:											7	8	9
             3:											x	0	y
             4: Schalterpos -
             5: enter
             6: Schalterpos +
             7:
             8:
             9:
             
             12: Manuell aus
             */
            
            TastaturCount++;
            if ((TastaturCount>=200) )
            {
               lcd_gotoxy(16,1);
               lcd_puts("T:\0");
               //lcd_putint(Tastenwert);
               lcd_putc(' ');
               lcd_putc(' ');
               
               
               lcd_gotoxy(18,1);
               
               Taste=Tastenwahl(Tastenwert);
               lcd_putint2(Taste);
               //lcd_putc(' ');
               
               TastaturCount=0;
               Tastenwert=0x00;
               //uint8_t i=0;
               //uint8_t pos=0;
               //				lcd_gotoxy(18,1);
               //				lcd_putint2(Taste);
               programmstatus |= (1<<UPDATESCREEN);

               switch (Taste)
               {
                     #pragma mark Taste 0
                  case 0:// Schalter auf Null-Position
                  {
                     if (navscreen)
                     {
                        navscreen--;
                     }
                     
                     if (programmstatus & (1<<MANUELL))
                     {
                        manuellcounter=0;
                      }
                     
                  }break;
                     
                  case 1:
                  {
                     #pragma mark Taste 1
                    if (manuellcounter)
                    {
                       senderstatus ^= (1<<MOTOR_ON);
                        manuellcounter=0;
                        
                     }
                  }break;
                     
                  case 2://
                  {
                     #pragma mark Taste 2
                     switch (curr_screen)
                     {
                        case HOMESCREEN: // home
                        {
                           
                        }break;
                           
                        case SETTINGSCREEN: // Settings
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              //lcd_gotoxy(0,1);
                              if (curr_cursorzeile ) // curr_cursorzeile ist >0,
                              {
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile--;
                                 //lcd_putc('+');
                              }
                              else
                              {
                                 
                                 //lcd_putc('-');
                              }
                               manuellcounter=0;
                           }
                           else if (manuellcounter)
                           {
                              /*
                              lcd_gotoxy(0,1);
                              lcd_puthex((blink_cursorpos & 0xFF00)>>8);
                              lcd_putc('*');
                              lcd_puthex((blink_cursorpos & 0x00FF));
                               */
                              //switch((blink_cursorpos & 0xFF00)>>8) // Blink-Zeile
                              switch(curr_cursorzeile) // Blink-Zeile
                              {
                                 case 0: // modell
                                 {
                                    //switch (blink_cursorpos & 0x00FF)
                                    switch (curr_cursorspalte)
                                    {
                                       case 0:
                                       {
                                          //lcd_putc('0');
                                          if (curr_model )
                                          {
                                             curr_model--;
                                          }
   
                                       }break;
                                          
                                       case 1:
                                       {
                                          //lcd_putc('1');
                                          if (curr_setting )
                                          {
                                             curr_setting--;
                                          }

                                       }break;
                                    } // switch Spalte
                                    
                                 }break;
 
                                 case  1:
                                 {
                                    
                                 }break;

                                 case  2:
                                 {
 
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                           
                        }break;
                           
                        case KANALSCREEN: // Kanalsettings
                        {
                           /*
                           lcd_gotoxy(5,1);
                           lcd_puthex(curr_cursorzeile);
                           lcd_putc('*');
                           lcd_puthex((blink_cursorpos & 0xFF00)>>8); // Zeile
                           lcd_putc('*');
                           //lcd_putc('*');
                            */
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                          
                           {
                              if (curr_cursorzeile )//
                              {
                                 if (curr_cursorzeile<8)
                                 {
                                    char_height_mul=1;
                                 }
                                 else
                                 {
                                    char_height_mul=1;
                                 }

                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile--;
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc('+');
                              }
                              else
                              {
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc('-');
                              }
                              //lcd_putint2(curr_cursorzeile);
                              
                              //lcd_putc(' ');
                              
                              
                              manuellcounter=0;
                           }
                           else if (manuellcounter) // blinken ist on
                           {
                              
                              //switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanal
                                 {
                                    //uint8_t tempspalte = (blink_cursorpos & 0x00FF);
                                    //lcd_puthex(curr_cursorspalte);
                                    //blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          if (curr_kanal )
                                          {
                                             curr_kanal--;
                                          }
                                       }break;
                                          
                                       case 1: // Richtung
                                       {
                                          //if (curr_settingarray[curr_kanal][1] & 0x80)
                                          if (curr_expoarray[curr_kanal] & 0x80)
                                          {
                                             curr_expoarray[curr_kanal] &= ~0x80;
                                          }
                                          else
                                          {
                                             curr_expoarray[curr_kanal] |= 0x80;
                                          }
                                       }break;

                                       case 2: // Funktion
                                       {
                                          //lcd_gotoxy(5,1);
                                          //lcd_putc('*');
                                          //Bezeichnung von: FunktionTable[curr_funktionarray[curr_kanal]]
                                          //uint8_t tempfunktion = curr_funktionarray[curr_kanal]& 0x07; // Bit 0-3
                                          //lcd_puthex(tempfunktion);
                                          //lcd_putc('*');
                                          
                                          //tempfunktion--; // decrementieren
                                          //tempfunktion &= 0x07; // Begrenzen auf 0-7
                                          //lcd_puthex(tempfunktion);
                                          //curr_funktionarray[curr_kanal] |= tempfunktion; // cycle in FunktionTable
                                          
                                          uint8_t tempfunktion = curr_funktionarray[curr_kanal]&0x07; //bit 0-2
                                          tempfunktion--;
                                          tempfunktion &= 0x07;
                                          
                                          curr_funktionarray[curr_kanal] = (curr_funktionarray[curr_kanal]&0xF0)|tempfunktion; // cycle in FunktionTable

                                       }break;

                                          
                                          
                                    }// switch tempspalte
                                    
                                 }break;
                                    
                                 case  1: // level
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // Levelwert A
                                       {
                                          if ((curr_levelarray[curr_kanal] & 0x70)>>4)
                                          {
                                             curr_levelarray[curr_kanal] -= 0x10;
                                          }
                                          
                                       }break;
                                       case 1: // Levelwert B
                                       {
                                          if ((curr_levelarray[curr_kanal] & 0x07))
                                          {
                                             curr_levelarray[curr_kanal] -= 0x01;
                                          }
                                          
                                       }break;
                                          
                                       case 2: //
                                       {
                                          curr_cursorspalte = 1; // fehler, back
                                          
                                       }break;
                                          
                                    }// switch tempspalte
                                    
                                 }break;
                                    
                                 case  2: // Expo
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // expowert A
                                       {
                                          if ((curr_expoarray[curr_kanal] & 0x70)>>4)
                                          {
                                             curr_expoarray[curr_kanal] -= 0x10;
                                          }
                                       }break;
                                          
                                       case 1: // Expowert B
                                       {
                                          if ((curr_expoarray[curr_kanal] & 0x07))
                                          {
                                             curr_expoarray[curr_kanal] -= 0x01;
                                          }
                                       }break;
                                          
                                       case 2: //
                                       {
                                          curr_cursorspalte = 1; // fehler, back
                                       }break;
                                          
                                    }// switch tempspalte
                                 }break;
                                    
                                 case  4:
                                 {
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           } // if manuellcounter
                        }break; // canalscreen
                           
                        case MIXSCREEN:
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (curr_cursorzeile )//
                              {
                                 if (curr_cursorzeile<8)
                                 {
                                    char_height_mul=1;
                                 }
                                 else
                                 {
                                    char_height_mul=1;
                                 }
                                 
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile--;
                              }
                              manuellcounter=0;
                           }
                           else if (manuellcounter) // blinken ist on
                           {
                              switch (curr_cursorspalte)
                              {
                                 case 0: // Mix weiterschalten
                                 {
                                    
                                    if (curr_mixarray[2*curr_cursorzeile+1])
                                    {
                                       curr_mixarray[2*curr_cursorzeile+1] -= 0x01;// Mix ist auf ungerader zeile
                                    }
                                    
                                 }break;
                                    
                                 case 1: // Kanal A zurueckschalten
                                 {
                                    uint8_t tempdata =(curr_mixarray[2*curr_cursorzeile] & 0x70)>>4;// kanal a ist auf gerader zeile in bit 4-6
                                    
                                    if (tempdata) //
                                    {
                                       curr_mixarray[2*curr_cursorzeile] -= 0x10;
                                    }
                                    
                                    
                                 }break;
                                    
                                 case 2: // Kanal B zurueckschalten
                                 {
                                    uint8_t tempdata =(curr_mixarray[2*curr_cursorzeile] & 0x07);// kanal b ist auf gerader zeile in bit 0-2
                                    
                                    if (tempdata)
                                    {
                                       curr_mixarray[2*curr_cursorzeile] -= 0x01;
                                    }
                                    
                                 }break;
                                    
                              }// switch curr_cursorspalte

                              manuellcounter = 0;
                           }
                        }break; // mixscreen
                          
                           
                        case ZUTEILUNGSCREEN:
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (curr_cursorzeile )//
                              {
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile--;
                                 //lcd_puthex(curr_cursorzeile);
                                 //lcd_putc('+');
                              }
                              else
                              {
                              }
                               manuellcounter=0;
                           }
                           else if (manuellcounter) // blinken ist on
                           {
                              //funktionarray: bit 0-3: Kanal bit 4-7: Zuteilung an Pitch/Schieber/Schalter
                              /*
                               const char funktion0[] PROGMEM = "Seite  \0";
                               const char funktion1[] PROGMEM = "Hoehe  \0";
                               const char funktion2[] PROGMEM = "Quer   \0";
                               const char funktion3[] PROGMEM = "Motor  \0";
                               const char funktion4[] PROGMEM = "Quer L\0";
                               const char funktion5[] PROGMEM = "Quer R\0";
                               const char funktion6[] PROGMEM = "Lande  \0";
                               const char funktion7[] PROGMEM = "Aux    \0";
                               
                               */
                              /*
                               lcd_gotoxy(0,0);
                               lcd_puthex(curr_cursorzeile);
                               lcd_putc(' ');
                               lcd_puthex(curr_cursorspalte);
                               lcd_putc(' ');
                               */
                              switch (curr_cursorzeile)
                              {
                                 case 0: // pitch vertikal
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // L_V index 1
                                       {
                                          // Kanalnummer decrement
                                          if (((curr_devicearray[1]& 0x07)))
                                          {
                                             curr_devicearray[1]-= 0x01;
                                          }
                                       }break;
                                          
                                       case 1: // R_V index 3
                                       {
                                          if (((curr_devicearray[3]& 0x07)))
                                          {
                                             curr_devicearray[3]-= 0x01;
                                          }
                                       }break;
                                    }// switch curr_cursorspalte
                                 }break; // pitch v
                                    
                                 case 1: // pitch horizontal
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // L_H index 0
                                       {
                                          if (((curr_devicearray[0]& 0x07)))
                                          {
                                             // Kanalnummer fuer Device decrement
                                             curr_devicearray[0]-= 0x01;
                                          }
                                       }break;
                                          
                                       case 1: // R_H index 2
                                       {
                                          if (((curr_devicearray[2]& 0x07)))
                                          {
                                             curr_devicearray[2]-= 0x01;
                                          }
                                       }break;
                                    }// switch curr_cursorspalte
                                 }break; // case spalte
                                    
                                 case 2: // schieber
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // S_L index 4
                                       {
                                          if (((curr_devicearray[4]& 0x07)))
                                          {
                                             // Kanalnummer fuer Device increment
                                             curr_devicearray[4]-= 0x01;
                                          }
                                       }break;
                                          
                                       case 1: // S_R index 5
                                       {
                                          if (((curr_devicearray[5]& 0x07)))
                                          {
                                             curr_devicearray[5]-= 0x01;
                                          }
                                       }break;
                                    }// switch curr_cursorspalte
                                 }break; // case spalte
                                    
                              }//switch curr_cursorzeile
                              manuellcounter = 0;
                              
                           } // else if manuellcounter

                        }break; // zuteilungscreen
                           
                        case AUSGANGSCREEN:
                        {
                           #pragma mark 2 AUSGANGSCREEN
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (curr_cursorzeile)// noch nicht zuoberst
                              {
                                 char_height_mul=1;
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 if ((curr_impuls < 4) || (curr_impuls > 4))
                                 {
                                 curr_cursorzeile--;
                                 }
                                 else // zurueckscrollen
                                 {
                                    curr_cursorzeile = 3;
                                 }
                                 
                                 curr_impuls--;
                              }
                              manuellcounter=0;
                           }
                           else if (manuellcounter) // blinken ist on
                           {
                              //funktionarray: bit 0-3: Kanal bit 4-7: Zuteilung an Pitch/Schieber/Schalter
                              /*
                               const char funktion0[] PROGMEM = "Seite  \0";
                               const char funktion1[] PROGMEM = "Hoehe  \0";
                               const char funktion2[] PROGMEM = "Quer   \0";
                               const char funktion3[] PROGMEM = "Motor  \0";
                               const char funktion4[] PROGMEM = "Quer L\0";
                               const char funktion5[] PROGMEM = "Quer R\0";
                               const char funktion6[] PROGMEM = "Lande  \0";
                               const char funktion7[] PROGMEM = "Aux    \0";
                               */
                              /*
                              lcd_gotoxy(0,0);
                              lcd_puthex(curr_cursorzeile);
                              lcd_putc(' ');
                              lcd_puthex(curr_cursorspalte);
                              lcd_putc(' ');
                              */
                              switch (curr_cursorspalte)
                              {
                                 case 0: // Kanal
                                 {
                                    // Kanalnummer im Devicearray increment
                                    if (((curr_ausgangarray[curr_cursorzeile]& 0x07)))
                                    {
                                       curr_ausgangarray[curr_cursorzeile]-= 0x01;
                                    }
                                 }break;
                                    
                                 case 1: // Zeile  nach oben verschieben
                                 {
                                    if (((curr_ausgangarray[curr_cursorzeile]& 0x07))<8)
                                    {
                                       uint8_t tempzeilenwert =curr_ausgangarray[curr_cursorzeile];
                                       if (curr_impuls) // nicht erste Zeile, auf erster Seite
                                       {
                                          if ((curr_cursorzeile < 4) && (curr_impuls < 4)) // Noch vor scrollen, auf erster Seite
                                          {
                                             tempzeilenwert =curr_ausgangarray[curr_impuls];
                                             
                                             curr_ausgangarray[curr_impuls] =curr_ausgangarray[curr_impuls-1]; // Wert von naechster zeile
                                             curr_ausgangarray[curr_impuls -1] = tempzeilenwert;
                                             // cursorzeile verschieben
                                             display_cursorweg();
                                             
                                             curr_cursorzeile--;
                                             // blink-cursorzeile verschieben
                                             blink_cursorpos = cursorpos[curr_cursorzeile][curr_cursorspalte];
                                             
                                          }
                                          else  if ((curr_cursorzeile == 1) && (curr_impuls == 4))// zweite Zeile auf Seite 2, scrollen.
                                          {
                                             tempzeilenwert =curr_ausgangarray[curr_impuls];
                                             
                                             curr_ausgangarray[curr_impuls] =curr_ausgangarray[curr_impuls-1]; // Wert von naechster zeile, noch auf dieser Seite
                                             curr_ausgangarray[curr_impuls -1] = tempzeilenwert;
                                             display_cursorweg();
                                             curr_cursorzeile = 3; // Scroll
                                             // blink-cursorzeile verschieben
                                             blink_cursorpos = cursorpos[3][curr_cursorspalte];
                                          }
                                          else  if ((curr_cursorzeile) && (curr_impuls >4))// zweite Zeile oder mehr auf zweiter Seite
                                          {
                                             tempzeilenwert =curr_ausgangarray[curr_impuls];
                                             curr_ausgangarray[curr_impuls] =curr_ausgangarray[curr_impuls-1]; // Wert von naechster zeile
                                             curr_ausgangarray[curr_impuls -1] = tempzeilenwert;
                                             // cursorzeile verschieben
                                             display_cursorweg();
                                             
                                             curr_cursorzeile--;
                                             // blink-cursorzeile verschieben
                                             blink_cursorpos = cursorpos[curr_cursorzeile][curr_cursorspalte];
                                             
                                          }
                                          curr_impuls--;
                                       }
                                       else // letzte Zeile, mit erster zeile vertauschen
                                       {
                                          /*
                                           tempzeilenwert =curr_ausgangarray[curr_impuls];
                                           curr_ausgangarray[curr_impuls] =curr_ausgangarray[0]; // Wert von erster zeile
                                           curr_ausgangarray[0] = tempzeilenwert;
                                           display_cursorweg();
                                           curr_cursorzeile=0;
                                           curr_impuls =0;
                                           blink_cursorpos = cursorpos[0][curr_cursorspalte];
                                           */
                                       }
                                    }
                                    
                                 }break;
                                    
                                    
                              }// switch curr_cursorspalte
                              manuellcounter = 0;
                              
                           } // else if manuellcounter
                           
                        }break; // case ausgang

                     }// switch
                     
                  }break;
                     
                  case 3: //
                  {
                     #pragma mark Taste 3
                     if (manuellcounter)
                     {
                        senderstatus ^= (1<<STOP_ON);
                         manuellcounter=0;
                     }
                  }break;
                     
                  case 4:// nach links
                  {
                     #pragma mark Taste 4
                     switch (curr_screen)
                     {
                        case HOMESCREEN: // home
                        {
                           
                        }break;
                           
                        case SETTINGSCREEN: // Settings
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Modellname
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Modellname
                                       {
                                        
                                       }   break;
                                          
                                       case 1: // Set text
                                       {
                                          display_cursorweg();
                                          char_height_mul=1;
                                          last_cursorspalte =curr_cursorspalte;
                                          curr_cursorspalte--;
                                       }break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  2: // Kanal
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // levelwert A, nicht weiter nach links
                                       {
                                          
                                       }   break;
                                          
                                       case 1: // Levelwert B
                                       {
                                          display_cursorweg();
                                          char_height_mul=1;
                                          last_cursorspalte =curr_cursorspalte;
                                          curr_cursorspalte--;
                                       }break;
                                    }
                                 }break;
                                    
                                 case  3: // Mix
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // expowert A, nicht weiter nach links
                                       {
                                          
                                       }   break;
                                          
                                       case 1: // expowert B
                                       {
                                          display_cursorweg();
                                          char_height_mul=1;
                                          last_cursorspalte =curr_cursorspalte;
                                          
                                          curr_cursorspalte--;
                                          
                                       }break;
                                    }
                                    
                                 }break;
                                     //
                                    
                              }// switch
                              
                            }
                           else
                           {
                              switch(curr_cursorzeile) // zeile
                              
                              {
                                 case 0: // modell
                                 {
                                    
                                 }break;
                                 case  1:
                                 {
                                 }break;
                                    
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                           }
                           
                           
                        }break;
                           
                        case KANALSCREEN: // Kanal
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Kanaltext
                                       {
                                       }   break;
                                          
                                       default: // Richtung
                                       {
                                          display_cursorweg();
                                          char_height_mul=1;
                                          last_cursorspalte =curr_cursorspalte;
                                          
                                          curr_cursorspalte--;
                                          

                                       }break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1: // Level
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // levelwert A, nicht weiter nach links
                                       {
                                          
                                       }   break;
                                          
                                       default: // Levelwert B
                                       {
                                          display_cursorweg();
                                          char_height_mul=1;
                                          last_cursorspalte =curr_cursorspalte;
                                          
                                          curr_cursorspalte--;

                                       }break;
                                    }
                                 }break;
                                    
                                 case  2: // Expo
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // expowert A, nicht weiter nach links
                                       {
                                          
                                       }   break;
                                          
                                       case 1: // expowert B
                                       {
                                          display_cursorweg();
                                          char_height_mul=1;
                                          last_cursorspalte =curr_cursorspalte;
                                          
                                          curr_cursorspalte--;
                                      
                                       }break;
                                    }
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           else if (manuellcounter)
                           {
                              //switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0:
                                 {
                                    
                                 
                                 }break;
                                 case  1:
                                 {
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                        }break; // Kanalscreen

                        case MIXSCREEN: // Mixing
                        {
                           lcd_gotoxy(0,0);
                           
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (curr_cursorspalte)
                              {
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte--;
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc(' ');
                                 lcd_puthex(curr_cursorspalte);
                                 lcd_putc(' ');
                                 lcd_puthex(posregister[curr_cursorzeile][curr_cursorspalte+1]);
                              }
                              manuellcounter=0;
                              
                           }
                           else
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          
                                       }   break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1:
                                 {
                                    //blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                           }
                           
                           
                        }break;

                        case ZUTEILUNGSCREEN: // Zuteilung
                        {
                           lcd_gotoxy(0,0);
                           
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (curr_cursorspalte)
                              {
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte--;
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc(' ');
                                 lcd_puthex(curr_cursorspalte);
                                 lcd_putc(' ');
                                 lcd_puthex(posregister[curr_cursorzeile][curr_cursorspalte+1]);
                              }
                              manuellcounter=0;
                              
                           }
                           else if (manuellcounter)
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          
                                       }   break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1:
                                 {
                                    //blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                           
                        }break; // case Zuteilungscreen
                           
                        case AUSGANGSCREEN: // Ausgang
                        {
                           lcd_gotoxy(0,0);
                           
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (curr_cursorspalte)
                              {
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte--;
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc(' ');
                                 lcd_puthex(curr_cursorspalte);
                                 lcd_putc(' ');
                                 lcd_puthex(posregister[curr_cursorzeile][curr_cursorspalte+1]);
                              }
                              manuellcounter=0;
                              
                           }
                           else if (manuellcounter)
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          
                                       }   break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1:
                                 {
                                    //blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                           
                        }break; // case Ausgangscreen


                           
                     }// switch curr_screen
                     
                     
                  }break;
                     
                  case 5://
                  {
#pragma mark Taste 5
                     switch (curr_screen)
                     {
                        case HOMESCREEN:
                        {
                           if (startcounter == 0) // Settings sind nicht aktiv
                           {
                              programmstatus |= (1<< SETTINGWAIT);
                              settingstartcounter++;
                           }
                           else if (startcounter > 5) // Irrtum, kein Umschalten
                           {
                              programmstatus &= ~(1<< SETTINGWAIT);
                              settingstartcounter=0;
                              startcounter=0;
                               manuellcounter = 0;
                           }
                           else
                           {
                              if (programmstatus & (1<< SETTINGWAIT)) // Umschaltvorgang noch aktiv
                              {
                                 settingstartcounter++; // counter fuer klicks
                                 if (settingstartcounter > 2)
                                 {
                                    programmstatus &= ~(1<< SETTINGWAIT);
                                    settingstartcounter=0;
                                    startcounter=0;
                                    
                                    // Umschalten
                                    display_clear();
                                    setsettingscreen();
                                    curr_screen = SETTINGSCREEN;
                                    curr_cursorspalte=0;
                                    curr_cursorzeile=0;
                                    last_cursorspalte=0;
                                    last_cursorzeile=0;
                                    blink_cursorpos=0xFFFF;
                                    manuellcounter = 0;
                                    
                                 } // if settingcounter <
                              }
                           }
                         }break;
                           
                        case SETTINGSCREEN: // setting
                        {
                           if (manuellcounter)
                           {
                              switch (curr_cursorzeile)
                              {
                                 case 0: // Modell
                                 {
                                    
                                   // lcd_gotoxy(0,0);
                                    //lcd_puthex(curr_cursorzeile);
                                    //lcd_putc('*');
                                    //lcd_puthex(curr_cursorspalte);
                                    if (manuellcounter)
                                    {
                                       blink_cursorpos =  cursorpos[curr_cursorzeile][curr_cursorspalte];
                                       manuellcounter=0;
                                    } // if manuellcounter
/*
                                    switch (curr_cursorspalte)
                                    {
                                       case 0:
                                       {
                                         // lcd_putc('>');
                                          blink_cursorpos =  cursorpos[0][0]; // Modellname
                                          
                                       }break;
                                       case 1: // Set
                                       {
                                          lcd_putc('1');
                                          blink_cursorpos =  cursorpos[0][1]; // setting nummer
                                          
                                       }break;
                                       case 2:
                                       {
                                          blink_cursorpos =  cursorpos[0][2]; // setting nummer
                                       }break;
                                          
                                          
                                    } // switch curr_cursorspalte
 */
                                   }break;
                                    
                                    
                                 case 1: // Kanal
                                 {
                                    // Zu Kanal-Screen
                                    //blink_cursorpos =  cursorpos[2][0]; // canalcursor
                                    if (manuellcounter)
                                    {
                                       display_clear();
                                       
                                       curr_screen = KANALSCREEN;
                                       blink_cursorpos=0xFFFF;
                                       curr_cursorspalte=0;
                                       curr_cursorzeile=0;
                                       last_cursorspalte=0;
                                       last_cursorzeile=0;
                                       setcanalscreen();
                                       manuellcounter=0;
                                    }
                                    
                                    
                                 }break;
                                 case 2: // Mix
                                 {
                                    //zu Mix-Screen
                                    if (manuellcounter)
                                    {
                                       display_clear();
                                       
                                       curr_screen = MIXSCREEN;
                                       blink_cursorpos=0xFFFF;
                                       curr_cursorspalte=0;
                                       curr_cursorzeile=0;
                                       last_cursorspalte=0;
                                       last_cursorzeile=0;
                                       setmixscreen();
                                       manuellcounter=0;
                                    }
                                    
                                 }break;

                                 case 3: // Zuteilung
                                 {
                                    //zu Zuteilung-Screen
                                    if (manuellcounter)
                                    {
                                       display_clear();
                                       
                                       curr_screen = ZUTEILUNGSCREEN;
                                       blink_cursorpos=0xFFFF;
                                       curr_cursorspalte=0;
                                       curr_cursorzeile=0;
                                       last_cursorspalte=0;
                                       last_cursorzeile=0;
                                       setzuteilungscreen();
                                       manuellcounter=0;
                                    }
                                    
                                 }break;

                                 case 4: // Ausgang
                                 {
                                    //zu Zuteilung-Screen
                                    if (manuellcounter)
                                    {
                                       display_clear();
                                       
                                       curr_screen = AUSGANGSCREEN;
                                       blink_cursorpos=0xFFFF;
                                       curr_cursorspalte=0;
                                       curr_cursorzeile=0;
                                       last_cursorspalte=0;
                                       last_cursorzeile=0;
                                       setausgangscreen();
                                       manuellcounter=0;
                                    }
                                    
                                 }break;

                              
                              }// switch curr_cursorzeile
                           } // if manuellcounter
                           
                        }break;
                           
                        case KANALSCREEN: // Kanal
                        {
                           if (manuellcounter)
                           {
                              blink_cursorpos =  cursorpos[curr_cursorzeile][curr_cursorspalte];
                              manuellcounter=0;
                           } // if manuellcounter

                           /*
                           if (manuellcounter)
                           {
                              switch (curr_cursorzeile)
                              {
                                 case 0: // Kanal
                                 {
                                    
                                    switch (curr_cursorspalte)
                                    {
                                       case 0:
                                       {
                                          blink_cursorpos =  cursorpos[0][0]; // kanalcursor
                                       }break;
                                       case 1: // Richtung
                                       {
                                          
                                          blink_cursorpos =  cursorpos[0][1]; // richtungpfeilcursor

                                       }break;
                                       case 2: // funktion
                                       {
                                          blink_cursorpos =  cursorpos[0][2];
                                       }break;
                                          
                                          
                                    } // switch curr_cursorspalte
                                 }break;// case 0
                                    
                                 case 1: // Level
                                 {
                                    if (curr_cursorspalte < 2)
                                    {
                                       switch (curr_cursorspalte)
                                       {
                                          case 0:// kanalwert A
                                          {
                                             
                                             blink_cursorpos =  cursorpos[1][0]; // kanalwert A
                                          }break;
                                             
                                          case 1: // kanalwert B
                                             
                                          {
                                             blink_cursorpos =  cursorpos[1][1]; // kanalwert B
                                             
                                          }break;
                                          case 2:
                                          {
                                             blink_cursorpos =  1; //fehler, back
                                          }break;
                                             
                                       } //  case curr_cursorspalte
                                    }
                                    }break;
                                    
                                 case 2: // Expo
                                 {
                                    if (curr_cursorspalte < 2)
                                    {
                                       switch (curr_cursorspalte)
                                       {
                                          case 0:// expowert A
                                          {
                                             
                                             blink_cursorpos =  cursorpos[2][0]; // expowert A
                                          }break;
                                             
                                          case 1: // expowert B
                                             
                                          {
                                             
                                             blink_cursorpos =  cursorpos[2][1]; // expowert B
                                             
                                          }break;
                                          case 2:
                                          {
                                             blink_cursorpos =  1; //fehler, back

                                          }break;
                                             
                                       } //  case curr_cursorspalte
                                    }
                                  }break;
                                    
                                 case 3:
                                 {
                                    
                                 }break;
                                    
                                    
                              }// switch cursorzeile                        }break;
                              manuellcounter=0;
                           } // if manuellcounter
                           
                           //display_kanaldiagramm (char_x, uchar_y, level, expo, uint8_t typ )
                           // level: 0-3 expo: 0-3
                           //display_kanaldiagramm (64, 7, curr_levelarray[curr_kanal], curr_expoarray[curr_kanal], 1);
  
                           //manuellcounter=0;
                           */
                           
                        }break; // case kanalscreen
                           
                           
                        case MIXSCREEN: // Mixing
                        {
                           if (manuellcounter)
                           {
                              blink_cursorpos =  cursorpos[curr_cursorzeile][curr_cursorspalte];
                              manuellcounter=0;
                           } // if manuellcounter

                           /*
                           if (manuellcounter)
                           {
                              lcd_gotoxy(0,0);
                              lcd_puthex(curr_cursorzeile);
                              lcd_putc(' ');
                              lcd_puthex(curr_cursorspalte);
                              lcd_putc(' ');
                              //lcd_puthex(posregister[curr_cursorzeile][curr_cursorspalte+1]);

                              switch (curr_cursorzeile)
                              {
                                 case 0: // Mix 0
                                 {
                                    
                                    switch (curr_cursorspalte)
                                    {
                                       case 0:
                                       {
                                          blink_cursorpos =  cursorpos[0][0]; // Typ
                                       }break;
                                          
                                       case 1: // Kanal A
                                       {
                                          blink_cursorpos =  cursorpos[0][1]; // richtungpfeilcursor
                                          
                                       }break;
                                       case 2: // Kanal B
                                       {
                                          blink_cursorpos =  cursorpos[0][2];
                                       }break;
                                          
                                          
                                    } // switch curr_cursorspalte
                                 }break;// case 0
                                    
                                 case 1: // Mix 1
                                 {
                                    
                                       switch (curr_cursorspalte)
                                       {
                                          case 0:// Typ
                                          {
                                             
                                             blink_cursorpos =  cursorpos[1][0]; // Typ
                                          }break;
                                             
                                          case 1: // kanalwert A
                                             
                                          {
                                             blink_cursorpos =  cursorpos[1][1]; // Kanal A
                                             
                                          }break;
                                          case 2:
                                          {
                                             blink_cursorpos =  cursorpos[1][2];// Kanal B
                                          }break;
                                             
                                       } //  case curr_cursorspalte
                                    
                                 }break;
                                    
                                 case 2: //
                                 {
                                  }break;
                                    
                                 case 3:
                                 {
                                    
                                 }break;
                                    
                                    
                              }// switch cursorzeile                        }break;
                              manuellcounter=0;
                           } // if manuellcounter
                           */
                           
                           
                        }break; // case mixscreen
     
                        case ZUTEILUNGSCREEN: // Zuteilung
                        case AUSGANGSCREEN:
                        {
                           if (manuellcounter)
                           {
                              blink_cursorpos =  cursorpos[curr_cursorzeile][curr_cursorspalte];
                              manuellcounter=0;
                            } // if manuellcounter
                           
                           
                           
                        }break; // case zuteilungscreen
                          
                           
                     }// switch curr_screen
                  } break; // 5
                     
                     
                  case 6:// cursor nach rechts
                  {
                     #pragma mark Taste 6
                     switch (curr_screen)
                     {
                        case HOMESCREEN: // home
                        {
                           
                        }break;
                           
                        case SETTINGSCREEN: // Settings
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                               //lcd_gotoxy(0,1);
                              //if (curr_cursorspalte <1) // curr_cursorzeile ist >0,
                              if (posregister[curr_cursorzeile][curr_cursorspalte+1]<0xFFFF)
                              {
                                 if (curr_cursorzeile ==0)
                                 {
                                    char_height_mul=2;
                                 }
                                 display_cursorweg();
                                 
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte++;
                                 //lcd_putc('+');
                              }
                              else
                              {
                                 
                                 //lcd_putc('-');
                              }
                              manuellcounter=0;
                            }
                           else
                           {
                              switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              {
                                 case 0: // modell
                                 {
                                    
                                 }break;
                                 case  4:
                                 {
                                    blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                    //
                                    
                              }// switch
                           }
                           
                           
                        }break;
                           
                        case KANALSCREEN: // Kanal
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile][curr_cursorspalte+1]<0xFFFF)
                              {
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte++;
                              }
                              manuellcounter=0;
                            }
                           else if (manuellcounter)
                           {
                              switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (blink_cursorpos & 0x00FF) // cursorspalte
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          
                                       }   break;
                                    }
                                   // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1:
                                 {
                                    //blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                    
                                    
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                        }break;
 
                        case MIXSCREEN: // Mixing
                        {
                           lcd_gotoxy(0,0);
                           
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile][curr_cursorspalte+1]< 0xFFFF)
                              {
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte++;
                                 
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc(' ');
                                 lcd_puthex(curr_cursorspalte);
                                 lcd_putc(' ');
                                 lcd_puthex(posregister[curr_cursorzeile][curr_cursorspalte+1]);
                              }
                              manuellcounter=0;

                           }
                           else
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          
                                       }   break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1:
                                 {
                                    //blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                           }
                           
                           
                        }break;
                         
                           
                        case ZUTEILUNGSCREEN: // Zuteilung der Kanaele
                        {
                           lcd_gotoxy(0,0);
                           
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile][curr_cursorspalte+1]< 0xFFFF)
                              {
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte++;
                                 
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc(' ');
                                 lcd_puthex(curr_cursorspalte);
                                 lcd_putc(' ');
                                 lcd_puthex(posregister[curr_cursorzeile][curr_cursorspalte+1]);
                              }
                              manuellcounter=0;
                              
                           }
                           else
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          
                                       }   break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1:
                                 {
                                    //blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                           }
                           
                           
                        }break;

                           
                        case AUSGANGSCREEN: // Ausgang
                        {
                           lcd_gotoxy(0,0);
                           lcd_puthex(curr_cursorzeile);
                           lcd_putc(' ');
                           lcd_puthex(curr_cursorspalte);
                           lcd_putc(' ');
                           
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile][curr_cursorspalte+1]< 0xFFFF)
                              {
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte++;
                                 
                                  lcd_puthex(posregister[curr_cursorzeile][curr_cursorspalte+1]);
                              }
                              manuellcounter=0;
                              
                           }
                           else
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanaltext
                                 {
                                    switch (curr_cursorspalte) // cursorspalte
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          
                                       }   break;
                                    }
                                    // blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    
                                    
                                 }break;
                                    
                                    
                                 case  1:
                                 {
                                    //blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  2:
                                 {
                                    
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                           }
                           
                           
                        }break;

                     }// switch
                     
                        manuellcounter=0;
      
                     
                  }break;
                     
                     
                  case 7://home, in wenn 3* click aus default
                  {
                     #pragma mark Taste 7
                     //manuellcounter=0; // timeout zuruecksetzen
                     
                     if (curr_screen) // nicht homescreen
                     {
                        switch (curr_screen)
                        {
                           case HOMESCREEN:
                           {
                              display_clear();
                              curr_screen=0;
                              curr_cursorspalte=0;
                              curr_cursorzeile=0;
                              last_cursorspalte=0;
                              last_cursorzeile=0;
                              blink_cursorpos = 0xFFFF;
                              sethomescreen();
                           }break;
                              
                           case SETTINGSCREEN: // Settings
                           {
                              if ((blink_cursorpos == 0xFFFF) && manuellcounter)
                              {
                                 manuellcounter=0;
                                 display_clear();
                                 curr_screen=HOMESCREEN;
                                 curr_cursorspalte=0;
                                 //curr_cursorzeile=0;
                                 last_cursorspalte=0;
                                 //last_cursorzeile=0;
                                 blink_cursorpos = 0xFFFF;
                                 settingstartcounter=0;
                                 startcounter=0;

                                 sethomescreen();
 
                                 manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 
                                 blink_cursorpos = 0xFFFF;
                                 manuellcounter=0;
                              }
                              
                           }break;

                           case KANALSCREEN: // Settings
                           {
                              if ((blink_cursorpos == 0xFFFF) && manuellcounter)
                              {
                                 manuellcounter=0;
                                 
                                 blink_cursorpos = 0xFFFF;
                                 //
                                 if (curr_cursorspalte==0) // position am linken Rand
                                 {
                                    display_clear();
                                    curr_cursorzeile=0;
                                    last_cursorspalte=0;
                                    last_cursorzeile=0;
                                    curr_screen=SETTINGSCREEN;
                                    setsettingscreen();
                                 }
                                 else
                                 {
                                    
                                 }
                                 manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 blink_cursorpos = 0xFFFF;
                                 manuellcounter=0;
                              }
                           }break;
                              
                              
                           case LEVELSCREEN: // Level einstellen
                           {
                              if ((blink_cursorpos == 0xFFFF) && manuellcounter)
                              {
                                 manuellcounter=0;
                                 display_clear();
                                 curr_screen=KANALSCREEN;
                                 curr_cursorspalte=0;
                                 curr_cursorzeile=1; // Zeile Level
                                 last_cursorspalte=0;
                                 last_cursorzeile=0;
                                 blink_cursorpos = 0xFFFF;
                                 setcanalscreen();
                                 
                                 manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 
                                 blink_cursorpos = 0xFFFF;
                                 manuellcounter=0;
                              }
                              
                           }break;

                           case EXPOSCREEN: // Level einstellen
                           {
                              if ((blink_cursorpos == 0xFFFF) && manuellcounter)
                              {
                                 manuellcounter=0;
                                 display_clear();
                                 curr_screen=KANALSCREEN;
                                 curr_cursorspalte=0;
                                 curr_cursorzeile=2; //Zeile Expo
                                 last_cursorspalte=0;
                                 last_cursorzeile=0;
                                 blink_cursorpos = 0xFFFF;
                                 setcanalscreen();
                                 
                                 manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 
                                 blink_cursorpos = 0xFFFF;
                                 manuellcounter=0;
                              }
                              
                           }break;

                           case MIXSCREEN: // Settings
                           {
                              if ((blink_cursorpos == 0xFFFF) && manuellcounter)
                              {
                                 manuellcounter=0;
                                 
                                 blink_cursorpos = 0xFFFF;
                                 //
                                 if (curr_cursorspalte==0) // position am linken Rand
                                 {
                                    display_clear();
                                    curr_cursorzeile=0;
                                    last_cursorspalte=0;
                                    last_cursorzeile=0;
                                    curr_screen=SETTINGSCREEN;
                                    setsettingscreen();
                                 }
                                 else
                                 {
                                    
                                 }
                                 manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 
                                 blink_cursorpos = 0xFFFF;
                                  manuellcounter=0;
                              }
                           }break;

                           case ZUTEILUNGSCREEN: // Settings
                              
                           {
                              if ((blink_cursorpos == 0xFFFF) && manuellcounter)
                              {
                                 manuellcounter=0;
                                 
                                 blink_cursorpos = 0xFFFF;
                                 //
                                 if (curr_cursorspalte==0) // position am linken Rand
                                 {
                                    display_clear();
                                    curr_cursorzeile=0;
                                    last_cursorspalte=0;
                                    last_cursorzeile=0;
                                    curr_screen=SETTINGSCREEN;
                                    setsettingscreen();
                                 }
                                 else
                                 {
                                    
                                 }
                                 manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 
                                 blink_cursorpos = 0xFFFF;
                                 manuellcounter=0;
                              }
                           }break;

                           case AUSGANGSCREEN: // Settings
                              
                           {
                              if ((blink_cursorpos == 0xFFFF) && manuellcounter)
                              {
                                 manuellcounter=0;
                                 
                                 blink_cursorpos = 0xFFFF;
                                 //
                                 if (curr_cursorspalte==0) // position am linken Rand
                                 {
                                    
                                    display_clear();
                                    curr_cursorzeile=0;
                                    last_cursorspalte=0;
                                    last_cursorzeile=0;
                                    curr_screen=SETTINGSCREEN;
                                    setsettingscreen();
                                 }
                                 else
                                 {
                                    
                                 }
                                 manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 
                                 blink_cursorpos = 0xFFFF;
                                 manuellcounter=0;
                              }
                           }break;

                        }// switch
                     }
                     else // schon homescreen, motorzeit reset
                     {
                        if (manuellcounter) // kurz warten
                        {
                           senderstatus &= ~(1<<MOTOR_ON);
                           motorsekunde=0;
                           manuellcounter=0; // timeout zuruecksetzen
                        }
                     }
                  }break;
                     
                     
                  case 8://
                  {
                     #pragma mark Taste 8
                     switch (curr_screen)
                     {
                        case HOMESCREEN: // home
                        {
                           
                        }break;
                           
                        case SETTINGSCREEN: // Settings
                        {
                           if ((blink_cursorpos == 0xFFFF) && manuellcounter) // kein Blinken
                           {
                              lcd_gotoxy(5,1);
                              lcd_puthex(curr_cursorzeile);
                              lcd_putc('*');
                              lcd_puthex((posregister[curr_cursorzeile+1][curr_cursorspalte]&0xFF00)>>8);
                              lcd_putc('*');
                              lcd_puthex((posregister[curr_cursorzeile+1][curr_cursorspalte]&0x00FF));
                              lcd_putc('*');
                              
                              //if (curr_cursorzeile < 3 )//
                              if (posregister[curr_cursorzeile+1][curr_cursorspalte]<0xFFFF)
                              {
                                 if (curr_cursorzeile == 0)
                                 {
                                    char_height_mul = 2;
                                 }
                                 display_cursorweg();
                                 char_height_mul = 1;
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile++;
                                 
                                 lcd_gotoxy(19,1);
                                 lcd_putc('+');
                              }
                              else
                              {
                                 
                                 lcd_putc('-');
                              }
                              manuellcounter=0;
                           }
                           else if (manuellcounter)
                           {
                              /*
                               lcd_gotoxy(0,1);
                               lcd_puthex((blink_cursorpos & 0xFF00)>>8);
                               lcd_putc('*');
                               lcd_puthex((blink_cursorpos & 0x00FF));
                               lcd_putc(' ');
                               */
                              //switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // modell
                                 {
                                    //switch (blink_cursorpos & 0x00FF) // spalte
                                    switch (curr_cursorspalte) // spalte
                                    {
                                       case 0:
                                       {
                                          //lcd_putc('0');
                                          if (curr_model <8)
                                          {
                                             curr_model++;
                                          }
                                          
                                       }break;
                                          
                                       case 1:
                                       {
                                          //lcd_putc('1');
                                          if (curr_setting <4)
                                          {
                                             curr_setting++;
                                          }
                                          
                                       }break;
                                          
                                       case 2: //
                                       {
                                          
                                          
                                       }break;
                                          
                                    } // switch Spalte
                                    
                                 }break;
                                    
                                 case  1:
                                 {
                                    lcd_putc('1');
                                    
                                 }break;
                                    
                                 case  2:
                                 {
                                    lcd_putc('2');
                                 }break;
                                 case  3:
                                 {
                                    lcd_putc('3');
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter =0;
                           }
                        }break;
                           
                        case KANALSCREEN: // Kanalsettings
                        {
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile+1][curr_cursorspalte]<0xFFFF)//
                              {
                                 if (curr_cursorzeile==1)
                                 {
                                    char_height_mul=2;
                                 }
                                 else
                                 {
                                    char_height_mul=1;
                                 }
                                 
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile++;
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc('+');
                              }
                              else
                              {
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc('-');
                              }
                              manuellcounter=0;
                           }
                           else if (manuellcounter)
                           {
                              switch(curr_cursorzeile) // zeile
                              {
                                 case 0: // Kanal
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // Kanalnummer
                                       {
                                          if (curr_kanal < 7)
                                          {
                                             curr_kanal++;
                                          }
                                       }break;
                                       case 1: // Richtung
                                       {
                                          if (curr_expoarray[curr_kanal] & 0x80)
                                          {
                                             curr_expoarray[curr_kanal] &= ~0x80;
                                          }
                                          else
                                          {
                                             curr_expoarray[curr_kanal] |= 0x80;
                                          }
                                       }break;
                                          
                                       case 2: // Funktion
                                       {
                                          //lcd_gotoxy(5,1);
                                          //lcd_putc('*');
                                          //Bezeichnung von: FunktionTable[curr_funktionarray[curr_kanal]]&0x07
                                          // Funktion ist bit 0-2, Steuerdevice ist bit 4-6!!
                                          uint8_t tempfunktion = curr_funktionarray[curr_kanal]&0x07; //bit 0-2
                                           tempfunktion++;
                                          tempfunktion &= 0x07;
                                          
                                          //lcd_puthex(tempfunktion);
                                          curr_funktionarray[curr_kanal] = (curr_funktionarray[curr_kanal]&0xF0)|tempfunktion; // cycle in FunktionTable
                                          
                                          
                                          /*
                                          if (tempfunktion<8)
                                          {
                                             curr_funktionarray[curr_kanal] += 1;
                                          }
                                          */
                                       }break;
                                          
                                    }// switch tempspalte
                                    
                                 }break;
                                    
                                 case  1: // Level
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // Levelwert A
                                       {
                                          if (((curr_levelarray[curr_kanal] & 0x70)>>4)<5)
                                          {
                                             curr_levelarray[curr_kanal] += 0x10;
                                          }
                                          
                                       }break;
                                       case 1: // Levelwert B
                                       {
                                          if (((curr_levelarray[curr_kanal] & 0x07))<5)
                                          {
                                             curr_levelarray[curr_kanal] += 0x01;
                                          }
                                          
                                       }break;
                                          
                                       case 2: //
                                       {
                                          curr_cursorspalte = 1; // fehler, back
                                          
                                       }break;
                                    }// switch tempspalte
                                 }break;
                                    
                                 case  2: // Expo
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // Expowert A
                                       {
                                          if (((curr_expoarray[curr_kanal] & 0x70)>>4)<3)
                                          {
                                             curr_expoarray[curr_kanal] += 0x10;
                                          }
                                       }break;
                                          
                                       case 1: // Expowert B
                                       {
                                          if (((curr_expoarray[curr_kanal] & 0x07))<3)
                                          {
                                             curr_expoarray[curr_kanal] += 0x01;
                                          }
                                       }break;
                                          
                                       case 2: //
                                       {
                                          curr_cursorspalte = 1; // fehler, back
                                          
                                       }break;
                                    }// switch tempspalte
                                 }break;
                                 case  4: // Typ
                                 {
                                 }break;
                              }// switch
                              manuellcounter=0;
                           }
                        }break; // kanalscreen
                           
                        case MIXSCREEN:
                        {
                           
                           lcd_gotoxy(5,1);
                           lcd_puthex(curr_cursorzeile);
                           lcd_putc('*');
                           lcd_puthex((blink_cursorpos & 0xFF00)>>8); // Zeile
                           lcd_putc('*');
                           lcd_putc('*');
                           
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile+1][curr_cursorspalte]<0xFFFF)//
                              {
                                 char_height_mul=1;
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile++;
                                 
                                 lcd_puthex(curr_cursorzeile);
                                 lcd_putc('+');
                              }
                              else
                              {
                                 //lcd_puthex(curr_cursorzeile);
                                 //lcd_putc('-');
                              }
                              //lcd_putint2(curr_cursorzeile);
                              
                              //lcd_putc(' ');
                              
                              
                              manuellcounter=0;
                           }
                           else if (manuellcounter) // blinken ist on
                           {
                              /*
                               lcd_gotoxy(0,0);
                               lcd_puthex(curr_cursorzeile);
                               lcd_putc(' ');
                               lcd_puthex(curr_cursorspalte);
                               lcd_putc(' ');
                               */
                              switch (curr_cursorspalte)
                              {
                                 case 0: // Mix weiterschalten
                                 {
                                    if (curr_mixarray[2*curr_cursorzeile+1]<4)
                                    {
                                       curr_mixarray[2*curr_cursorzeile+1] += 0x01;// Mix ist auf ungerader zeile
                                    }
                                 }break;
                                    
                                 case 1: // Kanal A weiterschalten
                                 {
                                    uint8_t tempdata =(curr_mixarray[2*curr_cursorzeile] & 0x70)>>4;// kanal a ist auf gerader zeile in bit 4-6
                                    
                                    if (tempdata < 8) //
                                    {
                                       curr_mixarray[2*curr_cursorzeile] += 0x10;
                                    }
                                    
                                    
                                 }break;
                                    
                                 case 2: // Kanal B weiterschalten
                                 {
                                    uint8_t tempdata =(curr_mixarray[2*curr_cursorzeile] & 0x07);// kanal b ist auf gerader zeile in bit 0-2
                                    
                                    if (tempdata < 8)
                                    {
                                       curr_mixarray[2*curr_cursorzeile] += 0x01;
                                    }
                                    
                                 }break;
                                    
                              }// switch curr_cursorspalte
                              
                              manuellcounter = 0;
                              
                           }
                        }break; // mixscreen
                           
                           
                        case ZUTEILUNGSCREEN:
                        {
                           #pragma mark 8 ZUTEILUNGSCREEN
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile+1][curr_cursorspalte]<0xFFFF)//
                              {
                                 char_height_mul=1;
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 
                                 curr_cursorzeile++;
                                 
                                 //lcd_puthex(curr_cursorzeile);
                                 //lcd_putc('+');
                              }
                              else
                              {
                                 //lcd_puthex(curr_cursorzeile);
                                 //lcd_putc('-');
                              }
                              manuellcounter=0;
                           }
                           else if (manuellcounter) // blinken ist on
                           {
                           //funktionarray: bit 0-3: Kanal bit 4-7: Zuteilung an Pitch/Schieber/Schalter
                              /*
                               const char funktion0[] PROGMEM = "Seite  \0";
                               const char funktion1[] PROGMEM = "Hoehe  \0";
                               const char funktion2[] PROGMEM = "Quer   \0";
                               const char funktion3[] PROGMEM = "Motor  \0";
                               const char funktion4[] PROGMEM = "Quer L\0";
                               const char funktion5[] PROGMEM = "Quer R\0";
                               const char funktion6[] PROGMEM = "Lande  \0";
                               const char funktion7[] PROGMEM = "Aux    \0";

                               */
                              
                               lcd_gotoxy(0,0);
                               lcd_puthex(curr_cursorzeile);
                               lcd_putc(' ');
                               lcd_puthex(curr_cursorspalte);
                               lcd_putc(' ');
                              
                              switch (curr_cursorzeile)
                              {
                                 case 0: // pitch vertikal
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // L_V index 1
                                       {
                                          // Kanalnummer im Devicearray increment
                                          if (((curr_devicearray[1]& 0x07))<8)
                                          {
                                             curr_devicearray[1]+= 0x01;
                                          }
                                       }break;
                                          
                                       case 1: // R_V index 3
                                       {
                                          if (((curr_devicearray[3]& 0x07))<8)
                                          {
                                             curr_devicearray[3]+= 0x01;
                                          }
                                       }break;
                                    }// switch curr_cursorspalte
                                 }break; // pitch v
                                    
                                 case 1: // pitch horizontal
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // L_H index 0
                                       {
                                          if (((curr_devicearray[0]& 0x07))<8)
                                          {
                                             // Kanalnummer fuer Device increment
                                             curr_devicearray[0]+= 0x01;
                                          }
                                       }break;
                                          
                                       case 1: // R_H index 2
                                       {
                                          if (((curr_devicearray[2]& 0x07))<8)
                                          {
                                             curr_devicearray[2]+= 0x01;
                                          }
                                       }break;
                                    }// switch curr_cursorspalte
                                 }break; // case spalte


                                    
                                 case 2: // schieber
                                 {
                                    switch (curr_cursorspalte)
                                    {
                                       case 0: // S_L index 4
                                       {
                                          if (((curr_devicearray[4]& 0x07))<8)
                                          {
                                             // Kanalnummer fuer Device increment
                                             curr_devicearray[4]+= 0x01;
                                          }
                                       }break;
                                          
                                       case 1: // S_R index 5
                                       {
                                          if (((curr_devicearray[5]& 0x07))<8)
                                          {
                                             curr_devicearray[5]+= 0x01;
                                          }
                                       }break;
                                    }// switch curr_cursorspalte
                                 }break; // case spalte

                              }//switch curr_cursorzeile
                              manuellcounter = 0;
                              
                           } // else if manuellcounter
                           
                        }break; // case zuteilung
                      
                        case AUSGANGSCREEN:
                        {
#pragma mark 8 AUSGANGSCREEN
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              if (posregister[curr_cursorzeile+1][curr_cursorspalte]<0xFFFF)//
                              {
                                 char_height_mul=1;
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 if ((curr_cursorzeile < 3) || (curr_impuls > 3)) // Noch vor scrollen oder nach umschalten
                                 {
                                    curr_cursorzeile++;
                                 }
                                 else
                                 {
                                    curr_cursorzeile = 1; // Scroll
                                 }
                                 curr_impuls++;
                              }
                              
                               manuellcounter=0;
                           }
                           else if (manuellcounter) // blinken ist on
                           {
                              //funktionarray: bit 0-3: Kanal bit 4-7: Zuteilung an Pitch/Schieber/Schalter
                              /*
                               const char funktion0[] PROGMEM = "Seite  \0";
                               const char funktion1[] PROGMEM = "Hoehe  \0";
                               const char funktion2[] PROGMEM = "Quer   \0";
                               const char funktion3[] PROGMEM = "Motor  \0";
                               const char funktion4[] PROGMEM = "Quer L\0";
                               const char funktion5[] PROGMEM = "Quer R\0";
                               const char funktion6[] PROGMEM = "Lande  \0";
                               const char funktion7[] PROGMEM = "Aux    \0";
                               */
                              
                              lcd_gotoxy(0,0);
                              lcd_puthex(curr_cursorzeile);
                              lcd_putc(' ');
                              lcd_puthex(curr_cursorspalte);
                              lcd_putc(' ');
                              
                              switch (curr_cursorspalte)
                              {
                                 case 0: // Kanal
                                 {
                                    // Kanalnummer im Devicearray increment
                                    if (((curr_ausgangarray[curr_cursorzeile]& 0x07))<8)
                                    {
                                       curr_ausgangarray[curr_cursorzeile]+= 0x01;
                                    }
                                 }break;
                                    
                                 case 1: // Zeile nach unten verschieben
                                 {
                                    if (((curr_ausgangarray[curr_cursorzeile]& 0x07))<8)
                                    {
                                       uint8_t tempzeilenwert =curr_ausgangarray[curr_cursorzeile];
                                       if (curr_impuls < 7) // nicht letzte Zeile
                                       {
                                          if ((curr_cursorzeile < 3) && (curr_impuls < 3)) // Noch vor scrollen, auf erster Seite
                                          {
                                              tempzeilenwert =curr_ausgangarray[curr_impuls];
                                             
                                             curr_ausgangarray[curr_impuls] =curr_ausgangarray[curr_impuls+1]; // Wert von naechster zeile
                                             curr_ausgangarray[curr_impuls +1] = tempzeilenwert;
                                             // cursorzeile verschieben
                                             display_cursorweg();
                                             
                                             curr_cursorzeile++;
                                             // blink-cursorzeile verschieben
                                             blink_cursorpos = cursorpos[curr_cursorzeile][curr_cursorspalte];
                                             
                                          }
                                          else  if ((curr_cursorzeile == 3) && (curr_impuls == 3))// zweitunterste Zeile, scrollen.
                                          {
                                             tempzeilenwert =curr_ausgangarray[curr_impuls];
                                             curr_ausgangarray[curr_impuls] =curr_ausgangarray[curr_impuls+1]; // Wert von naechster zeile, noch auf dieser Seite
                                             curr_ausgangarray[curr_impuls +1] = tempzeilenwert;
                                             display_cursorweg();
                                             curr_cursorzeile = 1; // Scroll
                                             // blink-cursorzeile verschieben
                                             blink_cursorpos = cursorpos[1][curr_cursorspalte];
                                          }
                                          else  if ((curr_cursorzeile < 4) && (curr_impuls >3))// zweite Zeile oder mehr auf zweiter Seite
                                          {
                                             tempzeilenwert =curr_ausgangarray[curr_impuls];
                                             curr_ausgangarray[curr_impuls] =curr_ausgangarray[curr_impuls+1]; // Wert von naechster zeile
                                             curr_ausgangarray[curr_impuls +1] = tempzeilenwert;
                                             // cursorzeile verschieben
                                             display_cursorweg();
                                             
                                             curr_cursorzeile++;
                                             // blink-cursorzeile verschieben
                                             blink_cursorpos = cursorpos[curr_cursorzeile][curr_cursorspalte];
                                             
                                          }
                                          curr_impuls++;
                                       }
                                       else // letzte Zeile, mit erster zeile vertauschen
                                       {
                                          /*
                                          tempzeilenwert =curr_ausgangarray[curr_impuls];
                                          curr_ausgangarray[curr_impuls] =curr_ausgangarray[0]; // Wert von erster zeile
                                          curr_ausgangarray[0] = tempzeilenwert;
                                          display_cursorweg();
                                          curr_cursorzeile=0;
                                          curr_impuls =0;
                                          blink_cursorpos = cursorpos[0][curr_cursorspalte];
                                           */
                                       }
                                     }

                                 }break;
                              
                                    
                              }// switch curr_cursorspalte
                              manuellcounter = 0;
                              
                           } // else if manuellcounter
                           
                        }break; // case ausgang

                     }// switch
                     
                     
                     
                     
                  }break; // case 8
                     
                  case 9://set, out wenn auf home
                  {
                     #pragma mark Taste 9
                     if (manuellcounter) // kurz warten
                     {
                        senderstatus &= ~(1<<STOP_ON);
                        stopsekunde=0;
                        manuellcounter=0; // timeout zuruecksetzen
                     }

                  }break;
                     
                  case 10:// *Manuell einschalten
                  {
                     #pragma mark 
                    }break;
                     
                  case 11://
                  {
                     #pragma mark Taste 11
                  }break;
                     
                  case 12: // # Normalbetrieb einschalten
                  {
                     #pragma mark Taste 12
                     
                  }break;
                     
               }//switch Tastatur
               
               Tastenwert=0;
             }//if TastaturCount
            
         } // if Tastenwert > 5
      }
	}
	
	
	return 0;
}
