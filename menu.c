//
//  menu.c
//  Tastenblinky
//
//  Created by Ruedi Heimlicher on 27.10.2013.
//
//

#include <stdio.h>
#include <avr/pgmspace.h>



const char DefaultTitel[]  PROGMEM ="RC-Sender";
const char Laufzeit[]  PROGMEM ="Zeit";
const char Einschaltzeit[]  PROGMEM ="ON";
const char Stopuhr[]  PROGMEM ="Stop";
const char Batterie[]  PROGMEM ="Bat";
PGM_P const DefaultTable[] PROGMEM ={DefaultTitel, Laufzeit,Einschaltzeit ,Stopuhr,Batterie};



const char Titel[]  PROGMEM ="Einstellungen";
const char Modelwahl[]  PROGMEM ="Model:";
const char Kanalwahl[]  PROGMEM ="Kanal";
const char Mixwahl[]  PROGMEM ="Mix";
PGM_P const TitelTable[] PROGMEM ={Titel, Modelwahl, Kanalwahl, Mixwahl};



// fenster:00 zeile:0 textposition:00 datapos:00 | FF: Menu, weiter zu Fenster


const char Home0[]  PROGMEM ="0100000Einstellungen";
const char Home1[]  PROGMEM ="0110006Model:";
const char Home2[]  PROGMEM ="01200FFKanal";
const char Home3[]  PROGMEM ="01207FFMix";
PGM_P const HomeTable[] PROGMEM ={Home0, Home1, Home2, Home3};




const char Kanal0[] PROGMEM = "0200002Nr";
const char Kanal1[] PROGMEM = "0200407Fkt";
const char Kanal2[] PROGMEM = "0211315Ri";
const char Kanal3[] PROGMEM = "0210005Mode";
const char Kanal4[] PROGMEM = "02300FFLevel";
const char Kanal5[] PROGMEM = "02307FFExpo";
PGM_P const KanalTable[] PROGMEM = {Kanal0, Kanal1, Kanal2, Kanal3, Kanal4, Kanal5};




const char Raum0[] PROGMEM = "Heizung\0";
const char Raum1[] PROGMEM = "Werkstatt\0";
const char Raum2[] PROGMEM = "WoZi\0";
const char Raum3[] PROGMEM = "Buero\0";
const char Raum4[] PROGMEM = "Labor\0";
const char Raum5[] PROGMEM = "OG 1\0";
const char Raum6[] PROGMEM = "OG 2\0";
const char Raum7[] PROGMEM = "Estrich\0";
PGM_P const RaumTable[] PROGMEM = {Raum0, Raum1, Raum2, Raum3, Raum4, Raum5, Raum6, Raum7};

/*
char Titel[] PROGMEM = "HomeCentral\0";
char Name[] PROGMEM = "Ruedi Heimlicher\0";
char Adresse[] PROGMEM = "Falkenstrasse 20\0";
char Ort[] PROGMEM = "8630 Rueti\0";
PGM_P P_StartTable[] PROGMEM = {Titel, Name, Adresse, Ort};
*/