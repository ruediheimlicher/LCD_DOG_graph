//
//  text.h
//  DOG_LCD
//
//  Created by Ruedi Heimlicher on 22.11.2013.
//
//

#ifndef DOG_LCD_text_h
#define DOG_LCD_text_h

// Homescreen
const char titel0[] PROGMEM = "RC Home\0";
const char titel1[] PROGMEM = "ON-Zeit:\0";
const char titel2[] PROGMEM = "Stoppuhr\0";
const char titel3[] PROGMEM = "Motorzeit\0";
const char titel4[] PROGMEM = "Menu";
const char titel5[] PROGMEM = "Set";
const char titel6[] PROGMEM = "Akku\0";
const char titel7[] PROGMEM = "D\0";

PGM_P const TitelTable[] PROGMEM = {titel0, titel1, titel2, titel3, titel4, titel5, titel6, titel7};

// Modelle
const char model0[] PROGMEM = "E-Segler A  ";
const char model1[] PROGMEM = "E-Segler B  ";
const char model2[] PROGMEM = "Hangsegler A\0";
const char model3[] PROGMEM = "Motor A     \0";
const char model4[] PROGMEM = "Motor B     \0";
const char model5[] PROGMEM = "AA\0        ";
const char model6[] PROGMEM = "BB\0        ";
const char model7[] PROGMEM = "CC\0        ";

PGM_P const ModelTable[] PROGMEM = {model0, model1, model2, model3, model4, model5, model6, model7};





// Settingscreen
const char menutitel[] PROGMEM = "Settings";
const char model[] PROGMEM = "Modell";
const char setting[] PROGMEM = "Set";
const char kanal[] PROGMEM = "Kanal";

const char mix[] PROGMEM = "Mix";
PGM_P const SettingTable[] PROGMEM = {menutitel, model, setting, kanal,  mix};


// Kanalscreen
const char kanaltitel[] PROGMEM = "Kan:";
const char richtung[] PROGMEM = "Ri:";
const char level[] PROGMEM = "Level";
const char expo[] PROGMEM = "Expo";

const char seitea[] PROGMEM = "A:";
const char seiteb[] PROGMEM = "B:";
const char kanaltyp[] PROGMEM = "Typ:";

PGM_P const KanalTable[] PROGMEM = {kanaltitel, richtung, level, expo, seitea, seiteb,kanaltyp};


// Kanaltyp
const char pitchtyp[] PROGMEM = "Pitch";
const char schiebertyp[] PROGMEM = "Schieber";
const char schaltertyp[] PROGMEM = "Schalter";

PGM_P const KanalTypTable[] PROGMEM = {pitchtyp,schiebertyp,schaltertyp};


// Mix
const char mixtitel[] PROGMEM = "Mix";
const char vmix[] PROGMEM = "V-Mix";
const char butterfly[] PROGMEM = "B-fly";
PGM_P const MixTable[] PROGMEM = {mixtitel,vmix,butterfly};


// Funktion

const char funktion0[] PROGMEM = "Seite  \0";
const char funktion1[] PROGMEM = "Hoehe  \0";
const char funktion2[] PROGMEM = "Quer   \0";
const char funktion3[] PROGMEM = "Motor  \0";
const char funktion4[] PROGMEM = "Quer L\0";
const char funktion5[] PROGMEM = "Quer R\0";
const char funktion6[] PROGMEM = "Lande  \0";
const char funktion7[] PROGMEM = "Aux    \0";

PGM_P const FunktionTable[] PROGMEM = {funktion0, funktion1, funktion2, funktion3, funktion4, funktion5, funktion6, funktion7};




#endif


