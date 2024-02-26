/* EGR */

//TODO: Disziplin 1 -> Fahren entlang einer Linie

#include <hw_types.h>
#include "IRSensors.h"
#include "i2c.h"
#include "delay_ms.h"
#include "uartStdio.h"
#include "stdio.h"
#include "Motorsteuerung.h"


void spurVorgabeMotor(int richtung){

	if(richtung > 2 || richtung < 0){
		UARTprintf("Richtung ungültig! \n");
		return;
	}

	switch(richtung) {
		case 0: motorAnsteuernMitLenkungOhneJoystick(3, 70); break;
		case 1: motorAnsteuernMitLenkungOhneJoystick(5, 70); break;
		case 2: motorAnsteuernMitLenkungOhneJoystick(7, 70); break;
		default: UARTprintf("Keine Richtung vorgegeben!! \n"); break;
	}

	UARTprintf("Richtung: %d \n", richtung);


}

int main() {

	//Initializations
	motorInit();
	initi2c();
	InfrarotSensorInit();

	int schwarzeLinieLage;

	while (1)
	{
		schwarzeLinieLage = IRWertRichtung(); //Richtung auslesen, abhängig von Lage der schwarzen Linie

		//delay_ms(500); //Fahrzeug soll erstmal schrittweise fahren

		//print out the direction / position on Terminal
		if(schwarzeLinieLage == 1){
			UARTprintf("Linie ist: mittig \n");
		} else if(schwarzeLinieLage == 0){
			UARTprintf("Linie ist: links \n");
		} else if(schwarzeLinieLage == 2){
			UARTprintf("Linie ist: rechts \n");
		} else{
			UARTprintf("Linie ist nicht auffindbar! \n");
		}

		//Motor ansteuern - entsprechend der Lage der schwarzen Linie!
		spurVorgabeMotor(schwarzeLinieLage);

	}
	return 0;
}
