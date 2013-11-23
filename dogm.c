// ********************************************************************
// Demoprogramm
// Inhalt: LCD (DOG-M 081/162/163) mit (ST7036 Controller) ansteuern,
// 5V/3V Variante, mit selbst definierten Zeichen im RAM, SPI-Ansteuerung seriell
// LCD-Controller: ST7036 kompatibel
// Stand: 18.04.2008, Autor: Matthias Kahnt, Url: pro-51.eltra-tec.de
// SDCC-Version: M-IDE-51/SDCC 2.8.0
// AT89C51ED2: --code-loc 0x0000 --iram-size 256 --xram-loc 0x0000 --xram-size 0x0700
// AT89S8253:  --code-loc 0x0000 --iram-size 256
// ********************************************************************
// Für 8051-Microcontroller, z.B. AT89C51ED2 oder AT89S8253
// Frequenz: 11.0592 MHz
// Code-Speicherbedarf: ca. 1.3kByte, keine XRAM-Verwendung
// Das Programm verwendet nur 8051/52 Standard-SFRs und wurde
// mit dem AT89C51ED2 auf dem PRO-51 System getestet.
// ********************************************************************
// Die Programmfunktionen dürfen für private Nutzung verwendet und
// verändert werden. Für gewerbliche Nutzung ist die Zustimmung
// des Autors erforderlich. Die Nutzung erfolgt auf eigene Gefahr.
// Für eventuell entstehende Schäden wird keine Haftung übernommen.
// ********************************************************************

#include <string.h>      // Stingfunktionen
#include <avr/eeprom.h>

// Typendefinition
typedef unsigned char byte;
typedef unsigned int  word;
typedef unsigned long dword;

char menubuffer[16];
char titelbuffer[16];


// Hardware PINs, bitte anpassen
#define DOG_PORT       PORTD
#define DOG_DDR        DDRD   

#define LCD_ON    4
#define LCD_RS    5
#define LCD_SCL   6
#define LCD_DATA  7

//#define LCD_CSB (CS-Signal: wird im Bsp. nicht verwendet)

// Displaytyp, anpassen
#define SPALTEN  16
#define ZEILEN   3
#define SPANNUNG 5

extern volatile uint16_t loopcount1;
extern volatile uint8_t modelnummer;

extern volatile uint8_t            blinkline;
extern volatile uint8_t            blinkcol;
extern volatile char            blinkzeichen;

extern volatile uint8_t            laufsekunde;
extern volatile uint8_t            laufminute;

// Tabelle für 8 selbst definierte Sonderzeichen
char  sonderzeichen[] =
{
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F, // 00-07 Zeichen an Adresse 0
  0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0x1F, // 08-15 Zeichen an Adresse 1
  0x00,0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F, // 16-23 Zeichen an Adresse 2
  0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F, // 32-39 Zeichen an Adresse 3
  0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F, // 40-47 Zeichen an Adresse 4
  0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // 48-55 Zeichen an Adresse 5
  0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // 56-63 Zeichen an Adresse 6
  0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F  // 64-71 Zeichen an Adresse 7
};

const uint8_t  sonderzeichenA[] PROGMEM =
{
   0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03, // 00-07 Zeichen an Adresse 0 // Senkrechter Balken rechts
   0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, // 08-15 Zeichen an Adresse 1 // Senkrechter Balken links
   0x1B,0x1B,0x1B,0x1B,0x1B,0x1B,0x1B,0x1B, // 16-23 Zeichen an Adresse 2 // 2 senkrechte Balken
   0x00,0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F, // 32-39 Zeichen an Adresse 3
   0x00,0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F, // 40-47 Zeichen an Adresse 4
   0x00,0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // 48-55 Zeichen an Adresse 5
   0x00,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F, // 56-63 Zeichen an Adresse 6
   0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F  // 64-71 Zeichen an Adresse 7
};

const uint8_t  sonderzeichenB[] PROGMEM =
{
   0x00,0x00,0x10,0x08,0x00,0x04,0x02,0x01, // 00-07 Zeichen an Adresse 0 // Senkrechter Balken rechts
   0x00,0x00,0x00,0x10,0x08,0x04,0x02,0x01, // 08-15 Zeichen an Adresse 1 // Senkrechter Balken links
   0x00,0x00,0x00,0x00,0x10,0x08,0x06,0x01, // 16-23 Zeichen an Adresse 2 // 2 senkrechte Balken
   0x00,0x00,0x00,0x00,0x00,0x10,0x0C,0x03, // 32-39 Zeichen an Adresse 3
   0x01,0x00,0x02,0x00,0x04,0x08,0x00,0x10, // 40-47 Zeichen an Adresse 4
   0x00,0x00,0x00,0x01,0x02,0x04,0x08,0x10, // 48-55 Zeichen an Adresse 5
   0x00,0x00,0x00,0x00,0x01,0x02,0x0C,0x10, // 56-63 Zeichen an Adresse 6
   0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x18  // 64-71 Zeichen an Adresse 7
};

const uint8_t  sonderzeichenC[] PROGMEM =
{
   0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 00-07 Zeichen an Adresse 0 // Senkrechter Balken rechts
   0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00, // 08-15 Zeichen an Adresse 1 // Senkrechter Balken links
   0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00, // 16-23 Zeichen an Adresse 2 // 2 senkrechte Balken
   0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00, // 32-39 Zeichen an Adresse 3
   0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00, // 40-47 Zeichen an Adresse 4
   0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x00, // 48-55 Zeichen an Adresse 5
   0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00, // 56-63 Zeichen an Adresse 6
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07  // 64-71 Zeichen an Adresse 7
};

const uint8_t  sonderzeichenD[]  PROGMEM =
{
   0x1C,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // 00-07 Zeichen an Adresse 0 // Senkrechter Balken rechts
   0x00,0x1C,0x00,0x00,0x00,0x00,0x00,0x00, // 08-15 Zeichen an Adresse 1 // Senkrechter Balken links
   0x00,0x00,0x1C,0x00,0x00,0x00,0x00,0x00, // 16-23 Zeichen an Adresse 2 // 2 senkrechte Balken
   0x00,0x00,0x00,0x1C,0x00,0x00,0x00,0x00, // 32-39 Zeichen an Adresse 3
   0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00, // 40-47 Zeichen an Adresse 4
   0x00,0x00,0x00,0x00,0x00,0x07,0x00,0x00, // 48-55 Zeichen an Adresse 5
   0x00,0x00,0x00,0x00,0x00,0x00,0x07,0x00, // 56-63 Zeichen an Adresse 6
   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07  // 64-71 Zeichen an Adresse 7
};





// Tabelle der DOG-M Initialisierung
char lcd_init_code[] = {
  0x31,0x1C,0x51,0x6A,0x74, // 00-04 DOG-M 081 5V
  0x31,0x14,0x55,0x6D,0x7C, // 05-09 DOG-M 081 3V
  0x39,0x1C,0x52,0x69,0x74, // 10-14 DOG-M 162 5V
  0x39,0x14,0x55,0x6D,0x78, // 15-19 DOG-M 162 3V
  0x39,0x1D,0x50,0x6C,0x77, // 20-24 DOG-M 163 5V
  0x39,0x15,0x55,0x6E,0x72  // 25-29 DOG-M 163 3V
};

// Tabelle der DOG-M Zeilenanfangsadressen
char  lcd_zeilen_adresse[] =
{
  0x00,0x00,0x00, // 00-02 DOG-M 081
  0x00,0x40,0x40, // 03-05 DOG-M 162
  0x00,0x10,0x20  // 06-08 DOG-M 163
};

// ********************************************************************
// Verzögerungsschleife für kurze Zeit
// Zeit: Wartezeit in [zeit]
//       1 = 20us
//       2 = 28us
//       3 = 36us u.s.w. (gilt für 11.0952MHz)
//     255 = ca. 2ms
// ********************************************************************

// ********************************************************************
// Verzögerungsschleife für lange Zeit
// Zeit: Wartezeit in [zeit]
//       1 = ca. 10ms
//       2 = ca. 20ms
//       3 = ca. 30ms u.s.w.
//     255 = ca. 2,5s (gilt für 11.0952MHz)
// ********************************************************************
void delay(uint8_t zeit)
{
uint8_t zaehler;
  for (zaehler = zeit; zaehler; zaehler--)
  {
    _delay_us(255);  // dauert ca. 2ms
    _delay_us(255);  // dauert ca. 2ms
    _delay_us(255);  // dauert ca. 2ms
    _delay_us(255);  // dauert ca. 2ms
    _delay_us(255);  // dauert ca. 2ms
  }
}


// ***********************************************************************
// Grundinitialisierung des LCD-Moduls in SPI-Mode (seriell)
// ***********************************************************************
#pragma save
#pragma disable_warning 126
void init_lcd(void)
{
}
#pragma restore



void displaypage(uint8_t page,uint8_t line,uint8_t col)
{}

