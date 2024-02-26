/* EGR */

#include "hw_types.h"
#include "Conf_mod.h"
#include "delay_ms.h"
#include "EGR_Cape.h"
#include "GPIO.h"


int main() {

	//LED-Pins als GPIO konfigurieren
	PinMuxing(0x8d8, PULL_DISABLE, PULL_DOWN, GPIO_MODE); //LCD_DATA14
	PinMuxing(0x8d4, PULL_DISABLE, PULL_DOWN, GPIO_MODE); //LCD_DATA13
	PinMuxing(0x8d0, PULL_DISABLE, PULL_DOWN, GPIO_MODE); //LCD_DATA12


	//LED-Pins als Output konfigurieren
	EGR_GPIODirSet(GPIO_PORT2_PIN2_MODUL, GPIO_PORT2_PIN2, GPIO_OUTPUT); //LCD_DATA14
	EGR_GPIODirSet(GPIO_PORT2_PIN4_MODUL, GPIO_PORT2_PIN4, GPIO_OUTPUT); //LCD_DATA13
	EGR_GPIODirSet(GPIO_PORT2_PIN6_MODUL, GPIO_PORT2_PIN6, GPIO_OUTPUT); //LCD_DATA12

	//Taster auf Port 1 als GPIO konfigurieren
	PinMuxing(CONF_PORT1_PIN2, PULL_ENABLE, PULL_UP, GPIO_MODE); //Taster 1
	PinMuxing(CONF_PORT1_PIN4, PULL_ENABLE, PULL_UP, GPIO_MODE); //Taster 2

	//Taster als GPIO_DATAIN konfigurieren
	EGR_GPIODirSet(GPIO_PORT1_PIN2_MODUL,GPIO_PORT1_PIN2,GPIO_INPUT); //Taster 1
	EGR_GPIODirSet(GPIO_PORT1_PIN4_MODUL,GPIO_PORT1_PIN4,GPIO_INPUT); //Taster 2
	EGR_GPIODirSet(GPIO_PORT1_PIN6_MODUL,GPIO_PORT1_PIN6,GPIO_INPUT	); //Taster 3 -> NOTAUS TASTER

	//Funktion, die LED nacheinander eine halbe sekunde leuchten lässt
	void lauflicht(int speed){
		//LED 1
		EGR_PinWrite(GPIO_PORT2_PIN2_MODUL, GPIO_PORT2_PIN2, PIN_LOW);
		delay_ms(speed); //halbe Sekunde
		EGR_PinWrite(GPIO_PORT2_PIN2_MODUL, GPIO_PORT2_PIN2, PIN_HIGH);
		delay_ms(speed); //halbe Sekunde


		//LED 2
		EGR_PinWrite(GPIO_PORT2_PIN4_MODUL, GPIO_PORT2_PIN4, PIN_LOW);
		delay_ms(speed); //halbe Sekunde
		EGR_PinWrite(GPIO_PORT2_PIN4_MODUL, GPIO_PORT2_PIN4, PIN_HIGH);
		delay_ms(speed); //halbe Sekunde


		//LED 3
		EGR_PinWrite(GPIO_PORT2_PIN6_MODUL, GPIO_PORT2_PIN6, PIN_LOW);
		delay_ms(speed); //halbe Sekunde
		EGR_PinWrite(GPIO_PORT2_PIN6_MODUL, GPIO_PORT2_PIN6, PIN_HIGH);
		delay_ms(speed); //halbe Sekunde
	}

	int aktiv = 0; //Lauflicht aus am Anfang
	int pause = 500; //500ms am Anfang
	int counter = 0;

	while (1)
	{	//An- und Ausschalten
		if(!EGR_PinRead(GPIO_PORT1_PIN2_MODUL, GPIO_PORT1_PIN2)){
			aktiv = !aktiv;
		}

		//Geschwindigkeit des Lauflichts setzen
		if(!EGR_PinRead(GPIO_PORT1_PIN4_MODUL, GPIO_PORT1_PIN4)){
			counter++;
		}

		//Geschwindigkeit des Lauflichts setzen
		if(!EGR_PinRead(GPIO_PORT1_PIN6_MODUL, GPIO_PORT1_PIN6)){
			aktiv = !aktiv;
		}

		switch(counter){
		case 0: pause = 500; break;
		case 1: pause = 500/2; break;
		case 2: pause = 500/4; break;
		default: counter = 0; break;
		}

		if(aktiv){
			lauflicht(pause);
		}

		delay_ms(1500); //kurze Pause

	}
	return 0;
}
