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
#include "EGR_DCMotor.h"

volatile int meinDutyCycle = 0;
volatile unsigned int meinDutyCycleOhneVZ = 0;


/*
 * Diese Funktion wird nicht benutzt in diesem .c Programm -> sie wird stattdessen in den Motorsteuerung Dateien unter einem anderen Namen benutzt
 * Die übergebene Geschwindigkeit kann auch negativ sein!
 *
 */
//void motorFunktionTest(int geschwindigkeit){
//
//	UARTprintf("Geschwindigkeit: %d!\n", geschwindigkeit);
//
//	//Hilfsvariablen
//	unsigned int geschwindigkeitOhneVZ = geschwindigkeit;
//
//	if(geschwindigkeit < 0){
//		geschwindigkeitOhneVZ = -geschwindigkeit;
//	}
//
//	//Geschwindigkeit setzen
//	EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, geschwindigkeitOhneVZ);
//	EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, geschwindigkeitOhneVZ);
//
//	//Motordrehrichung setzen
//	if(geschwindigkeit > 0){
//		//Motor 1 vorwärts mit PWM
//		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
//
//		EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, 0);
//		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);
//
//		//Motor 2 vorwärts mit PWM
//		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
//
//		EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, 0);
//		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);
//
//		UARTprintf("Rückwärts!\n");
//
//	} else if(geschwindigkeit < 0){
//
//		//Motor 1 rückwärts mit PWM
//			configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
//
//			EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, 0);
//			configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);
//
//			//Motor 2 rückwärts mit PWM
//			configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
//
//			EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, 0);
//			configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);
//
//		UARTprintf("Vorwärts!\n");
//
//	} else if(geschwindigkeit == 0){
//		UARTprintf("Stillstand!\n");
//
//	} else{
//		UARTprintf("Ungültiger Geschwindigkeitswert!\n");
//	}
//
//}

void tasterISR(){

	UARTprintf("tasterISR aktiv\n");

	//___________________Geschwindigkeit über PWM mit Taster___________________

	if((HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) & (1<<GPIO_PORT1_PIN2)) != 0){ //falls Taster 1 gedrückt

		if(meinDutyCycle < 100){
			meinDutyCycle += 10;
			if(meinDutyCycle < 0){
				meinDutyCycleOhneVZ = -meinDutyCycle;
			} else { meinDutyCycleOhneVZ = meinDutyCycle;}
		}

		EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, meinDutyCycleOhneVZ);
		EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, meinDutyCycleOhneVZ);

		UARTprintf("Geschwindigkeit: %d!\n", meinDutyCycle);

	} else if((HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) & (1<<GPIO_PORT1_PIN4)) != 0){ //falls Taster 2 gedrückt

		if(meinDutyCycle > -100){
			meinDutyCycle -= 10;
			if(meinDutyCycle < 0){
				meinDutyCycleOhneVZ = -meinDutyCycle;
			} else { meinDutyCycleOhneVZ = meinDutyCycle;}

		}

		EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, meinDutyCycleOhneVZ);
		EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, meinDutyCycleOhneVZ);

		UARTprintf("Geschwindigkeit: %d!\n", meinDutyCycle);

	}

	//___________________Vorwärts/Rückwärts___________________

	if(meinDutyCycle > 0){
		//Motor 1 vorwärts mit PWM
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		//Motor 2 vorwärts mit PWM
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		UARTprintf("Vorwärts!\n");

	} else if(meinDutyCycle < 0){
		//Motor 1 rückwärts mit PWM
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		//Motor 2 rückwärts mit PWM
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		UARTprintf("Rückwärts!\n");

	} else{
		UARTprintf("Stillstand!\n");
	}

	//Löschen des Interrupt-Flags -> am Ende der ISR
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) |= (1<<GPIO_PORT1_PIN2); //Pin 2 Flag löschen
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS(0)) |= (1<<GPIO_PORT1_PIN4); //Pin 4 Flag löschen

}


int main() {

	//_______________________MOTOR________________________

	//EHRPWM1 und EHRPWM2 initialisieren
	EHRPWMinitForDCMotor();

	//Muxing mit Modus 6 (ehrpwm1A und ehrpwm1B)
	//Motor 1 Muxing
	PinMuxing(0x848, PULL_DISABLE, PULL_DOWN, 6); //Offsetadresse von conf_gpmc_a2 (RIN)
	PinMuxing(0x84C, PULL_DISABLE, PULL_DOWN, 6); //Offsetadresse von conf_gpmc_a3 (FIN)


	//Motor 2 Muxing
	PinMuxing(0x820, PULL_DISABLE, PULL_DOWN, 4); //Offsetadresse von conf_gpmc_ad8 (RIN)
	PinMuxing(0x824, PULL_DISABLE, PULL_DOWN, 4); //Offsetadresse von conf_gpmc_ad9 (FIN)


	//__________________ZWEI TASTER____________________________

	//Taster 1 Muxing
	PinMuxing(CONF_PORT1_PIN2, PULL_ENABLE, PULL_UP, GPIO_MODE);
	EGR_GPIODirSet(GPIO_PORT1_PIN2_MODUL, GPIO_PORT1_PIN2, GPIO_INPUT);

	//Taster 2 Muxing
	PinMuxing(CONF_PORT1_PIN4, PULL_ENABLE, PULL_UP, GPIO_MODE);
	EGR_GPIODirSet(GPIO_PORT1_PIN4_MODUL, GPIO_PORT1_PIN4, GPIO_INPUT);

	//GPIO Interrupt initialisieren
	IntAINTCInit(); //Initialisierung ARM Interrupt Controller
	IntMasterIRQEnable(); //globale Interruptsteuerung aktivieren
	IntSystemEnable(SYS_INT_GPIOINT2A); //lokale Interruptsteuerung -> Interruptquelle -> GPIO2
	IntRegister(SYS_INT_GPIOINT2A, tasterISR); //ISR übergeben

	//Interrupt-Trigger setzen
	HWREG(SOC_GPIO_2_REGS + GPIO_FALLINGDETECT) |= (1<<GPIO_PORT1_PIN2);
	HWREG(SOC_GPIO_2_REGS + GPIO_FALLINGDETECT) |= (1<<GPIO_PORT1_PIN4);

	//Interrupt Art-Zuweisung
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS_SET(0)) |= (1<<GPIO_PORT1_PIN2); //PIN2 -> Interrupt A
	HWREG(SOC_GPIO_2_REGS + GPIO_IRQSTATUS_SET(0)) |= (1<<GPIO_PORT1_PIN4); //PIN4 -> Interrupt A

	while (1)
	{
		//
	}
	return 0;
}
