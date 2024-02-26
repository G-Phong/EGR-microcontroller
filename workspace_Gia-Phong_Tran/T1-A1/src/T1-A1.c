/* EGR */

#include <hw_types.h>

int main() {

	//Control Module -> Modus 7 setzen an PIN "LCD_Data6" (Steckplatz 1) -> Modus 7 ist GPIO-Modul
	//Basisadresse: 0x44E10000 ist Adresse von Control-Module (aus Memory Map)
	//Offset-Adresse 0x8b8 -> führt auf die Addresse von lcd_data6 (aus Control Module Pinmuxing Register)
	HWREG(0x44E10000 + 0x8B8) &=~ ((1<<2)|(1<<1)|(1<<0)); //erst löschen, damit eventuelle Einser weg sind
	HWREG(0x44E10000 + 0x8B8) |= (7<<0); //"7" setzen als MUX-Mode -> GPIO Funktion (aus Übersicht über BeagleBone Pins)

	//Pin als Output einstellen
	//Basisadresse: GPIO2 0x481AC000 (aus Memory Map)
	//Offset-Adresse: 0x134 GPIO_OE (output enable) -> aus GPIO Register
	HWREG(0x481AC000 + 0x134) &=~ (1<<12);
	//12tes Bit setzen -> dann bezieht sich das auf LCD_DATA6 Pin (aus Übersicht über BeagleBone Pins)

	//Pin ausschalten
	//Basisadresse: GPIO2 0x481AC000 (aus Memory Map)
	//Offset-Adresse: 0x13C GPIO_DATAOUT -> aus GPIO Register
	HWREG(0x481AC000 + 0x13C) &=~ (1<<12);
	//12tes Bit setzen -> dann bezieht sich das auf LCD_DATA6 Pin (aus Übersicht über BeagleBone Pins)

	while (1)
	{
		/* Place main loop code here */
	}
	return 0;
}
