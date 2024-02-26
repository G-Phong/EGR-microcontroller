/* EGR */

#include <hw_types.h>
#include <interrupt.h>
#include <hw_gpio_v2.h>
#include <hw_dmtimer.h>
#include <soc_AM335x.h>
#include <uartStdio.h>

#include "GPIO.h"
#include "EGR_Cape.h"
#include "Conf_mod.h"
#include "delay_ms.h"


volatile int tastendruck = 1;

/*hier noch ein GPIO Interrupt für den Taster
 * hier soll einfach die "Polarität" umgedreht werden: LOW -> HIGH / HIGH -> LOW
 * Logiktabelle beachten
 *
 */
void tasterISR(){
	UARTprintf("ISR aktiv\n");

	//Hilfsvariablen
	unsigned int high, low = 0;

	//Zustand invertieren (Links/Rechts)
	tastendruck = !tastendruck;

	//Zustand umsetzen -> HIGH Signal sorgt dafür, dass Motoren mit maximaler Leistung fahren
	if(tastendruck){
		high = PIN_HIGH;
		low = PIN_LOW;
		UARTprintf("Vorwärts fahren\n");

	} else{
		high = !PIN_HIGH;
		low = !PIN_LOW;
		UARTprintf("Rückwärts fahren\n");

	}

	//FIN auf HIGH setzen
	EGR_PinWrite(SOC_GPIO_1_REGS,18,high); //Motor 1 FIN
	EGR_PinWrite(SOC_GPIO_0_REGS,22,low); //Motor 2 FIN (low! -> dann drehen sich Räder in die gleiche Richtung)

	//RIN auf LOW setzen
	EGR_PinWrite(SOC_GPIO_1_REGS,19,low); //Motor 1 RIN
	EGR_PinWrite(SOC_GPIO_0_REGS,23,high); //Motor 2 RIN (high! -> dann drehen sich Räder in die gleiche Richtung)

	//Löschen des Interrupt-Flags -> am Ende der ISR
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) |= (1<<GPIO_PORT1_PIN2); //Pin 2 Flag löschen
}


int main() {

	//_______________________MOTOR________________________

	//Motor 1 Muxing
	PinMuxing(0x848, PULL_DISABLE, PULL_DOWN, GPIO_MODE); //Offsetadresse von conf_gpmc_a2 (RIN)
	PinMuxing(0x84C, PULL_DISABLE, PULL_DOWN, GPIO_MODE); //Offsetadresse von conf_gpmc_a3 (FIN)

	EGR_GPIODirSet(0x4804C000, 18, GPIO_OUTPUT); //gpio1_18
	EGR_GPIODirSet(0x4804C000, 19, GPIO_OUTPUT); //gpio1_19

	//Motor 2 Muxing
	PinMuxing(0x820, PULL_DISABLE, PULL_DOWN, GPIO_MODE); //Offsetadresse von conf_gpmc_ad8 (RIN)
	PinMuxing(0x824, PULL_DISABLE, PULL_DOWN, GPIO_MODE); //Offsetadresse von conf_gpmc_ad9 (FIN)

	EGR_GPIODirSet(0x44E07000, 22, GPIO_OUTPUT); //gpio0_22
	EGR_GPIODirSet(0x44E07000, 23, GPIO_OUTPUT); //gpio0_23


	//__________________TASTER____________________________

	//Taster Muxing
	PinMuxing(CONF_PORT1_PIN2, PULL_ENABLE, PULL_UP, GPIO_MODE);
	EGR_GPIODirSet(GPIO_PORT1_PIN2_MODUL, GPIO_PORT1_PIN2, GPIO_INPUT);

	//GPIO Interrupt initialisieren
	IntAINTCInit(); //Initialisierung ARM Interrupt Controller
	IntMasterIRQEnable(); //globale Interruptsteuerung aktivieren
	IntSystemEnable(SYS_INT_GPIOINT2A); //lokale Interruptsteuerung -> Interruptquelle -> GPIO2
	IntRegister(SYS_INT_GPIOINT2A, tasterISR); //ISR übergeben

	//Interrupt-Trigger setzen
	HWREG(SOC_GPIO_2_REGS + GPIO_FALLINGDETECT) |= (1<<GPIO_PORT1_PIN2);
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS_SET(0)) |= (1<<GPIO_PORT1_PIN2); //Interrupt A

	while (1)
	{
		//
	}
	return 0;
}
