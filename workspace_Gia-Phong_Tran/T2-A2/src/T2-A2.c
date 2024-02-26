/* EGR */

#include <hw_types.h>
#include <interrupt.h>
#include "EGR_Cape.h"
#include <hw_gpio_v2.h>
#include <hw_dmtimer.h>
#include <soc_AM335x.h>
#include "GPIO.h"
#include <uartStdio.h>
#include "Conf_mod.h"
#include "delay_ms.h"

//#define 	TIMER_LOAD_VALUE	 4294487295 //0xFFFF FFFF - 20*24000
//#define 	COMPARE_VALUE 		 4294511295 //4294487295 + 24000
#define MITTELSTELLUNG 0xFFF9399F

volatile int aux = 0;
volatile int increment = 2400; //0,1ms
//volatile int leftIncrement = 2400; //0,1ms

void servoISR(){

	// Abfrage der Interruptquelle
	if ((HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQSTATUS) & (1<<1)) != 0)
	{
		// Löschen des Timer-interrupt-Flags
		HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQSTATUS) = (1<<1);

		// hier: eigener Code für Timer Overflow (20ms)
		//aux = 0; //zurücksetzen nach 20ms
		EGR_PinWrite(GPIO_PORT3_PIN2_MODUL,	GPIO_PORT3_PIN2, PIN_HIGH); // HIGH SETZEN


	}

	else if ((HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQSTATUS) & (1<<0)) != 0)
	{
		// Löschen des Timer-interrupt-Flags
		HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQSTATUS) = (1<<0);

		// hier: eigener Code für Compare Match
//		if(!aux){
//			EGR_PinWrite(GPIO_PORT3_PIN2_MODUL,	GPIO_PORT3_PIN2, PIN_LOW);	//LOW setzen
//			aux = 1;
//				}

		EGR_PinWrite(GPIO_PORT3_PIN2_MODUL,	GPIO_PORT3_PIN2, PIN_LOW);	//LOW setzen


	}

}

void tasterISR(){



	//Abfrage der Taster durch Flags
	if ((HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) & (1<<GPIO_PORT1_PIN2)) != 0){
		if(HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) < MITTELSTELLUNG + 5*increment){
			HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) += increment; //nach rechts bei Taster 1
					UARTprintf("Rechts\n");
		} else{
			UARTprintf("Maximale Rechtsstellung\n");
		}

	}

	else if ((HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) & (1<<GPIO_PORT1_PIN4)) != 0){

		if(HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) > MITTELSTELLUNG - 5*increment){
					HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) -= increment; //nach links bei Taster 2
							UARTprintf("Links\n");
		} else{
			UARTprintf("Maximale Linksstellung\n");
		}
	}

	else if ((HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) & (1<<GPIO_PORT1_PIN6)) != 0){

		HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) = MITTELSTELLUNG; //Mittelstellung
		UARTprintf("Mitte\n");
	}


	//Löschen des Interrupt-Flags -> am Ende der ISR
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) |= (1<<GPIO_PORT1_PIN2); //Pinnumber 2
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) |= (1<<GPIO_PORT1_PIN4); //Pinnumber 4
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) |= (1<<GPIO_PORT1_PIN6); //Pinnumber 6

}



int main() {

	//Initialisierung: Timer-Interrupt
	IntAINTCInit(); //Initialisierung ARM Interrupt Controller
	IntMasterIRQEnable(); //globale Interruptsteuerung aktivieren

	IntSystemEnable(SYS_INT_TINT2); //lokale Interruptsteuerung -> Interruptquelle -> Timer 2
	IntRegister(SYS_INT_TINT2, servoISR); //ISR übergeben

	IntSystemEnable(SYS_INT_GPIOINT2A); //lokale Interruptsteuerung -> Interruptquelle -> GPIO2
	IntRegister(SYS_INT_GPIOINT2A, tasterISR); //tasterISR übergeben

	//Control Register des DMTimer 2 beschreiben
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCLR) &=~ ((1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0)); //erst löschen
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCLR) |= (1<<6)|(1<<1); //setzen -> CompareMatch-Modus aktivieren und AutoReload

	//unsigned int timer_load_value = 4294487295;
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCRR) = 0xFFF8ACFF; //Timer CounteR Register -> Startwert für ersten Timerstart
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TLDR) = 0xFFF8ACFF; //Timer LoaD Register -> Reloadwert für ersten Timer
	//0xFFFF FFFF (=0d4294967295) ist overflow wert -> 4294487295 bis 4294967295 entspricht 20ms bis overflow


	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQENABLE_SET) |= (1<<1)|(1<<0); //Bits setzen um Compare-Match und Overflow zu enablen

	// set a new compare value
	//unsigned int meineVariable = 4294487295 + 24000; //dieser Wert wird nach 1ms erreicht, wenn man beim timer_load_value anfängt zu zählen
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) = 0xFFF90ABF;

	//2ms
	//HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) = 0xFFF9687F; //dieser Wert wird nach 2ms erreicht, wenn man beim timer_load_value anfängt zu zählen


	//Ansteuern des Servos über 1-2ms Impulse im Abstand von 20ms
	//1 ms-Impuls maximale Linksstellung, 2ms-Impuls maximale Rechtsstellung und 1,5ms Mittelstellung


	//______________________PIN GPIO_________________________
	PinMuxing(GPIO_PORT3_PIN2_MODUL, PULL_ENABLE, PULL_DOWN, GPIO_MODE); //Pinmuxing

	//Port 3 Pin 2 -> "GPMC_AD4" -> GPIO1_4 -> als GPIO_OUTPUT setzen
	EGR_GPIODirSet(GPIO_PORT3_PIN2_MODUL,GPIO_PORT3_PIN2, GPIO_OUTPUT); //PIN ist OUTPUT -> für Steuersignal des Servos
	//_________________________________________________________


	//Realisieren sie die Periode mithilfe des Overflows und
	//des Reload-Values und die Länge des Pulses mit einem Compare-Match.


	//______________________PIN GPIO TASTER_________________________
	PinMuxing(CONF_PORT1_PIN2, PULL_ENABLE, PULL_UP, GPIO_MODE); //Taster 1 = rechts drehen
	PinMuxing(CONF_PORT1_PIN4, PULL_ENABLE, PULL_UP, GPIO_MODE); //Taster 2 = links drehen
	PinMuxing(CONF_PORT1_PIN6, PULL_ENABLE, PULL_UP, GPIO_MODE); //Taster 3 = Motor-Nullstellung

	//Port 3 Pin 2 -> "GPMC_AD4" -> GPIO1_4 -> als GPIO_INPUT setzen
	EGR_GPIODirSet(GPIO_PORT1_PIN2_MODUL, GPIO_PORT1_PIN2, GPIO_INPUT);
	EGR_GPIODirSet(GPIO_PORT1_PIN4_MODUL, GPIO_PORT1_PIN4, GPIO_INPUT);
	EGR_GPIODirSet(GPIO_PORT1_PIN6_MODUL, GPIO_PORT1_PIN6, GPIO_INPUT);
	//_________________________________________________________


	//GPIO-INTERRUPTS
	//Interrupt-Trigger setzen
	HWREG(SOC_GPIO_2_REGS + GPIO_FALLINGDETECT) |= (1<<GPIO_PORT1_PIN2);
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS_SET(0)) |= (1<<GPIO_PORT1_PIN2); //Interrupt A

	HWREG(SOC_GPIO_2_REGS + GPIO_FALLINGDETECT) |= (1<<GPIO_PORT1_PIN4);
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS_SET(0)) |= (1<<GPIO_PORT1_PIN4); //Interrupt A

	HWREG(SOC_GPIO_2_REGS + GPIO_FALLINGDETECT) |= (1<<GPIO_PORT1_PIN6);
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS_SET(0)) |= (1<<GPIO_PORT1_PIN6); //Interrupt A


	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCLR) |= (1<<0); //Timer starten -> muss im letzten Schritt vor while(1) passieren

	while (1)
	{
		//Servomotor hin und herbewegen
		//Steuersignal mit Frequenzen 1ms und 2ms senden
		//Frequenzen kriege ich hin, indem ich im Register LOW und HIGH einfach abwechselnd setze, dabei nutze ich Interrupts mit ISR um das Register zu beschreiben.
		//Wie oft die ISR durchgeführt wird bestimmte ich mit Timern

//		if(!aux){
//			EGR_PinWrite(GPIO_PORT3_PIN2_MODUL,	GPIO_PORT3_PIN2, PIN_HIGH);	//Am Anfang auf High setzen
//		}

//		HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) = 0xFFF90ABF; //1ms
//
//		delay_ms(2*1000);
//
//		HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) = 0xFFF9687F; //2ms
//
//		delay_ms(2*1000);

	}
	return 0;
}
