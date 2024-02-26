/* EGR */

#include "i2c.h"
#include <hw_types.h>
#include "delay_ms.h"
#include "uartStdio.h"
#include "stdio.h"
#include "IRSensors.h"

#define PORT_EXP_ADDR 0x38

int main() {

	//initi2c(); //I2C-Kommunikation initialisieren
	UARTprintf("Loop\n");

	initi2c();

	InfrarotSensorInit();


	int ausgabeLage;

	while (1)
	{
		UARTprintf("while-Schleife1");

		ausgabeLage = IRWertRichtung();

		delay_ms(100);

		UARTprintf("while-Schleife");



	}

//	while (1)
//	{
//
//		IRauslesen();
//
//	}
	return 0;
}
