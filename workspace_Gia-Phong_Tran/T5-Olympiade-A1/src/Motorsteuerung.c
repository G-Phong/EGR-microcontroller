// Fuegt hier eure Funktionen zum Ansteuern der Motoren ein

#include <hw_types.h>
#include <hw_gpio_v2.h>
#include <soc_AM335x.h>
#include <uartStdio.h>

#include "EGR_Cape.h"
#include "Conf_mod.h"
#include "EGR_DCMotor.h"
#include "Motorsteuerung.h"




/**
 * \brief  Diese Funktion initialisiert die PWM-Module und
 * 			muxt die PWM Pins der Motoren.
 *
 */
void motorInit(){
	//EHRPWM1 und EHRPWM2 initialisieren
	EHRPWMinitForDCMotor();

	//PWM-Pins separat muxen -> Muxing mit Modus 6 (ehrpwm1A und ehrpwm1B)
	//Motor 1 Muxing
	PinMuxing(0x848, PULL_DISABLE, PULL_DOWN, 6); //Offsetadresse von conf_gpmc_a2 (RIN)
	PinMuxing(0x84C, PULL_DISABLE, PULL_DOWN, 6); //Offsetadresse von conf_gpmc_a3 (FIN)


	//Motor 2 Muxing
	PinMuxing(0x820, PULL_DISABLE, PULL_DOWN, 4); //Offsetadresse von conf_gpmc_ad8 (RIN)
	PinMuxing(0x824, PULL_DISABLE, PULL_DOWN, 4); //Offsetadresse von conf_gpmc_ad9 (FIN)

	UARTprintf("Motor initialized!\n");
}


/**
 * \brief  Diese Funktion lässt beide Motoren laufen mit einer
 * 			gewissen Geschwindigkeit.
 *
 * \param	geschwindigkeit		 Wertebereich [-100;100]
 *
 *     Vorzeichen entscheidet über Richtung (vorwärts/rückwärts).
 */
void motorAnsteuern(int geschwindigkeit){

	UARTprintf("Geschwindigkeit: %d!\n", geschwindigkeit);

	//Hilfsvariablen
	unsigned int geschwindigkeitOhneVZ = geschwindigkeit;

	if(geschwindigkeit < 0){
		geschwindigkeitOhneVZ = -geschwindigkeit;
	}

	//Geschwindigkeit setzen
	EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, geschwindigkeitOhneVZ);
	EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, geschwindigkeitOhneVZ);

	//Motordrehrichung setzen
	if((geschwindigkeit < 0) && (geschwindigkeit >= -100)){
		//Motor 1 vorwärts mit PWM
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		//Motor 2 vorwärts mit PWM
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		UARTprintf("Rückwärts!\n");

	} else if((geschwindigkeit > 0)  && (geschwindigkeit <= 100)){

		//Motor 1 rückwärts mit PWM
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		//Motor 2 rückwärts mit PWM
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		UARTprintf("Vorwärts!\n");

	} else if(geschwindigkeit == 0){
		UARTprintf("Stillstand!\n");

	} else{

		//Motor 1 zurücksetzen
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		//Motor 2 zurücksetzen
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW);

		UARTprintf("Ungültiger Geschwindigkeitswert. Bremse auf 0!\n");


	}

}


/**
 * \brief  Diese Funktion lässt beide Motoren laufen mit einer
 * 			gewissen Geschwindigkeit.
 *
 * \param	geschwindigkeit		 Wertebereich [-100;100]
 *
 *     Vorzeichen entscheidet über Richtung (vorwärts/rückwärts).
 */
void motorAnsteuernMitLenkung(int geschwindigkeit1, int geschwindigkeit2){ //Input werte sind ADC-Werte aus dem FIFO

	//y-Achse invertieren (siehe Skizze)
//	int aux = 0;
//	aux = (4100-geschwindigkeit2);
//	int geschwindigkeit2_inv = aux;

	int x = 0;
	int y = 0;

	//Werte außerhalb des Wertebereichs behandeln (x)
	if(geschwindigkeit1 > 4000){
		x = 100;
	} else if(geschwindigkeit1 < 0){
		x = -100;
	} else{
		x = (100.0/2000.0)*((double)geschwindigkeit1-2000.0); //Skalierung x-Richtung (von -100 bis 100)
	}

	//Werte außerhalb des Wertebereichs behandeln (y) | ACHTUNG: WERTEBEREICH INVERTIERT! siehe skizze (TODO)
	if(geschwindigkeit2 > 4000){
		y = 100;
	} else if(geschwindigkeit2 < 0){
		y = -100;
	} else{
		y = (100.0/2000.0)*((double)geschwindigkeit2-2000.0); //Skalierung y-Richtung (von -100 bis 100)
	}

	y = -y; //wegen Achsen (siehe Skizze)

	UARTprintf("Geschwindigkeit ADC1: %d\n", geschwindigkeit1);
	UARTprintf("Geschwindigkeit ADC2: %d\n", geschwindigkeit2);

	UARTprintf("Geschwindigkeit x (skaliert): %d\n", x); //UARTprintf kann nur int ausgeben!
	UARTprintf("Geschwindigkeit y (skaliert): %d\n", y); //UARTprintf kann nur int ausgeben!

	//Hilfsvariablen
	//unsigned int geschwindigkeitOhneVZ_X = x;
	//unsigned int geschwindigkeitOhneVZ_Y = y;

	/*if(x < 0){
		geschwindigkeitOhneVZ_X = -x;
	}

	if(y < 0){
		geschwindigkeitOhneVZ_Y = -y;
	}*/


	int v_links = 0;
	int v_rechts = 0;


	if(x > 0){ //Rechts abbiegen
		v_links = y + 0.5*x;
		v_rechts = y - 0.5*x;
	} else if (x < 0){ //Links abbiegen
		v_links = y + 0.5*x;
		v_rechts = y - 0.5*x;
	} else {
		v_links = y;
		v_rechts = y;
	}




	UARTprintf("Geschwindigkeit v_links (skaliert): %d\n", v_links); //UARTprintf kann nur int ausgeben!
	UARTprintf("Geschwindigkeit v_rechts (skaliert): %d\n", v_rechts); //UARTprintf kann nur int ausgeben!

	int v_links_OhneVZ = v_links;
	int v_rechts_OhneVZ =  v_rechts;

	//Vorzeichen behandeln
	if(v_links < 0){
		v_links_OhneVZ = -v_links;
	}

	if(v_rechts < 0){
		v_rechts_OhneVZ = -v_rechts;
	}


	//Geschwindigkeit setzen
	EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, v_links_OhneVZ); //Motor1 (links)
	EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, v_rechts_OhneVZ); //Motor2 (rechts)

	//Motordrehrichung setzen -> TODO: FALLUNTERSCHEIDUNG!!
	if((v_links > 0) && (v_links <= 100)){
		//Motor 1 vorwärts mit PWM
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 1A
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); // LOW bei 1B

		UARTprintf("Links Vorwärts!\n");

	}

	if((v_links < 0) && (v_links >= -100)){

		//Motor 1 rückwärts mit PWM
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 1B
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); //LOW bei 1A

		UARTprintf("Links Rückwärts!\n");

	}

	if((v_rechts > 0) && (v_rechts <= 100)){

		//Motor 2 vorwärts mit PWM
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 2B
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); //LOW bei 2A

		UARTprintf("Rechts Vorwärts!\n");

	}

	if((v_rechts < 0) && (v_rechts >= -100)){

		//Motor 2 rückwärts mit PWM
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 2A
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); //LOW bei 2B

		UARTprintf("Rechts Rückwärts!\n");

	}




}


/**
 * \brief  Diese Funktion lässt beide Motoren laufen mit einer
 * 			gewissen Geschwindigkeit.
 *
 * \param	geschwindigkeit		 Wertebereich [-100;100]
 *
 *  richtung -> [0;10]
 *  geschw -> [-100;100]
 *
 *     Vorzeichen entscheidet über Richtung (vorwärts/rückwärts).
 */
void motorAnsteuernMitLenkungOhneJoystick(int richtung, int geschw){ //Input werte sind ADC-Werte aus dem FIFO

	int x = 0;
	int y = -geschw;


	int v_links = 0;
	int v_rechts = 0;

	//stumpf: 2 modi
	if((richtung < 5) && (richtung >= 0)){
		x = -50;
	} else if((richtung > 5) && (richtung <= 10)){
		x = 50;
	} else if(richtung == 5){
		x = 0;
	}


	if(x > 0){ //Rechts abbiegen
		v_links = y + 0.5*x;
		v_rechts = y - 0.5*x;
	} else if (x < 0){ //Links abbiegen
		v_links = y + 0.5*x;
		v_rechts = y - 0.5*x;
	} else {
		v_links = y;
		v_rechts = y;
	}


	UARTprintf("Geschwindigkeit v_links (skaliert): %d\n", v_links); //UARTprintf kann nur int ausgeben!
	UARTprintf("Geschwindigkeit v_rechts (skaliert): %d\n", v_rechts); //UARTprintf kann nur int ausgeben!

	int v_links_OhneVZ = v_links;
	int v_rechts_OhneVZ =  v_rechts;

	//Vorzeichen behandeln
	if(v_links < 0){
		v_links_OhneVZ = -v_links;
	}

	if(v_rechts < 0){
		v_rechts_OhneVZ = -v_rechts;
	}


	//Geschwindigkeit setzen
	EHRPWMsetDutyCycle(SOC_EPWM_1_REGS, v_links_OhneVZ); //Motor1 (links)
	EHRPWMsetDutyCycle(SOC_EPWM_2_REGS, v_rechts_OhneVZ); //Motor2 (rechts)

	//Motordrehrichung setzen -> TODO: FALLUNTERSCHEIDUNG!!
	if((v_links > 0) && (v_links <= 100)){
		//Motor 1 vorwärts mit PWM
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 1A
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); // LOW bei 1B

		UARTprintf("Links Vorwärts!\n");

	}

	if((v_links < 0) && (v_links >= -100)){

		//Motor 1 rückwärts mit PWM
		configEHRPWM_B(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 1B
		configEHRPWM_A(SOC_EPWM_1_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); //LOW bei 1A

		UARTprintf("Links Rückwärts!\n");

	}

	if((v_rechts > 0) && (v_rechts <= 100)){

		//Motor 2 vorwärts mit PWM
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 2B
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); //LOW bei 2A

		UARTprintf("Rechts Vorwärts!\n");

	}

	if((v_rechts < 0) && (v_rechts >= -100)){

		//Motor 2 rückwärts mit PWM
		configEHRPWM_A(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_HIGH, EHRPWM_SET_OUTPUT_LOW); //PWM bei 2A
		configEHRPWM_B(SOC_EPWM_2_REGS, EHRPWM_SET_OUTPUT_LOW, EHRPWM_SET_OUTPUT_LOW); //LOW bei 2B

		UARTprintf("Rechts Rückwärts!\n");

	}




}
