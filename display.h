/*----------------------------------------------------------------------------
 Copyright:
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        21.11.2009
 Description:    EA DOG (M/L)128-6
------------------------------------------------------------------------------*/

#ifndef _DISPLAY_H
	#define _DISPLAY_H

// nicht von font.h????
#define FONT_WIDTH 	6
//#define FONT_HEIGHT 8  //8, 16, 32, 64, 128


	#define DISPLAY_OFFSET		2	
	
	#define LINE1				5			
	
	#define LINE2				1



// Hardware PINs, bitte anpassen
#define DOG_PORT       PORTB
#define DOG_DDR        DDRB


#define DOG_A0       0
#define DOG_RST      1

#define DOG_CS       2

#define DOG_SCL      3
#define DOG_DATA     5


// SOFT-SPI defines

#define SOFT_SPI_PORT   PORTD
#define SOFT_SPI_DDR    DDRD

#define A0_HI        SOFT_SPI_PORT |= (1<<DOG_A0)
#define A0_LO        SOFT_SPI_PORT &= ~(1<<DOG_A0)

#define RST_HI        SOFT_SPI_PORT |= (1<<DOG_RST)
#define RST_LO        SOFT_SPI_PORT &= ~(1<<DOG_RST)


#define CS_HI        SOFT_SPI_PORT |= (1<<DOG_CS)
#define CS_LO        SOFT_SPI_PORT &= ~(1<<DOG_CS)

#define SCL_HI       SOFT_SPI_PORT |= (1<<DOG_SCL)
#define SCL_LO       SOFT_SPI_PORT &= ~(1<<DOG_SCL)

#define DATA_HI      SOFT_SPI_PORT |= (1<<DOG_DATA)
#define DATA_LO      SOFT_SPI_PORT &= ~(1<<DOG_DATA)





//	#if defined (__AVR_ATmega8__)|| defined(__AVR_ATmega168__)
		//A0 Port


		#define PORT_A0  			PORTB
		#define DDR_A0   			DDRB
		#define PIN_A0   			0

		//Reset Port
		#define PORT_RST 			PORTB
		#define DDR_RST  			DDRB
		#define PIN_RST  			1

		//SPI Port
		#define SPI_SS				2	
		#define SPI_DO				3
		#define SPI_DI				4
		#define SPI_Clock			5

		//Hintergrundbeleuchtung PWM
		#define BRIGHTNESS_PWM_DDR	DDRD
		#define BRIGHTNESS_PWM_PORT	PORTD
		#define BRIGHTNESS_PWM_PIN	7
//	#endif
/*
	#if defined (__AVR_ATmega32__) || defined(__AVR_ATmega644__) || defined (__AVR_ATmega644P__)
		//A0 Port
		#define PORT_A0  			PORTB
		#define DDR_A0   			DDRB
		#define PIN_A0   			2

		//Reset Port
		#define PORT_RST 			PORTB
		#define DDR_RST  			DDRB
		#define PIN_RST  			3

		//SPI Port
		#define SPI_SS				4	
		#define SPI_DO				5
		#define SPI_DI				6			
		#define SPI_Clock			7			

		//Hintergrundbeleuchtung PWM
		#define BRIGHTNESS_PWM_DDR	DDRD
		#define BRIGHTNESS_PWM_PORT	PORTD
		#define BRIGHTNESS_PWM_PIN	7
	#endif
*/

// http://www.cczwei-forum.de/cc2/thread.php?postid=49733#post49733
#define DISPOFF      0xAE
#define DISPON       0xAF
#define DISPSTART    0x40
#define PAGEADR      0xB0
#define COLADRL      0x00
#define COLADRH      0x10
#define ADCNORMAL    0xA0
#define ADCREVERSE   0xA1
#define COMNORMAL    0xC0
#define COMREVERSE   0xC8
#define DISPNORMAL   0xA6
#define DISPREVERSE  0xA7
#define LCDBIAS9     0xA2
#define LCDBIAS7     0xA3
#define RESET        0xE2
#define SETPOWERCTRL 0x28
#define REGRESISTOR  0x20
#define SETCONTRAST  0x81
#define STATINDMODE  0xAC
#define BOOSTERRATIO 0xF8



char menubuffer[20];
char titelbuffer[20];


volatile uint8_t cursortab[4] = {0,32,64,96};
volatile uint8_t itemtab[4] = {8,48,72,104};


	void display_init(void);
	void display_mem(PGM_P pointer);
	void display_clear(void);
	void display_go_to (unsigned char, unsigned char);
	void display_back_char (void);
	
	void display_write_char(unsigned char);
	void display_write_str(char *str);
void display_write_inv_str(char *str);

	void display_write_P (const char *Buffer,...);

void display_write_int(uint8_t zahl);
void display_inverse(uint8_t inv);
uint8_t spi_out(uint8_t dataout);
void display_write_symbol(PGM_P symbol);

void display_pfeilvollrechts(uint8_t col, uint8_t page);
void display_write_min_sek(uint16_t rawsekunde);
void display_writeprop_str(uint8_t page, uint8_t column, uint8_t inverse, const uint8_t *pChain);

void display_write_prop_str(uint8_t page, uint8_t column, uint8_t inverse, const uint8_t *pChain);

void display_write_spannungbis10(uint8_t rawspannung); // eine Dezimale
void r_uitoa8(int8_t zahl, char* string);

uint8_t update_screen(void);



	#define display_write(format, args...)   display_write_P(PSTR(format) , ## args)

	volatile unsigned char char_x,char_y,char_height_mul,char_width_mul;

	//write to lc-display command or data register
	#define CMD		1
	#define DATA	0
	
	//Befehlstabelle EA DOGM128-6 Seite 5
	// (1) Display ON/OFF
	#define DISPLAY_ON       			0xAF  //LCD_DISPLAY_ON
	#define DISPLAY_OFF      			0xAE  //LCD_DISPLAY_OFF

	// (2) Display start line set
	

	// (3) Page address set
	#define DISPLAY_PAGE_ADDRESS		0xB0

	// (4) Column address set upper bit
	#define DISPLAY_COL_ADDRESS_MSB		0x10
	// (4) Column address set lower bit
	#define DISPLAY_COL_ADDRESS_LSB		0x00  

	// (5) Status read (doesn't work in SPI mode)

	// (6) Display data write
	
	// (7) Display data read (doesn't work in SPI mode)
	
	// (8) ADC select
	#define DISPLAY_BOTTOMVIEW			0xA0  
	#define DISPLAY_TOPVIEW				0xA1  

	// (9) Display normale/reverse
	#define DISPLAY_NORMAL   			0xA6
	#define DISPLAY_REVERSE				0xA7

	// (10) Display all points ON/OFF
	#define DISPLAY_SHOW_NORMAL			0xA4
	#define DISPLAY_SHOW_ALL_POINTS		0xA5

	// (11) LCD bias set
	#define DISPLAY_BIAS_1_9			0xA2
	#define DISPLAY_BIAS_1_7			0xA3

	// (12) Read-modify-write (doesn't work in SPI mode)

	// (13) End clear read/modify/write (doesn't work in SPI mode)

	// (14) RESET
	#define DISPLAY_RESET_CMD			0xE2

	// (15) Common output mode select
	#define DISPLAY_SCAN_DIR_NORMAL		0xC0  
	#define DISPLAY_SCAN_DIR_REVERSE	0xC8  

	// (16) Power control set
	#define DISPLAY_POWER_CONTROL		0x28
	#define DISPLAY_POWER_LOW_POWER		0x2F
	#define DISPLAY_POWER_WIDE_RANGE	0x2F 
	#define DISPLAY_POWER_LOW_VOLTAGE	0x2B 

	// (17) V0 voltage regulator internal resistor ratio set
	#define DISPLAY_VOLTAGE          	0x20

	// (18) Electronic volume mode set
	#define DISPLAY_VOLUME_MODE_1    	0x81
	// (18) Register
	#define DISPLAY_VOLUME_MODE_2    	0x00

	// (19) Static indicator ON/OFF
	#define DISPLAY_INDICATOR_ON       	0xAD  
	#define DISPLAY_INDICATOR_OFF      	0xAC  
	// (19) Static indicator register set
	#define DISPLAY_INDICATOR_MODE_OFF 	0x00
	#define DISPLAY_INDICATOR_MODE_1HZ 	0x01
	#define DISPLAY_INDICATOR_MODE_2HZ 	0x10
	#define DISPLAY_INDICATOR_MODE_ON  	0x11

	// (20) Booster ratio set
	#define DISPLAY_BOOSTER_SET      	0xF8
	#define DISPLAY_BOOSTER_234      	0x00
	#define DISPLAY_BOOSTER_5        	0x01
	#define DISPLAY_BOOSTER_6        	0x03

	// (21) Power save

	// (22) NOP
	#define LCD_NOP              		0xE3
	
	// (23) Test Command for IC test. Do not use this command.
	


#endif //_DISPLAY_H





