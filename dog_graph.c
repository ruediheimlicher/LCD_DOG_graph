//
//  dog_graph.c
//  DOG_LCD
//
//  Created by Ruedi Heimlicher on 20.11.2013.
//
//

#include <stdio.h>

#include "ea_font.h"


#define DEFAULT_CONTRAST    16  // Der Kontrast beim Einschalten
#define FONTWIDTH   font[(*pChain)-32][0]
#define INVERSE   1
#define NOINVERSE   0


volatile uint8_t A0 = 0;
volatile uint8_t actContrast=DEFAULT_CONTRAST;

void clearScreen(void);


// ***********************************************************************
// Schreiben eines Zeichens an das DOGM-Modul
// seriell Soft-SPI Mode, 3/4 Draht Interface
// Übergabe: lcd_byte : Auszugebendes Zeichen/Steuerzeichen
//           lcd_mode : 0 - Daten
//                      1 - Steuerzeichen
// ***********************************************************************
void write_dogm(uint8_t lcd_byte, uint8_t lcd_mode)
{
   uint8_t stelle;
   DOG_PORT &= ~(1<<DOG_CS );
   // Pause bevor nächstes Zeichen gesendet werden darf
   _delay_us(5); // Pause mind. 26,3us
   // LCD_CSB = 0;
   if (lcd_mode)
   {
      DOG_PORT &= ~(1<<DOG_A0 ); // Steuerzeichen
   }
   else
   {
      
      DOG_PORT |=  (1<<DOG_A0 );        // Daten
   }
   // Byte senden seriell senden, H-Bit zuerst (Bit 8 7 6 5 4 3 2 1 0)
   for ( stelle = 0x80; stelle; stelle >>= 1 )
   {
      //LCD_DATA = lcd_byte & stelle;
      if (lcd_byte & stelle)
      {
         DOG_PORT |=  (1<< DOG_DATA);// bit ist 1
      }
      else
      {
         DOG_PORT &=  ~(1<< DOG_DATA); // bit ist 0
      }
      
      DOG_PORT &= ~(1<< DOG_SCL);
      DOG_PORT |=  (1<< DOG_SCL);
   }
   _delay_us(5);
   DOG_PORT |= (1<<DOG_CS );
   // LCD_CSB = 1;
}

/*
 * Initialisierung des EA DOGL Displays mit dem SPI Bus
 http://pic-projekte.de/wiki/index.php?title=EA_DOGL#Datei_dogl.c
 */
void initDOGL(void)
{
   A0 = 0;         // Die folgenden Bytes sind Befehle
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
   
   clearScreen();
   
}

/*
 * Löscht den Inhalt des Displays und springt danach wieder
 * auf die Position 0,0 (oben links)
 */
void clearScreen(void)
{
   unsigned int k,adr;
   
   /*Displayinhalt löschen*/
   for(adr=0; adr<8; adr++)
   {
      A0=0;
      write_dogm(0xB0 + adr,A0);
      write_dogm(0x10,A0);
      write_dogm(0x00,A0);
      
      A0=1;
      for(k=0; k<128; k++)
         write_dogm(0x00,A0);
      A0=0;
   }
   
   write_dogm(0xB0,A0);  // Sprung zu: Zeile 0, Spalte 0
   write_dogm(0x10,A0);
   write_dogm(0x00,A0);
}

/*
 * Diese Funktion gibt eine Zeichenkette auf dem Bildschirm aus.
 * Mit einer Angabe der Adresse, kann dies an jedem beliebigen Ort auf
 * dem Display stattfinden. Des Weiteren kann zwischen normaler und
 * invertierter Darstellung gewählt werden.
 *
 * page:        Adresse - Zeile (0..7)
 * column:      Adresse - Spalte (0..127)
 * inverse:     Invertierte Darstellung wenn true sonst normal
 * *pChain:     Die Zeichnkette ansich, welche geschrieben werden soll
 */


void charChain(uint8_t page, uint8_t column, uint8_t  inverse, const uint8_t *pChain)
{
   unsigned char k, count=0;
   
   setAddress(page,column);
   A0=1;   // Daten zum DOGL
   
   if(inverse)
      write_dogm(font[0][1],A0);
   
   while(*pChain)
   {
      if( (FONTWIDTH + column + count) > 127)
      {
         count=column=0;
         setAddress(page+1,0);
         A0=1;
      }
      for(k=1; k <= FONTWIDTH; k++)
      {
         if(inverse)
            write_dogm(font[(*pChain)-32][k],A0);
         else
            write_dogm(font[(*pChain)-32][k],A0);
         count++;
      }
      
      if(inverse)
         write_dogm(~font[0][1],A0);
      else
         write_dogm(font[0][1],A0);
      count++;
      pChain++;
   }
}

/*
 * Auswahl einer Adresse. Diese Funktion wird in der Regel nicht vom
 * Anwender selbst aufgerufen, sondern nur durch die Funktionen dieser
 * C-Datei.
 *
 * page:    Addresse - Zeile (0..7)
 * column:  Addresse - Spalte (0..127)
 */
void setAddress(uint8_t page, uint8_t column)
{
   if( page<8 && column<128 )
   {
      A0=0;
      write_dogm(0xB0 + page,A0);
      write_dogm(0x10 + ((column&0xF0)>>4) ,A0);
      write_dogm(0x00 +  (column&0x0F) ,A0);
   }
}



//=============================================================================
//keeping track of current position in ram - necessary for big fonts & bitmaps
//=============================================================================

uint8_t lcd_current_page = 0;
uint8_t lcd_current_column = 0;





