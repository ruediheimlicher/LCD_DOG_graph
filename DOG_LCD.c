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
volatile uint16_t                 TastaturCount=0;
volatile uint16_t                manuellcounter=0; // Countr fuer Timeout

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

volatile uint8_t expowert=0x82;
volatile uint8_t expob=0x01;



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
   
   TCCR0B |= (1 << CS02);//|(1 << CS00);

}


ISR (TIMER0_OVF_vect)
{
   
   mscounter++;
   
   if (mscounter > CLOCK_DIV)
   {
      programmstatus ^= (1<<MS_DIV);
      mscounter=0;
      
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
               display_clear();
               curr_screen=0;
               curr_cursorspalte=0;
               curr_cursorzeile=0;
               last_cursorspalte=0;
               last_cursorzeile=0;
               sethomescreen();

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
               /*
               lcd_gotoxy(0,1);
               
               lcd_putc(' ');lcd_putc(' ');lcd_putc(' ');
               lcd_gotoxy(0,1);
               
               lcd_putint12(TastaturCount);
              lcd_putc(' ');
               */
               lcd_gotoxy(16,1);
               lcd_puts("T:\0");
               //lcd_putint(Tastenwert);
               lcd_putc(' ');
               lcd_putc(' ');
               
               
               lcd_gotoxy(18,1);
               
               Taste=Tastenwahl(Tastenwert);
               lcd_putint2(Taste);
               //lcd_putc(' ');
               //lcd_putint(Tastenwahl(Tastenwert));
               
               //
               //delay_ms(600);
               // lcd_clr_line(1);
               
               
               TastaturCount=0;
               Tastenwert=0x00;
               uint8_t i=0;
               uint8_t pos=0;
               //				lcd_gotoxy(18,1);
               //				lcd_putint2(Taste);
               
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
                        //programmstatus |= (1<<MANUELLNEU);
                        /*
                         lcd_gotoxy(13,0);
                         lcd_puts("S\0");
                         lcd_gotoxy(19,0);
                         lcd_putint1(Schalterposition); // Schalterstellung
                         lcd_gotoxy(0,1);
                         lcd_puts("SI:\0");
                         lcd_putint(ServoimpulsdauerSpeicher); // Servoimpulsdauer
                         lcd_gotoxy(5,0);
                         lcd_puts("SP\0");
                         lcd_putint(Servoimpulsdauer); // Servoimpulsdauer
                         */
                     }
                     
                  }break;
                     
                  case 1:
                  {
                     #pragma mark Taste 1
                     navscreen = 1;
                     senderstatus ^= (1<<MOTOR_ON);
                     
                     if (programmstatus & (1<<MANUELL))
                     {
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
                              if (curr_cursorzeile  && (cursorpos[curr_cursorzeile-1][curr_cursorspalte] < 0xFFFF)) // curr_cursorzeile ist >0,
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
                              //lcd_putint2(curr_cursorzeile);
                              
                              //lcd_putc(' ');
                              
                              
                              
                              //lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                              
                              //lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
                              
                              /*
                               lcd_putc(' ');
                               lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                               lcd_putc(' ');
                               lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
                               */
                              manuellcounter=0;
                           }
                           else if (manuellcounter)
                           {
                              switch((blink_cursorpos & 0xFF00)>>8) // Blink-Zeile
                              {
                                 case MODELLCURSOR: // modell
                                 {
                                    if (curr_model )
                                    {
                                       curr_model--;
                                    }
                                    
                                 }break;
 
                                 case  SETCURSOR:
                                 {
                                    if (curr_setting)
                                    {
                                       curr_setting--;
                                    }
                                    
                                 }break;

                                 case  KANALCURSOR:
                                 {
                                    if (curr_kanal)
                                    {
                                       curr_kanal--;
                                    }
 
                                 }break;
                                 case  MIXCURSOR:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                           
                        }break;
                           
                        case KANALSCREEN: // Kanalsettings
                        {
                           lcd_gotoxy(5,1);
                           lcd_puthex(curr_cursorzeile);
                           lcd_putc('*');
                           lcd_puthex((blink_cursorpos & 0xFF00)>>8); // Zeile
                           lcd_putc('*');
                           //lcd_putc('*');
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              /*
                               lcd_gotoxy(0,0);
                               lcd_puthex(curr_cursorzeile);
                               lcd_putc(' ');
                               lcd_puthex(curr_cursorspalte);
                               lcd_putc(' ');
                               lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                               lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
                               */
                              
                              //lcd_gotoxy(0,1);
                              if (curr_cursorzeile )//&& (cursorpos[curr_cursorzeile-1][curr_cursorspalte] < 0xFFFF)) // curr_cursorzeile ist >0,
                              {
                                 if (curr_cursorzeile<8)
                                 {
                                    char_height_mul=2;
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
                           else if (manuellcounter)
                           {
                              
                              switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              {
                                 

                                 case 1: // Kanal
                                 {
                                    //uint8_t tempspalte = (blink_cursorpos & 0x00FF);
                                    lcd_puthex(curr_cursorspalte);
                                    //blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    switch (curr_cursorspalte)
                                    //switch (tempspalte) // blink spalte
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
                                          expowert ^= 0x80;
                                          
                                       }break;

                                       case 2: //
                                       {
                                          //expoa ^= 0x80;
                                          
                                       }break;

                                          
                                          
                                    }// switch tempspalte
                                    
                                 }break;
                                    
                                 case  2:
                                 {
                                    if (curr_setting)
                                    {
                                       curr_setting--;
                                    }
                                    
                                 }break;
                                    
                                 case  3:
                                 {
                                    if (curr_kanal)
                                    {
                                       curr_kanal--;
                                    }
                                    
                                 }break;
                                 case  4:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                           
                           
                           
                        }break;
                           
                           
                     }// switch

                     
                     
                     
                     
                     
                  }break;
                     
                  case 3: //
                  {
                     #pragma mark Taste 3
                     senderstatus ^= (1<<STOP_ON);
                     if (programmstatus & (1<<MANUELL))
                     {
                        manuellcounter=0;
                     }
                  }break;
                     
                  case 4://
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
                              //lcd_gotoxy(0,1);
                              if (curr_cursorspalte  && (cursorpos[curr_cursorzeile][curr_cursorspalte-1] < 0xFFFF)) // curr_cursorzeile ist >0,
                                 
                              {
                                 display_cursorweg();
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte--;
                                 //lcd_putc('+');
                              }
                              else
                              {
                                 
                                 //lcd_putc('-');
                              }
                           }
                           else
                           {
                              switch((blink_cursorpos & 0xFF00)>>8) //
                              {
                                 case MODELLCURSOR: // modell
                                 {
                                    
                                 }break;
                                 case  SETCURSOR:
                                 {
                                    blink_cursorpos =  cursorpos[1][0]; // settingcursor
                                 }break;
                                    
                                    
                                 case  KANALCURSOR:
                                 {
                                    
                                 }break;
                                 case  MIXCURSOR:
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
                              //lcd_gotoxy(0,1);
                              if (curr_cursorspalte  && (cursorpos[curr_cursorzeile][curr_cursorspalte-1] < 0xFFFF)) // curr_cursorzeile ist >0,
                                 
                              {
                                 if (curr_cursorspalte==1)
                                 {
                                    char_height_mul =2;
                                 }
                                 if (curr_cursorzeile<8)
                                     {
                                        char_height_mul=2;
                                     }
                                 else
                                 {
                                    char_height_mul=1;
                                 }
                                 display_cursorweg();
                                 char_height_mul=1;
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte--;
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
                              switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              {
                                 case 1:
                                 {
                                    
                                 
                                 }break;
                                 case  2:
                                 {
                                    blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  3:
                                 {
                                    
                                 }break;
                                 case  4:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                           
                        }break;

                     }// switch
                     
                     
                  }break;
                     
                  case 5://
                  {
#pragma mark Taste 5
                     
                     switch (curr_screen)
                     {
                        case HOMESCREEN:
                        {
                           programmstatus |= (1<<MANUELL); // reset auf homescreen nach timeout
                           //lcd_gotoxy(6,1);
                           //lcd_puts("set ");
                           delay_ms(20);
                           display_clear();
                           setsettingscreen();
                           curr_screen = SETTINGSCREEN;
                           curr_cursorspalte=0;
                           curr_cursorzeile=0;
                           last_cursorspalte=0;
                           last_cursorzeile=0;
                           blink_cursorpos=0xFFFF;
                           
                           lcd_gotoxy(0,1);
                           
                           uint8_t maxX=40,maxY=40,stufe=1;
                           
                           uint16_t endY= 0xFF*maxY*(4-stufe)/4; // punkte, nicht page
                           lcd_putint12(endY);
                           lcd_putc(' ');
                           lcd_putint12(endY/maxX);

                           uint8_t a=display_diagramm (64, 7, 0, 3, 1);
                           lcd_putc(' ');
                           lcd_putint(a);
                           
                           
                        }break;
                           
                        case SETTINGSCREEN: // setting
                        {
                           /*
                            lcd_gotoxy(6,1);
                            lcd_puts(" set");
                            lcd_putint2(curr_cursorzeile);
                            lcd_putc(' ');
                            lcd_putint2(curr_cursorspalte);
                            */
                           switch (curr_cursorzeile)
                           {
                              case 0: // Modell
                              {
                                 blink_cursorpos =  cursorpos[0][0]; // modellcursor
                                 manuellcounter=0;
                              }break;
                                 
                              case 1: // setting
                              {
                                 switch (curr_cursorspalte)
                                 {
                                    case 0:
                                    {
                                       blink_cursorpos =  cursorpos[1][0]; // settingcursor
                                    }break;
                                    case 1:
                                    {
                                       blink_cursorpos =  cursorpos[1][1]; // settingnummercursor
                                    }break;
                                 } // switch curr_cursorspalte
                                 manuellcounter=0;
                              }break;
                                 
                              case 2: // Kanal
                              {
                                 // Zu Kanal-Screen
                                 //blink_cursorpos =  cursorpos[2][0]; // canalcursor
                                 display_clear();
                                 
                                 curr_screen = KANALSCREEN;
                                 blink_cursorpos=0xFFFF;
                                 curr_cursorspalte=0;
                                 curr_cursorzeile=0;
                                 last_cursorspalte=0;
                                 last_cursorzeile=0;
                                 setcanalscreen();
                                 manuellcounter=0;
                                 
                                 
                              }break;
                              case 3: // Mix
                              {
                                 //zu Mix-Screen
                                 manuellcounter=0;
                              }break;
                                 
                           }// switch curr_cursorzeile
                           
                           
                        }break;
                           
                        case KANALSCREEN: // Kanal
                        {
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
                                          //  blink_cursorpos =  cursorpos[0][0]; // kanalcursor
                                          //blink_cursorpos = 0xFFFF;
                                          blink_cursorpos =  cursorpos[0][0]; // kanalcursor
                                       }break;
                                       case 1: // zu Levelscreen
                                       {
                                          
                                          
                                          blink_cursorpos =  cursorpos[0][0]; // richtungpfeilcursor

                                       }break;
                                       case 2:
                                       {
                                          
                                          blink_cursorpos =  cursorpos[0][3]; // richtungpfeilcursor
                                       }break;
                                          
                                          
                                    } // switch curr_cursorspalte
                                 }break;// case 0
                                    
                                 case 1: // Level
                                 {
                                    display_clear();
                                    
                                    curr_screen = LEVELSCREEN;
                                    blink_cursorpos=0xFFFF;
                                    curr_cursorspalte=0;
                                    curr_cursorzeile=0;
                                    last_cursorspalte=0;
                                    last_cursorzeile=0;
                                    setlevelscreen();
                                    manuellcounter=0;

                                 }break;
                                    
                                 case 2: // Expo
                                 {
                                    display_clear();
                                    
                                    curr_screen = EXPOSCREEN;
                                    blink_cursorpos=0xFFFF;
                                    curr_cursorspalte=0;
                                    curr_cursorzeile=0;
                                    last_cursorspalte=0;
                                    last_cursorzeile=0;
                                    setlevelscreen();
                                    manuellcounter=0;
                                  
                                 }break;
                                    
                                 case 3:
                                 {
                                    
                                 }break;
                                    
                                    
                              }// switch cursorzeile                        }break;
                           } // if manuellcounter
                           
                           
                           manuellcounter=0;
                           
                           
                        }break; // case kanalscreen
                     }// switch curr_screen
                  } break; // 5
                  case 6://
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
                              if (curr_cursorspalte < 3  && (cursorpos[curr_cursorzeile][curr_cursorspalte+1] < 0xFFFF)) // curr_cursorzeile ist >0,
                             
                              {
                                 
                                 display_cursorweg();
                                 last_cursorspalte =curr_cursorspalte;
                                 
                                 curr_cursorspalte++;
                                 //lcd_putc('+');
                              }
                              else
                              {
                                 
                                 //lcd_putc('-');
                              }
                            }
                           else
                           {
                              switch((blink_cursorpos & 0xFF00)>>8) //
                              {
                                 case MODELLCURSOR: // modell
                                 {
                                    
                                 }break;
                                 case  SETCURSOR:
                                 {
                                    blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  KANALCURSOR:
                                 {
                                    
                                 }break;
                                 case  MIXCURSOR:
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
                              //lcd_gotoxy(0,1);
                              if (curr_cursorspalte < 3 )// && (cursorpos[curr_cursorzeile][curr_cursorspalte+1] < 0xFFFF)) // curr_cursorzeile ist >0,
                                 
                              {
                                 //if (curr_cursorspalte<=1) // Kanaltext oder -nummer
                                 {
                                    char_height_mul =2;
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
                           }
                           else
                           {
                              switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              {
                                 case 0x01: // Kanaltext
                                 {
                                    blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                 }break;
                                 case  SETCURSOR:
                                 {
                                    blink_cursorpos =  cursorpos[1][1]; // settingcursor
                                 }break;
                                    
                                 case  KANALCURSOR:
                                 {
                                    
                                 }break;
                                 case  MIXCURSOR:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                           }
                           
                           
                        }break;
                           
                     }// switch
                     
                     
                     
                     
                     
                    // if (programmstatus & (1<<MANUELL))
                     {
                        manuellcounter=0;
                     }
                     
                  }break;
                     
                     
                  case 7://home, in wenn 3* click aus default
                  {
                     #pragma mark Taste 7
                     //manuellcounter=0; // timeout zuruecksetzen
                     
                     if (curr_screen) // nicht homescreen
                     {
                        
                        //curr_screen--;
                        
                        switch (curr_screen)
                        {
                           case HOMESCREEN:
                           {
                              //lcd_gotoxy(6,1);
                              //lcd_puts("home");
                              //delay_ms(20);
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
                                 curr_cursorzeile=0;
                                 last_cursorspalte=0;
                                 last_cursorzeile=0;
                                 blink_cursorpos = 0xFFFF;
                                 sethomescreen();
 
                              manuellcounter=0;
                              }
                              else if (manuellcounter)
                              {
                                 
                                 blink_cursorpos = 0xFFFF;
                                 switch((blink_cursorpos & 0xFF00)>>8) //
                                 {
                                    case MODELLCURSOR: // modell
                                    {
                                       
                                    }break;
                                    case  SETCURSOR:
                                    {

                                    }break;
                                    case  KANALCURSOR:
                                    {

                                    }break;
                                    case  MIXCURSOR:
                                    {

                                    }break;
                                       //
                                       
                                 }// switch
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
                                 switch((blink_cursorpos & 0xFF00)>>8) //
                                 {
                                    case 1: //
                                    {
                                       
                                    }break;
                                    case  2:
                                    {
                                       
                                    }break;
                                    case  3:
                                    {
                                       
                                    }break;
                                    case  4:
                                    {
                                       
                                    }break;
                                       //
                                       
                                 }// switch
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
                                 switch((blink_cursorpos & 0xFF00)>>8) //
                                 {
                                    case 0: // Titel
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
                                 switch((blink_cursorpos & 0xFF00)>>8) //
                                 {
                                    case 0: // Titel
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

                     
                     /*
                      navscreen --;
                      navline = 0;
                      navcol = 0;
                      if (navscreen == 0)
                      {
                      blinkline=1;
                      blinkcol=6;
                      blinkzeichen = modelnummer + '0';
                      
                      }
                      */
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
                           if (blink_cursorpos == 0xFFFF && manuellcounter)
                           {
                              /*
                               lcd_gotoxy(0,0);
                               lcd_puthex(curr_cursorzeile);
                               lcd_putc(' ');
                               lcd_puthex(curr_cursorspalte);
                               lcd_putc(' ');
                               lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                               lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
                               */
                              //lcd_gotoxy(0,1);
                              
                              if (curr_cursorzeile < 3 )//&& (cursorpos[curr_cursorzeile+1][curr_cursorspalte] < 0xFFFF))
                              {
                                 display_cursorweg();
                                 last_cursorzeile =curr_cursorzeile;
                                 curr_cursorzeile++;
                                 
                                 lcd_putc('+');
                              }
                              else
                              {
                                 
                                 lcd_putc('-');
                              }
                              lcd_putint2(curr_cursorzeile);
                              lcd_putc(' ');
                              /*
                               lcd_putc(' ');
                               lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                               lcd_putc(' ');
                               lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
                               */
                              manuellcounter=0;
                           }
                           else
                           {
                              switch((blink_cursorpos & 0xFF00)>>8) //
                              {
                                 case 0: // modell
                                 {
                                    if (curr_model < 8)
                                    {
                                       curr_model++;
                                    }
                                 
                                 }break;
                                    
                                 case  1:
                                 {
                                    if (curr_setting<8)
                                    {
                                       curr_setting++;
                                    }
                                    
                                 }break;
                                    
                                 case  2:
                                 {
                                    if (curr_kanal < 8)
                                    {
                                       curr_kanal++;
                                    }
                                   
                                 }break;
                                 case  3:
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                           }

                           
                        }break;
                           
                        case KANALSCREEN: // Kanalsettings
                        {
                           lcd_gotoxy(5,1);
                           lcd_puthex(curr_cursorzeile);
                           lcd_putc('*');
                           lcd_puthex((blink_cursorpos & 0xFF00)>>8); // Zeile
                           lcd_putc('*');
                           //lcd_puthex((blink_cursorpos & 0x00FF));
                           //lcd_putc('*');
                           if (blink_cursorpos == 0xFFFF && manuellcounter) // Kein Blinken
                           {
                              /*
                               lcd_gotoxy(0,0);
                               lcd_puthex(curr_cursorzeile);
                               lcd_putc(' ');
                               lcd_puthex(curr_cursorspalte);
                               lcd_putc(' ');
                               lcd_puthex(((cursorpos[curr_cursorzeile][curr_cursorspalte])& 0xFF00)>>8);
                               lcd_puthex(cursorpos[curr_cursorzeile][curr_cursorspalte]);
                               */
                              
                              //lcd_gotoxy(0,1);
                              if (curr_cursorzeile <3)// && (cursorpos[curr_cursorzeile-1][curr_cursorspalte] < 0xFFFF)) // curr_cursorzeile ist >0,
                              {
                                 if (curr_cursorzeile<3)
                                 {
                                    char_height_mul=2;
                                 }
                                 else
                                 {
                                    char_height_mul=1;
                                 }

                                 display_cursorweg();
                                 
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
                              //lcd_putint2(curr_cursorzeile);
                              
                              //lcd_putc(' ');
                              
                              manuellcounter=0;
                              
                           }
                           else if (manuellcounter)
                           {
                              
                              switch((blink_cursorpos & 0xFF00)>>8) // zeile
                              {
                                    
                                    
                                 case 1: // Kanal
                                 {
                                    //uint8_t tempspalte = (blink_cursorpos & 0x00FF);
                                    lcd_puthex(curr_cursorspalte);
                                    //blink_cursorpos =  cursorpos[0][1]; // richtungcursor
                                    switch (curr_cursorspalte)
                                    //switch (tempspalte) // blink spalte
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
                                          expowert ^= 0x80;
                                          
                                       }break;
                                          
                                       case 2: //
                                       {
                                          
                                          
                                       }break;
                                          
                                          
                                          
                                    }// switch tempspalte
                                    
                                 }break;
                                    
                                 case  2: // Level
                                 {
                                     
                                 }break;
                                    
                                 case  3: // Expo
                                 {
                                    
                                 }break;
                                 case  4: // Typ
                                 {
                                    
                                 }break;
                                    //
                                    
                              }// switch
                              manuellcounter=0;
                           }
                           
                           
                           
                           
                        }break;
   
                           
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
                     #pragma mark Taste 10
                     if (!(programmstatus & (1<<MANUELL)))
                     {
                        lcd_gotoxy(4,1);
                        
                        if (navcounter == 0) // Beginn einschalten
                        {
                           navfix = loopcount1;
                           navcounter++;
                        }
                        else if ((loopcount1 - navfix) < MINWAIT)
                        {
                           navcounter++;
                        }
                        if (navcounter > 3)
                        {
                           MANUELL_PORT |= (1<<MANUELLPIN);
                           programmstatus |= (1<<MANUELL);	// MANUELL ON
                           navcounter = 0;
                           navfix = 0;
                           navscreen = 1; // Startpage von Setting anzeigen
                           //displaynav(navscreen, navline,navcol);
                           //lcd_putnav();
                           
                        }
                        
                        
                     }
                     else if (navcounter == 0)
                     {
                        
                        
                        navscreen--;
                        if (navscreen==0) // Settings verlassen
                        {
                           navcounter = 0;
                           navline=0;
                           navcol=0;
                           blinkline = 0xFF;
                           blinkcol = 0xFF;
                           MANUELL_PORT &= ~(1<<MANUELLPIN);
                           programmstatus &= ~(1<<MANUELL);
                        }
                        
                     }
                     
                     manuellcounter=0; // timeout zuruecksetzen
                     //displaynav(navscreen, navline,navcol);
                     
                  }break;
                     
                  case 11://
                  {
                     #pragma mark Taste 11
                  }break;
                     
                  case 12: // # Normalbetrieb einschalten
                  {
                     #pragma mark Taste 12
                     /*
                      if (navscreen)
                      {
                      navscreen--;
                      }
                      */
                     if (navscreen == 0)
                     {
                        programmstatus &= ~(1<<MANUELL); // MANUELL OFF
                        //programmstatus &= ~(1<<MANUELLNEU);
                        MANUELL_PORT &= ~(1<<MANUELLPIN);
                     }
                     
                  }break;
                     
               }//switch Tastatur
               
#pragma mark Tasten
               
               
               
               /*
                Tastenwert=0;
                initADC(tastatur_kanal);
                //err_gotoxy(10,1);
                //err_puts("TR \0");
                
                Tastenwert=(uint8_t)(readKanal(tastatur_kanal)>>2);
                */
               Tastenwert=0;
               //err_puthex(Tastenwert);
               
               //			closeADC();
               
               //uint8_t Taste=-1;
               //		Tastenwert/=8;
               //		Tastenwert*=3;
               //		err_clr_line(1);
               //		err_gotoxy(0,1);
               //		err_puts("Taste \0");
               //		err_putint(Taste);
               //		delay_ms(200);
               
               
               
               //if (Tastenwert>5) // ca Minimalwert der Matrix
               
               //				delay_ms(400);
               //				lcd_gotoxy(18,1);
               //				lcd_puts("  ");		// Tastenanzeige loeschen
               
            }//if TastaturCount	
            
         } // if Tastenwert > 5
      }
	}
	
	
	return 0;
}
