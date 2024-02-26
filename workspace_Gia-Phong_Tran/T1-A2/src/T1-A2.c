/* EGR */

#include <hw_types.h>

int main() {

	//Hier müssen die Pins der verwendeten Ports des EGR-Capes konfiguriert werden -> in der while-Schleife kommt dann die Taster-LED-Logik

	//Control Module
	//Modus 7 setzen an PIN LCD Data 6 -> gpio2_12
	HWREG(0x44E10000+0x8b8) &=~ ((1<<2)|(1<<1)|(1<<0));
	HWREG(0x44E10000+0x8b8) |= 7<<0;

	//Modus 7 setzen an PIN LCD Data 4 -> gpio2_10
	HWREG(0x44E10000+0x8b0) &=~ ((1<<2)|(1<<1)|(1<<0));
	HWREG(0x44E10000+0x8B0) |= 7<<0;

	//Modus 7 setzen an PIN LCD Data 2 -> gpio2_8
	HWREG(0x44E10000+0x8a8)  &=~ ((1<<2)|(1<<1)|(1<<0));
	HWREG(0x44E10000+0x8a8) |= 7<<0;

	//an dieser Stelle sind die Pins als GPIO konfiguriert! Jetzt müssen sie als Input konfiguriert werden

	//Pins als INPUT einstellen -> Port1 -> GPIO2-Modul Basisadresse ist 0x481AC000 -> Offsetadresse führt auf GPIO-Register
	HWREG(0x481AC000+0x134) |= (1<<12); //Bit 12 dieses Output-Enable Registers auf 1 setzen
										// -> "1" bedeutet "es ist KEIN output enabled"
	//Bit 12 dieses GPIO_DATAIN 0x138 (Offset) auf 0 setzen -> damit ist das PIN als Data-Input konfiguriert
	//-> dadurch kann man Taster einlesen
	HWREG(0x481AC000+0x138) &=~ (1<<12);

	//Pullup aktivieren, indem im Konfigurationsregister Bit 3 und 4 auf "0" und "1" gesetzt wird
	HWREG(0x44E10000+0x8b8) &=~ (1<<3); //3tes Bit auf "0" setzen -> aktiviert Pullup/Pulldown
	HWREG(0x44E10000+0x8b8) |= (1<<4); //4tes Bit auf "1" setzen -> wählt Pullup

	//das gleiche für PIN "LCD_DATA4" und "LCD_DATA2"
	HWREG(0x481AC000+0x134) |= (1<<10); //Bit 10 dieses Output-Enable Registers auf 1 setzen (für PIN LCD_DATA4)
	HWREG(0x481AC000+0x134) |= (1<<8); //Bit 8 dieses Output-Enable Registers auf 1 setzen (für PIN LCD_DATA2)

	//Pullup aktivieren, indem im Konfigurationsregister Bit 3 und 4 auf "0" und "1" gesetzt wird
	HWREG(0x44E10000+0x8b0) &=~ (1<<3); //3tes Bit auf "0" setzen -> aktiviert Pullup/Pulldown (für PIN LCD_DATA4)
	HWREG(0x44E10000+0x8b0) |= (1<<4); //4tes Bit auf "1" setzen -> wählt Pullup (für PIN LCD_DATA4)

	HWREG(0x44E10000+0x8a8) &=~ (1<<3); //3tes Bit auf "0" setzen -> aktiviert Pullup/Pulldown (für PIN LCD_DATA2)
	HWREG(0x44E10000+0x8a8) |= (1<<4); //4tes Bit auf "1" setzen -> wählt Pullup (für PIN LCD_DATA2)

	//__________________________Hier kommt die Konfiguration von Steckplatz 2: die LEDs __________________________

	//LCD_DATA14 -> gpio0_10 -> Control Module -> GPIO Funktion setzen
	HWREG(0x44E10000 + 0x8D8) &=~ ((1<<0)|(1<<1)|(1<<2));
	HWREG(0x44E10000 + 0x8D8) |= 7<<0;

	//LCD_DATA13 -> gpio0_9
	HWREG(0x44E10000 + 0x8D4) &=~ ((1<<0)|(1<<1)|(1<<2));
	HWREG(0x44E10000 + 0x8D4) |= 7<<0;

	//LCD_DATA12 -> gpio0_8
	HWREG(0x44E10000 + 0x8D0) &=~ ((1<<0)|(1<<1)|(1<<2));
	HWREG(0x44E10000 + 0x8D0) |= 7<<0;

	//Output konfigurieren von GPIO Module 0
	//LCD_DATA14 -> gpio0_10
	HWREG(0x44E07000 + 0x134) &=~ (1<<10);

	//LCD_DATA13 -> gpio0_9
	HWREG(0x44E07000 + 0x134) &=~ (1<<9);

	//LCD_DATA12 -> gpio0_8
	HWREG(0x44E07000 + 0x134) &=~ (1<<8);

	while (1)
	{
		//Überprüfen welcher Taster gedrückt ist -> entsprechendes Register der LEDs setzen

		if(HWREG(0x481ac000 + 0x138)&(1<<12)) //prüfen ob Bit 12 gesetzt in 138h (GPIO_DATAIN) -> Verunden mit "...1000000000000"
			//in C wird die Zahl die drinnesteht zu "True" ausgewertet, wenn irgendwo eine 1 auftaucht (logisch)
				{
					HWREG(0x44E07000 + 0x13c) |= (1<<10); //Led an, PIN abschalten
				}
		else
				{
					HWREG(0x44E07000 + 0x13c) &=~ (1<<10); //Led aus
				}

		if(HWREG(0x481ac000 + 0x138)&(1<<10)) //prüfen ob Bit 10 gesetzt
				{
					HWREG(0x44E07000 + 0x13c) |= (1<<9); //Led an
				}
		else
				{
					HWREG(0x44E07000 + 0x13c) &=~ (1<<9); //Led aus
				}

		//Hilfsvariable
		unsigned long int a = HWREG(0x481ac000 + 0x138); //unsigned um Vorzeichenfehler auszuschließen -> long damit wir 32 bit haben und nicht 16 wie bei int
		a >>= 8;
		a <<= 31;

		if(!a) //prüfen ob Bit 8 gesetzt -> wenn Taster gedrückt dann liegt "LOW" an -> also wenn in der Bedingung "1" rauskommt dann liegt HIGH an -> "Logikvertauschung"
				{
					HWREG(0x44E07000 + 0x13c) &=~ (1<<8); //Led an
				}
		else
				{
					HWREG(0x44E07000 + 0x13c) |= (1<<8); //Led aus
				}


	}
	return 0;
}
