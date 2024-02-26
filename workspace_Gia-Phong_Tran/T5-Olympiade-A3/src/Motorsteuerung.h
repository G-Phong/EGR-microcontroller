// Fuegt hier die Header-Daten zu euren Funktionen ein

/*
 * Motorsteuerung.h
 *
 */

#ifndef MOTORSTEUERUNG_H_
#define MOTORSTEUERUNG_H_

//Extremwerte für Geschwindigkeit [%]
//#define VOLLGAS_VOR	100
#define MOTORSTILLSTAND 	0
//#define VOLLGAS_ZURUECK	-100


//PWM Muxing
extern void motorInit();

//Motorfunktion
extern void motorAnsteuern(int geschwindigkeit);

//Motorfunktion mit Lenkung
extern void motorAnsteuernMitLenkung(int geschwindigkeit1, int geschwindigkeit2);

//Motorfunktion mit Lenkung ohne Joystick
extern void motorAnsteuernMitLenkungOhneJoystick(int richtung, int geschw);

#endif /* MOTORSTEUERUNG_H_ */
