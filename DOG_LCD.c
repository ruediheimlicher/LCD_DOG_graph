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

#include "dog_spi.c"

//#include "dog_graph.c"

#include "font.h"
#include "display.c"

#include "text.h"

char menubuffer[20];
char titelbuffer[20];


const  char pic1[] PROGMEM = LOGO;


uint16_t loopCount0=0;
volatile uint16_t loopcount1=0;
uint16_t loopCount2=0;

#define PROGRAMM_DS	1
#define GRUPPE_DS		0xC0
//#define GRUPPE_DS	0xB0

#define TWI_PORT		PORTC
#define TWI_PIN		PINC
#define TWI_DDR		DDRC

#define SDAPIN		4
#define SCLPIN		5


#define LOOPLED_PORT	PORTD
#define LOOPLED_DDR	DDRD
#define LOOPLED_PIN	4

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


#define MANUELL			7	// Bit 7 von Status 
#define MANUELLPIN		3	// Pin 6 von PORT D fuer Anzeige Manuell
//#define MANUELLNEU		7	// Pin 7 von Status. Gesetzt wenn neue Schalterposition eingestellt
#define MANUELLTIMEOUT	100 // Loopled-counts bis Manuell zurueckgesetzt wird. 02FF: ca. 100 s
#define MINWAIT         3 // Anzahl loops von loopcount1 bis einschalten

const char funktion0[] PROGMEM = "Hoehe\0";
const char funktion1[] PROGMEM = "Seite\0";
const char funktion2[] PROGMEM = "Quer\0";
const char funktion3[] PROGMEM = "Motor\0";
const char funktion4[] PROGMEM = "Quer_L\0";
const char funktion5[] PROGMEM = "Quer_R\0";
const char funktion6[] PROGMEM = "Lande\0";
const char funktion7[] PROGMEM = "Aux\0";

PGM_P const FunktionTable[] PROGMEM = {funktion0, funktion1, funktion2, funktion3, funktion4, funktion5, funktion6, funktion7};



volatile uint8_t  dog_curr_page=0; // aktuelle page
volatile uint8_t  dog_curr_col=0; // aktuelle colonne


volatile uint8_t              navcounter=0; // Zaehler fuer Einschalten
volatile uint16_t              navfix=0; // Stand des loopcounters fuer Einschalten


volatile uint8_t              navpage=0; // aktuelle seite
volatile uint8_t              navcol=0; // aktuelle kolonne
volatile uint8_t              navline=0; // aktuelle linie

volatile uint8_t            blinkline=0xFF;
volatile uint8_t            blinkcol=0xFF;
volatile char            blinkzeichen = ' ';


volatile uint8_t           modelnummer=1;


volatile uint8_t					programmstatus=0x00;
uint8_t Tastenwert=0;
uint8_t TastaturCount=0;
volatile uint16_t					manuellcounter=0; // Countr fuer Timeout
uint16_t TastenStatus=0;
uint16_t Tastencount=0;
uint16_t Tastenprellen=0x01F;

volatile uint8_t Menu_Ebene=0;
volatile uint8_t  Kanal_Thema =0;

volatile uint8_t  Anzeigekanal =0;


volatile uint8_t laufsekunde=0;
volatile uint8_t laufminute=0;


//#define MAXSENSORS 5
static volatile uint8_t sensornummer=0;
// Code 1_wire start
//uint8_t gSensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];

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

uint8_t Tastenwahl(uint8_t Tastaturwert)
{
//lcd_gotoxy(0,1);
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
if (Tastaturwert < TASTE_R)
return 12;

return -1;
}


void slaveinit(void)
{
	MANUELL_DDR |= (1<<MANUELLPIN);		//Pin 3 von PORT D als Ausgang fuer Manuell
	MANUELL_PORT &= ~(1<<MANUELLPIN);
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


	

//#define DOG_PORT        PORTD
  /*
   DOGM_DDR |= 1<< DOGM_MOSI_PIN;
   DOGM_DDR |= 1<< DOGM_SCL_PIN;
   DOGM_DDR |= 1<< DOGM_CS_PIN;
   DOGM_DDR |= 1<< DOGM_CMD_PIN;
*/
   
   //Pin 0 von   als Ausgang fuer OSZI
	OSZIPORTDDR |= (1<<OSZI_PULS_A);	//Pin 0 von  als Ausgang fuer LED TWI
   OSZIPORT |= (1<<OSZI_PULS_A);		// HI
	
   OSZIPORTDDR |= (1<<OSZI_PULS_B);		//Pin 1 von  als Ausgang fuer LED TWI
   OSZIPORT |= (1<<OSZI_PULS_B);		//Pin   von   als Ausgang fuer OSZI

}


void lcd_putnav(void)
{
   lcd_gotoxy(0,0);
   lcd_putc('P');
   lcd_putc(':');
   lcd_putint1(navpage);
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

   //lcd_puthex(navigation_old[navpage][navline][navcol]);

}




int main (void)
{
	/* INITIALIZE */
//	LCD_DDR |=(1<<LCD_RSDS_PIN);
//	LCD_DDR |=(1<<LCD_ENABLE_PIN);
//	LCD_DDR |=(1<<LCD_CLOCK_PIN);
	
   
	
	
	slaveinit();
	
	lcd_initialize(LCD_FUNCTION_8x2, LCD_CMD_ENTRY_INC, LCD_CMD_ON);
	lcd_puts("Guten Tag\0");
	delay_ms(1000);
	lcd_cls();
	lcd_puts("READY\0");
	
	
	delay_ms(100);
	lcd_gotoxy(0,0);
	lcd_puts("graphstarter\0");
	

	delay_ms(1000);
	uint8_t i=0;
   
   

 //
   display_soft_init();
   
//	display_mem(pic1);
   //Warteschleife 3s fuer Logo
	for(unsigned long counter= 0;counter<1000000;counter++){asm("nop");};
	
	//Display wird gelöscht
	display_clear();
	
	//char_x=50;//0-128
	//char_y=4; //0-8
	
	//display_write("Chief is here");

   /*
   initDOGL();
   _delay_us(50);
   //void charChain(uint8_t page, uint8_t column, uint8_t  inverse, const uint8_t *pChain)
   charChain(0,0,0,(uint8_t*)"Hallo");
   */
   _delay_us(50);
   
   
   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[0])));
   //lcd_gotoxy(4,1);
   //lcd_puts(titelbuffer);
   char_x=0;
   char_y = 1;

   display_write_str(titelbuffer);

   strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(TitelTable[1])));

   
   char_x=0;
   char_y = 2;
   display_write_str(titelbuffer);
   char_x=0;
   char_y = 3;


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
         //display_write("Hallo");
         //display_mem(pic1);
         //charChain(0,laufsekunde,1,(uint8_t*)"Hallo");
         //_delay_us(50);

      }
       if (loopCount0%8 == 4)
       {
          //display_clear();
       }
		
		if (loopCount0 >=0x08FF)
		{
         //if (loopcount1<6)
         {
            uint8_t zeit = loopcount1;
            
         //   char_height_mul=2;
         //display_write_byte(0,'A');
            if (zeit<10)
            {
               char_x=2*FONT_WIDTH;
            }
            else if (zeit<100)
            {
               char_x=1*FONT_WIDTH;
            }
            else
            {
               char_x=0;
            }
            char_y = 3;
            //if (zeit%2)
            {
              // display_inverse(zeit%2);
            }
            OSZI_A_LO;
            
            display_write_int(loopcount1 & 0xFF);
            OSZI_A_HI;
         //display_write_char('A'+ loopcount1);
            
         }
			LOOPLED_PORT ^= (1<<LOOPLED_PIN);
			loopcount1++;
         if (programmstatus &(1<<MANUELL))
         {
            manuellcounter++; // timeout von MANUELL incr.
         }
         
         if (loopcount1 % 2)
         {
            
            laufsekunde++;
            if (laufsekunde == 60)
            {
               laufminute++;
               laufsekunde=0;
               
            }
         }
         lcd_gotoxy(0,1);
         lcd_putint(laufsekunde);
         
 
         //volatile uint8_t jahr=0;
         //jahr = eeprom_read_byte(&EEjahr);
         //lcd_gotoxy(0,1);
         //lcd_putint(jahr);

         
        // write_zahl_lcd(1,2,eeprom_read_byte(&EEjahr),2);
         //write_zahl_lcd(5,0,17,2);
         //write_zahl_lcd(12,0,manuellcounter,3);
         //write_zahl_lcd(15,0,3,3);
         
         /*
         write_byte_lcd(12,1,loopcount1 % 8);
         if (loopcount1 % 2)
         {
          
            write_byte_lcd(0,1,32);
            write_byte_lcd(11,1,32);
            
         }
         else
         {
            write_byte_lcd(0,1,0x7e);
            write_byte_lcd(11,1,0x7f);
         }
         
         write_byte_lcd(9,2,2);
         write_byte_lcd(11,2,3);
*/
			
			if ((manuellcounter > MANUELLTIMEOUT) )
			{
				{
               programmstatus &= ~(1<<MANUELL);
               manuellcounter=0;
					MANUELL_PORT &= ~(1<<MANUELLPIN);
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
		initADC(TASTATURPIN);
		Tastenwert=(readKanal(TASTATURPIN)>>2);
		
//		lcd_gotoxy(3,1);
//		lcd_putint(Tastenwert);
//		Tastenwert=0;
		if (Tastenwert>5)
		{
			/*
			 0:											1	2	3
			 1:											4	5	6
			 2:											7	8	9
			 3:											x	0	y
			 4: Schalterpos -
			 5: Manuell ein
			 6: Schalterpos +
			 7: 
			 8: 
			 9: 
			 
			 12: Manuell aus
			 */
			 
			TastaturCount++;
			if (TastaturCount>=200)
			{
				
				 
				 lcd_gotoxy(15,1);
				 lcd_puts("T:  \0");
				 //lcd_putint(Tastenwert);
				 
				uint8_t Taste=Tastenwahl(Tastenwert);
				//Taste=0;
				 lcd_gotoxy(18,1);
				 lcd_putint2(Taste);
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
					case 0:// Schalter auf Null-Position
					{
                  if (navpage)
                  {
                     navpage--;
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
                  navpage = 1;

                  if (programmstatus & (1<<MANUELL))
						{
                     manuellcounter=0;
                     
                     
                     
						}
					}break;
						
					case 2://
					{
                  if (navline)
                  {
                     navline--;
                  }
						if (programmstatus & (1<<MANUELL))
						{
                     manuellcounter=0;
						}
						
					}break;
						
					case 3: //	Uhr aus
					{
						if (programmstatus & (1<<MANUELL))
						{
                     manuellcounter=0;
                     
                     
						}
					}break;
						
					case 4://
					{
                  if (navcol)
                  {
                     navcol--;
                  }
					}break;
						
					case 5://
					{
                  if (navpage<7)
                  {
                     //navpage++;
                  }
                  
                  switch (navpage)
                  {
                     case 1: // titel
                     {
                        blinkline=1;
                        blinkcol=6;
                        
                        blinkzeichen = modelnummer + '0';

                     }
                  }
                  
                 
 						manuellcounter=0;
                 
                  
					}break;
                  
               case 6://
               {
                 /*
                  if (navcol<7)
                     
                  {
                     
                      //(navigation_old[navpage][navline][navcol+1] < 0xFF))
                     for (i=navcol+1;i<8;i++) // naechsten cursorpunkt suchen
                     {
                        if (cursor[navpage][navline][navcol+1] < 0xFF)
                        {
                           navcol++;
                           blinkline=navline;
                           blinkcol=navigation_old[navpage][navline][navcol];
                           
                           blinkzeichen = 'x';

                        }
                     }
                     
                      displaypage(navpage, navline,navcol);
                  lcd_putnav();
                  }
                 */
               }break;
                  
                  
               case 7://home, in wenn 3* click aus default
               {
                  manuellcounter=0; // timeout zuruecksetzen
                  navpage --;
                  navline = 0;
                  navcol = 0;
                  if (navpage == 0)
                  {
                     blinkline=1;
                     blinkcol=6;
                     blinkzeichen = modelnummer + '0';

                  }
 
               }break;
                  
                  
               case 8://
               {
                  if ((navline<7))
                  {
                     navline++;
                  }
               }break;
                  
               case 9://set, out wenn auf home
               {
                  
               }break;
                  
                  
					case 10:// *Manuell einschalten
					{
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
                        navpage = 1; // Startpage von Setting anzeigen
                        //displaynav(navpage, navline,navcol);
                        //lcd_putnav();
                        
                     }
                     
                     
                  }
                  else if (navcounter == 0)
                  {
                     
                     
                     navpage--;
                     if (navpage==0) // Settings verlassen
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
                  //displaynav(navpage, navline,navcol);
                  
					}break;
                  
					case 11://
					{
						
					}break;
						
					case 12: // # Normalbetrieb einschalten
					{
                  /*
                  if (navpage)
                  {
                     navpage--;
                  }
                   */
                  if (navpage == 0)
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
            
            
            
            if (Tastenwert>5) // ca Minimalwert der Matrix
            {
               //			wdt_reset();
               /*
                0: Wochenplaninit
                1: IOW 8* 2 Bytes auf Bus laden
                2: Menu der aktuellen Ebene nach oben
                3: IOW 2 Bytes vom Bus in Reg laden
                4: Auf aktueller Ebene nach rechts (Heizung: Vortag lesen und anzeigen)
                5: Ebene tiefer
                6: Auf aktueller Ebene nach links (Heizung: Folgetag lesen und anzeigen)
                7:
                8: Menu der aktuellen Ebene nach unten
                9: DCF77 lesen
                
                12: Ebene höher
                */
               TastaturCount++;
               if (TastaturCount>=8)	//	Prellen
               {
                  
                  //err_clr_line(1);
                  //err_gotoxy(0,1);
                  //err_puts("ADC:\0");
                  //err_putint(Tastenwert);
                  
                  Taste=Tastenwahl(Tastenwert);
                  
                  //err_gotoxy(12,1);
                  //err_puts("T:\0");
                  //err_gotoxy(14,1);
                  //err_putint2(Taste);
                  /*
                   err_putint2(Taste);
                   //delay_ms(1200);
                   //err_clr_line(1);
                   */
                  TastaturCount=0;
                  Tastenwert=0x00;
                  //				uint8_t i=0;
                  
                  //			uint8_t inByte0=0;
                  //			uint8_t inByte1=0;
                  
                  uint8_t inBytes[4]={};
                  //			uint8_t pos=0;
                  //			lcd_gotoxy(12,0);
                  //			lcd_put_wochentag(DCF77buffer[5]);
                  //			lcd_gotoxy(15,0);
                  //			lcd_put_zeit(DCF77buffer[0],DCF77buffer[1]);
                  
                  //			Taste=0xff;
                  
                  
                  //Taste=4;
                  
                  switch (Taste)
                  {
                     case 0://WochenplanInit
                     {
                        break;
                        // Blinken auf C2
                        
                     }
                        break;
                        
                        
                     case 1:
                     {
                        //
                     }
                        break;
                        
                     case 2:											//	Menu vorwaertsschalten
                     {
                        lcd_cls();
                        /*
                         err_gotoxy(0,1);
                         err_puts("M:\0");
                         err_putint2(Menu_Ebene & 0xF0);
                         err_gotoxy(10,1);
                         err_puts("T:\0");
                         err_putint2(Taste);
                         //					delay_ms(1000);
                         */
                        switch (Menu_Ebene & 0xF0)					//	Bits 7-4, Menu_Ebene
                        {
                           case 0x00:								//	oberste Ebene, Raeume
                           {
                              //	(Kanal_Thema & 0xF0): Bit 7-4, Pos in Raum-Liste
                              if ((Kanal_Thema & 0xF0)<0x70)	//	Ende der Raum-Liste noch nicht erreicht
                                 
                              {
                                 Kanal_Thema += 0x10;			//	Naechster Raum
                              }
                              else
                              {
                                 lcd_gotoxy(14,0);
                                 lcd_puts(">>\0");				//Ende der Liste
                                 delay_ms(800);
                                 lcd_gotoxy(14,0);
                                 lcd_puts("  \0");
                              }
                              //err_cls();
                              //err_gotoxy(10,1);
                              //err_puts("Main:\0");
                              //err_puthex(Kanal_Thema);
                              //							delay_ms(800);
                              //lcd_cls();
                              /*
                               lcd_clr_line(0);
                               lcd_gotoxy(0,0);
                               //	RaumTable: Namen der Räume
                               strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(RaumTable[Kanal_Thema>>4])));//Bit 7 - 4
                               //	Raum anzeigen:
                               lcd_puts(titelbuffer);
                               delay_ms(1800);
                               */
                   //           displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde), Menu_Ebene);			//Anzeige aktualisieren
                              
                           }
                              break;
                              
                           case 0x10:								// erste Unterebene, Thema
                           {
                              if ((Kanal_Thema & 0x0F)<0x07)	//	(Kanal_Thema & 0x0F): Bit 3-0, Pos in Themen-Liste
                              {
                                 
                                 Kanal_Thema += 0x01;			//naechstes Thema
                              }
                              else
                              {
                                 lcd_gotoxy(13,0);
                                 lcd_puts(">>\0");				// Ende der Liste
                                 delay_ms(800);
                                 lcd_gotoxy(14,0);
                                 lcd_puts("  \0");
                              }
                              
                              //lcd_gotoxy(9,0);
                              //lcd_puthex(Kanal_Thema);
                              //lcd_gotoxy(12,0);
                              //lcd_puthex(Menu_Ebene);
                              
      //                        displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde),Menu_Ebene);			//Anzeige aktualisieren
                              
                           }
                              break;
                           case 	0x20:								// zweite Unterebene
                           {
                              
                           }break;
                        }//switch Menu_Ebene
                        
                     }
                        break;
                        
                     case 3:	//
                     {
                        
                     }break;
                        
                     case 4:	// Vortag
                     {
                        
                        //err_clr_line(0);
                        //err_gotoxy(0,0);
                        //err_puts("Taste 4\0");
                        //delay_ms(50);
                        //err_clr_line(0);
                        
                        
                        switch (Menu_Ebene & 0xF0)//Bits 7-4
                        {
                           case 0x00:	//oberste Ebene
                           {
                              //err_clr_line(0);
                              //err_puts("E0\0");
                              
                           }break;
                              
                           case 0x10: // erste Ebene
                           {
                              
                              
                           }break;
                              
                           case 0x20: // zweite Ebene, Plan
                           {
                              err_clr_line(0);
                              err_puts("T4 E2 \0");
                              err_puthex(Kanal_Thema);
                              err_putc(' ');
                              if ((Kanal_Thema & 0x0F)==0x00)	//Plan
                              {
                                 //err_puts(" Plan\0");
                                 
                                 //err_clr_line(1);
                                 
                                 //err_puts("Tag:\0");
                                 //err_putint(Anzeigekanal & 0x0F);
          //                       displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde), Menu_Ebene);	//	Anzeige aktualisieren
                                 
                                 //uint8_t tagblock[buffer_size];
                                 //uint8_t taglesenerfolg=TagLesen(EEPROM_WOCHENPLAN_ADRESSE, tagblock, 0, (Anzeigekanal));
                                 //TagZeigen(tagblock,(Menu_Ebene & 0x0F));
                                 //delay_ms(800);
                              }	// if Plan
                              else
                              {
                                 
                              }
    //                          err_putint1(Anzeigekanal);
                              delay_ms(800);
                           }break;	//	case 0x20
                              
                              
                        }//switch Menu_Ebene & 0xF0
                        
                     } break; // case Vortag
                        
                        
                     case 5:								// Ebene tiefer
                     {
                        //Taste=99;
                        lcd_clr_line(1);
                        //lcd_gotoxy(0,1);
                        //lcd_puts("Taste 5\0");
                        //delay_ms(200);
                        //lcd_clr_line(1);
                        //err_gotoxy(3,0);
                        //err_puthex(DCF77daten[5]);
                        Kanal_Thema &= 0xF0;							//	Bits 7 - 4 behalten, Bits 3-0 loeschen
                        Menu_Ebene &= 0xF0;							//	Bits 7 - 4 behalten, Bits 3-0 loeschen
                        if ((Menu_Ebene & 0xF0)<0x20)
                        {
                           switch (Menu_Ebene & 0xF0)
                           {
                              case 0x00: // erste Ebene, Thema
                              {
                                 Menu_Ebene = 0x10;
              //                   Anzeigekanal=(Zeit.wochentag) & 0x0F;
                              }break;
                                 
                              case 0x10:
                              {
                                 Menu_Ebene = 0x20;
                                 lcd_CGRAMInit_A();							//	Zeichen fuer Wochenplan setzen
                                 //Objekt_Wochentag =0x00;						//	Objekt 0 seetzen
                                 //Objekt_Wochentag = ((DCF77daten[5]) & 0x0F);		//	Wochentag setzen								//Objekt_Wochentag = 0x06;
                                 //err_gotoxy(5,0);
                                 //delay_ms(200);
    //                             Objekt_Wochentag = ((Zeit.wochentag) & 0x0F);		//	Wochentag setzen								//Objekt_Wochentag = 0x06;
                                 lcd_cls();
                                 
                              }break;
                                 
                                 
                                 
                                 
                           }//switch Menu_Ebene
                           
                           
                           
                           
                           //Kanal_Thema &= 0xF0;							//	Bits 7 - 4 behalten, Bits 3-0 loeschen
                           //Menu_Ebene &= 0xF0;							//	Bits 7 - 4 behalten, Bits 3-0 loeschen
                           
                           /*
                            lcd_clr_line(0);
                            lcd_clr_line(1);
                            lcd_clr_line(2);
                            lcd_clr_line(3);
                            
                            lcd_gotoxy(0,0);
                            strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(RaumTable[(Kanal_Thema>>4)])));
                            lcd_puts(titelbuffer);
                            
                            lcd_gotoxy(12,0);
                            lcd_put_wochentag(DCF77buffer[5]);
                            lcd_gotoxy(15,0);
                            lcd_put_zeit(DCF77buffer[0],DCF77buffer[1]);
                            
                            
                            //lcd_gotoxy(9,0);
                            //lcd_puthex(Kanal_Thema);
                            
                            //lcd_gotoxy(12,0);
                            //lcd_puthex(Menu_Ebene);
                            
                            //uint16_t aadr=(uint16_t)MenuTable[Kanal_Thema>>4];// Bit 7-4
                            //lcd_gotoxy(0,1);
                            //lcd_puthex(Kanal_Thema>>4);
                            //lcd_gotoxy(3,1);
                            //lcd_putint(adr);
                            */
                           
         //                  displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde), Menu_Ebene);	//	Anzeige aktualisieren
                           
                           /*
                            uint8_t adr=8*(Kanal_Thema>>4)+(Kanal_Thema & 0x0F); // Page: Bits 7-4 Zeile: Bits 3-0
                            strcpy_P(menubuffer, (PGM_P)pgm_read_word(&(P_MenuTable[adr])));//Bit 3 - 0	Untermenu im PROGMEM
                            //lcd_clr_line(1);
                            lcd_gotoxy(0,1);
                            //lcd_puthex((uint8_t)&adr);
                            lcd_puts(menubuffer);
                            */	
                           
                        }
                     }				break;
                        
                     case 6: // Folgetag
                     {
                        /*
                         err_clr_line(1);
                         err_gotoxy(0,1);
                         err_puts("Taste 6\0");
                         delay_ms(50);
                         err_clr_line(1);
                         */
                        
                        switch (Menu_Ebene & 0xF0)//Bits 7-4	
                        {
                           case 0x00:	//oberste Ebene
                              
                              err_clr_line(0);
                              err_puts("E0\0");
                              
                              break;
                              
                           case 0x10: // erste Ebene
                           {
                              
                              
                           }break;
                              
                           case 0x20: // zweite Ebene, Plan
                           {
                              
                              err_clr_line(0);
                              err_puts("T6 E2 \0");
                              err_puthex(Kanal_Thema);
                              err_putc(' ');
                              if ((Kanal_Thema & 0x0F)==0x00)	//Plan
                              {
                                 //err_puts(" Plan\0");
                                 if ((Anzeigekanal & 0x0F)< 6)	//	Nicht Sonntag
                                 {
                                    Anzeigekanal += 0x01;	//naechster Tag
                                 }
                                 else
                                 {
                                    lcd_gotoxy(3,0);
                                    lcd_puts("Es ist schon SO!\0");
                                    delay_ms(400);
                                    
                                 }
                                 //err_putint1(Anzeigekanal);
                                 //delay_ms(800);
                                 
                                 err_clr_line(1);
                                 err_puts("Tag:\0");
                                 err_putint(Anzeigekanal & 0x0F);
    //                             displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde), Menu_Ebene);	//	Anzeige aktualisieren
                                 
                                 //uint8_t tagblock[buffer_size];
                                 //uint8_t taglesenerfolg=TagLesen(EEPROM_WOCHENPLAN_ADRESSE, tagblock, 0, (Menu_Ebene & 0x0F));
                                 //TagZeigen(tagblock,(Menu_Ebene & 0x0F));
                              }	// if Plan
                              else
                              {
                                 
                              }
                              
                           }break;	// case 0x20
                              
                        } // Menu_Ebene & 0xF0
                        
                     } break; // case Folgetag
                        
                        //case 7:
                        
                        //	break;
                        
                        
                     case 8:												//Menu rueckwaertsschalten
                     {
                        //err_gotoxy(0,1);
                        //err_puts("M:\0");
                        //err_putint2(Menu_Ebene & 0xF0);
                        //err_gotoxy(10,1);
                        //err_puts("T:\0");
                        //err_putint2(Taste);
                        //delay_ms(1000);
                        
                        switch (Menu_Ebene & 0xF0)//Bits 7-4				oberste Ebene, Raeume
                        {
                           case 0x00:
                           {
                              
                              //					lcd_gotoxy(12,0);
                              //					lcd_puthex(Kanal_Thema);
                              //delay_ms(800);
                              //					pos=Kanal_Thema;
                              //					pos>>4;
                              
                              if ((Kanal_Thema & 0xF0)>0)
                              {
                                 
                                 Kanal_Thema -= 0x10;				//vorheriges Thema
                              }
                              else
                              {
                                 lcd_gotoxy(14,0);
                                 lcd_puts("<<\0");					//Ende der Liste
                                 delay_ms(800);
                                 lcd_gotoxy(14,0);
                                 lcd_puts("  \0");
                                 
                              }
                              //lcd_gotoxy(9,0);
                              //lcd_puthex(Kanal_Thema);
                              
                              //lcd_gotoxy(12,0);
                              //lcd_puthex(Menu_Ebene);
                              //							lcd_cls();	
                              /*
                               lcd_gotoxy(0,0);
                               lcd_clr_line(0);
                               //	RaumTable: Namen der Räume
                               strcpy_P(titelbuffer, (PGM_P)pgm_read_word(&(RaumTable[Kanal_Thema>>4])));//Bit 7 - 4
                               //	Raum anzeigen:
                               lcd_puts(titelbuffer);
                               */						
      //                        displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde), Menu_Ebene);			//Anzeige aktualisieren
                           }
                              break;
                              
                           case 0x10:									// erste Unterebene, Thema
                           {
                              if ((Kanal_Thema & 0x0F)>0x00)
                              {								
                                 Kanal_Thema -= 0x01;				//vorhergehendes Thema
                              }
                              else
                              {
                                 lcd_gotoxy(14,0);
                                 lcd_puts(">>\0");					// Ende der Liste
                                 delay_ms(800);
                                 lcd_gotoxy(14,0);
                                 lcd_puts("  \0");
                              }
                              
      //                        displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde), Menu_Ebene);				//Anzeige aktualisieren
                              
                           }
                              break;
                              
                        }// switch Menu_Ebene
                        
                     }
                        break;
                        
                        //case 9:
                        
                        
                        //	break;
                        
                        
                     case 12:// Ebene hoeher
                     {
                        //Taste=99;
                        
                        //lcd_clr_line(1);
                        //lcd_gotoxy(0,1);
                        //lcd_puts("Taste 12\0");
                        //delay_ms(100);
                        //lcd_clr_line(1);
                        switch (Menu_Ebene & 0xF0)
                        {
                           case 0x00:
                           {
                              
                           }break;
                              
                           case 0x10:
                           {
                              Menu_Ebene = 0x00;							//Ebene 0
                              //Menu_Ebene += (DCF77daten[5] & 0x0F);		//	Wochentag setzen
                              Kanal_Thema &=0xF0;
 //                             Anzeigekanal=(Zeit.wochentag) & 0x0F;
                              lcd_CGRAMInit_Titel();	//	Zeichen fuer Titel setzen
                              lcd_cls();
   //                           displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde), Menu_Ebene);	//	Anzeige aktualisieren
                              
                              
                           }break;
                           case 0x20:
                           {
                              Menu_Ebene = 0x10;							//	Ebene 1
                              Menu_Ebene &= 0xF0;							//	Bits 7 - 4 behalten, Bits 3-0 loeschen
                              //Menu_Ebene += (DCF77daten[5] & 0x0F);		//	Wochentag setzen
                              Kanal_Thema &=0xF0;
 //                             Anzeigekanal=(Zeit.wochentag) & 0x0F;
                              lcd_CGRAMInit_Titel();	//	Zeichen fuer Titel setzen
                              lcd_cls();
    //                          displayRaum(Kanal_Thema, Anzeigekanal, (Zeit.stunde>>4), Menu_Ebene);	//	Anzeige aktualisieren
                              
                              
                           }break;
                              
                              
                        }//switch MenuEbene
                        
                        
                     }break;
                        
                        
                  }//switch Taste
                  
               }
               //TastaturCount=0x00;
            }
//				delay_ms(400);
//				lcd_gotoxy(18,1);
//				lcd_puts("  ");		// Tastenanzeige loeschen

			}//if TastaturCount	
			
		}
	}
	
	
	return 0;
}
