/* EGR */

#include <hw_types.h>
#include "Conf_mod.h"
#include "GPIO.h"
#include "i2c.h"
#include "Motorsteuerung.h"
#include "IRSensors.h"
#include "EGR_Cape.h"
#include "delay_ms.h"
#include "EGR_DCMotor.h"

#include <interrupt.h>  //einbinden der Interrupt-Bib
#include <soc_AM335x.h>
#include <hw_gpio_v2.h>
#include <hw_dmtimer.h>
#include <uartStdio.h>

volatile int countdownFlag = 0; // 0 = off ; 2 = on

void timerISR(){

	UARTprintf("timerISR ausgelöst! \n");

	if ((HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQSTATUS) & (1<<0)) != 0){
			UARTprintf("Interrupt-Quelle: Compare-Match \n");
		}

	//tasterISR wird ausgelöst durch Tastendruck
	// -> tasterISR startet Timer
	// -> wenn Timer 5 Sekunden abgelaufen ist (Compare-Match), dann wird timerISR ausgelöst
	// -> timerISR wiederum setzt eine Flag, diese startet dann den "Rennmodus"b

	//Flag setzen
	countdownFlag++;

	UARTprintf("countdown-Flag nach timerISR: %d \n", countdownFlag);


	// Löschen des Timer-interrupt-Flags
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQSTATUS) = (1<<0); //Compare-Match Flag

}


void tasterCountdownISR(){ //Taster-ISR

	if(countdownFlag <= 1){
		UARTprintf("tasterISR aktiv: Timer wird gestartet! Rennmodus wird in 5 sek aktiviert! \n");
	} else{
		UARTprintf("Taster wurde gedrückt, Rennmodus ist aber bereits aktiv! \n");
	}


	//LÖSE IRGENDEINEN TIMER AUS
	// Compare-Match (5 Sekunden) -> Flag setzen
	// if(flag){führe Rennmodus aus}

	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCLR) |= (1<<0); //Timer starten

	UARTprintf("countdown-Flag nach tasterISR: %d \n", countdownFlag);

	//Löschen des Interrupt-Flags -> am Ende der ISR
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) |= (1<<GPIO_PORT1_PIN2); //Pinnumber 2
}

void spurVorgabeMotor(int richtung){

	if(richtung > 2 || richtung < 0){
		UARTprintf("Richtung ungültig! \n");
		return;
	}

	switch(richtung) {
		case 0: motorAnsteuernMitLenkungOhneJoystick(3, 65); break;
		case 1: motorAnsteuernMitLenkungOhneJoystick(5, 65); break;
		case 2: motorAnsteuernMitLenkungOhneJoystick(7, 65); break;
		default: UARTprintf("Keine Richtung vorgegeben!! \n"); break;
	}


}


int main() {

	delay_ms(500);
	//DMTIMER 3 NUTZEN, DAMIT ES NICHT MIT I2C interferiert!
	//HWREG(SOC_DMTIMER_3_REGS + DMTIMER_TCLR) &=~ (1<<0); //Timer starten ausschalten

	//Initializations
	motorInit();
	initi2c();
	InfrarotSensorInit();

	//Interrupts aktivieren
	//IntAINTCInit(); //Initialisierung ARM Interrupt Controller
	//IntMasterIRQEnable(); //globale Interruptsteuerung aktivieren

	//GPIO-Interrupts konfigurieren
	IntSystemEnable(SYS_INT_GPIOINT2A); //lokale Interruptsteuerung -> Interruptquelle -> GPIO2
	IntRegister(SYS_INT_GPIOINT2A, tasterCountdownISR); //ISR übergeben

	//Timer-Interrupts konfigurieren
	IntSystemEnable(SYS_INT_TINT2); //lokale Interruptsteuerung -> Interruptquelle -> Timer 2
	IntRegister(SYS_INT_TINT2, timerISR); //ISR übergeben

	UARTprintf("main1 \n");
	//Interrupt-Trigger setzen
	HWREG(SOC_GPIO_2_REGS + GPIO_FALLINGDETECT) |= (1<<GPIO_PORT1_PIN2);
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS_SET(0)) |= (1<<GPIO_PORT1_PIN2); //Interrupt A
	UARTprintf("main2 \n");
	//Taster konfigurieren
	PinMuxing(CONF_PORT1_PIN2,	PULL_ENABLE, PULL_UP, GPIO_MODE);
	EGR_GPIODirSet(GPIO_PORT1_PIN2_MODUL,GPIO_PORT1_PIN2,GPIO_INPUT);
	UARTprintf("main4 \n");
	//_______________________________________________________

	//Timer-Einstellungen für Countdown von 5s -> Timer soll Interrupt auslösen!
	//Control Register des DMTimer 2 beschreiben
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCLR) &=~ ((1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0)); //erst löschen
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCLR) |= (1<<6); //setzen -> CompareMatch-Modus aktivieren und ONE-SHOT MODUS (Bit 1 löschen)
	UARTprintf("main5 \n");
	//Timer CounteR Register -> Startwert für ersten Timerstart
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TCRR) = 0x00000000;
	UARTprintf("main6 \n");
	// set a new compare value, 5 Sekunden = 5 mal 24 Millionen Takte = 0x07270E00
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_TMAR) = 0x07270E00;
	UARTprintf("main7 \n");
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQENABLE_SET) &=~ ((1<<2)|(1<<1)|(1<<0)); //Bit 0 setzen um Compare-Match zu enablen
	HWREG(SOC_DMTIMER_2_REGS + DMTIMER_IRQENABLE_SET) |= (1<<0); //Bit 0 setzen um Compare-Match zu enablen

	UARTprintf("main8 \n");
	int schwarzeLinieLage;

	while (1)
	{
		if(countdownFlag >= 2){

			UARTprintf("Rennmodus aktiviert! \n");
			//Rennmodus-Implementierung

			/*Idee: wenn Sensoren alle weiß sehen -> Geradeaus fahren
			 * wenn linker Sensor schwarz sieht -> nach rechts fahren
			 * wenn rechter Sensor schwarz sieht -> nach links fahren
			 *
			 *
			*/

			schwarzeLinieLage = IRWertRichtung(); //Richtung auslesen, abhängig von Lage der schwarzen Linie

			UARTprintf("step \n");

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
					UARTprintf("step1 \n");

					//Motor ansteuern - entsprechend der Lage der schwarzen Linie!
					spurVorgabeMotor(schwarzeLinieLage);
					UARTprintf("step2 \n");
		}

		//delay_ms(1000);
	}
	return 0;
}
