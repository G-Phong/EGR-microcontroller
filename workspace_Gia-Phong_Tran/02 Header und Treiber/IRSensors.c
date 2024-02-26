// Verlinkt
//Dieser Treiber soll eine Funktion zum Auslesen der IR-Sensoren enthalten

#define PORT_EXP_ADDR 0x38

#include "i2c.h"
#include <hw_types.h>
#include "delay_ms.h"
#include "uartStdio.h"
#include "stdio.h"

int vorherLinks = 0;
int vorherRechts = 0;
int kurveAktiv = 0;

void IRauslesen(){

	char receiveArray[50];

	unsigned char portExpanderConfigArray[2]; //2 bytes: 1 für command byte und 1 für data byte
	portExpanderConfigArray[0] = 0b00000011; //config reg 3
	portExpanderConfigArray[1] = 0b01111111; //P0-P6 als Input, P7 als Output

	unsigned char portExpanderOutputHIGH[2];
	portExpanderOutputHIGH[0] = 0b00000001; //output reg 1
	portExpanderOutputHIGH[1] = 0b10000000; //Bit 7 auf 1

	unsigned char portExpanderOutputLOW[2];
	portExpanderOutputLOW[0] = 0b00000001; //output reg 1
	portExpanderOutputLOW[1] = 0b00000000;
	//= "0000000100000000"; //Bit 7 auf 0

	unsigned char portExpanderInput[1];
	portExpanderInput[0] = 0b00000000;

	writetoi2c(PORT_EXP_ADDR , &portExpanderConfigArray[0], 2, 1); //P0-P6 als input; P7 als output

	//Schreibbefehl -> entsprechendes Register auswählen vom Port-Expander (REG 0 für Input-REG)

	delay_ms(10); //10ms warten -> damit der IR-Sensor klarkommt


	writetoi2c(PORT_EXP_ADDR , portExpanderOutputHIGH, 2, 1); //Output Pin P7 HIGH -> LEDs an
	//UARTprintf(BYTETOBINARYPATTERN"\n", BYTETOBINARY(portExpanderOutputHIGH[1]));

	writetoi2c(PORT_EXP_ADDR , portExpanderInput, 1, 0);
	readfromi2c(PORT_EXP_ADDR, receiveArray, 1, 1);
	UARTprintf(BYTETOBINARYPATTERN"\n", BYTETOBINARY(receiveArray[0]));

	delay_ms(500);


	writetoi2c(PORT_EXP_ADDR , portExpanderOutputLOW, 2, 1); //Output Pin P7 LOW -> LEDs aus
	//UARTprintf(BYTETOBINARYPATTERN"\n", BYTETOBINARY(portExpanderOutputLOW[1]));


	delay_ms(500);

}


void InfrarotSensorInit(){

	//UARTprintf("IR-Sensors successfully initialized!\n");

	unsigned char portExpanderConfigArray[2]; //2 bytes: 1 für command byte und 1 für data byte
	portExpanderConfigArray[0] = 0b00000011; //config reg 3
	portExpanderConfigArray[1] = 0b01111111; //P0-P6 als Input, P7 als Output

//	unsigned char portExpanderOutputHIGH[2];
//	portExpanderOutputHIGH[0] = 0b00000001; //output reg 1
//	portExpanderOutputHIGH[1] = 0b10000000; //Bit 7 auf 1
//
//	unsigned char portExpanderOutputLOW[2];
//	portExpanderOutputLOW[0] = 0b00000001; //output reg 1
//	portExpanderOutputLOW[1] = 0b00000000;
//
//	unsigned char portExpanderInput[1];
//	portExpanderInput[0] = 0b00000000;

	writetoi2c(PORT_EXP_ADDR , &portExpanderConfigArray[0], 2, 1); //P0-P6 als input; P7 als output

	UARTprintf("IR-Sensors initialized!\n");

}


/*
 * gibt 3 mögliche int-Werte zurück:
 *
 *  0 = links -> schwarze Linie ist links
 *  1 = mitig -> schwarze Linie ist mittig
 *  2 = rechts -> schwarze Linie ist rechts
 *
 */
int IRWertRichtung(){

	//UARTprintf("IRWertRichtung Funktion \n");

	int lage = 1; //per default auf 1 -> mittig



	char receiveArray[50];

	unsigned char portExpanderInput[1];
	portExpanderInput[0] = 0b00000000;

	 //10ms warten -> damit der IR-Sensor klarkommt
	delay_ms(10);

	//Auslesen der Sensoren
	unsigned char portExpanderOutputHIGH[2];
	portExpanderOutputHIGH[0] = 0b00000001; //output reg 1
	portExpanderOutputHIGH[1] = 0b10000000; //Bit 7 auf 1

	unsigned char portExpanderOutputLOW[2];
	portExpanderOutputLOW[0] = 0b00000001; //output reg 1
	portExpanderOutputLOW[1] = 0b00000000;


	delay_ms(10); //10ms warten -> damit der IR-Sensor klarkommt

	//UARTprintf("IRWertRichtung Funktion step1 \n");

	writetoi2c(PORT_EXP_ADDR , portExpanderOutputHIGH, 2, 1); //Output Pin P7 HIGH -> LEDs an
	//UARTprintf("IRWertRichtung Funktion step1.1 \n");
	writetoi2c(PORT_EXP_ADDR , portExpanderInput, 1, 0);
	//UARTprintf("IRWertRichtung Funktion step1.2 \n");
	readfromi2c(PORT_EXP_ADDR, receiveArray, 1, 1); //hier werden die Sensorwerte ausgelesen und in das receiveArray gespeichert

	//UARTprintf("IRWertRichtung Funktion step2 \n");

	//UARTprintf(BYTETOBINARYPATTERN"\n", BYTETOBINARY(receiveArray[0]));

	delay_ms(10);

	writetoi2c(PORT_EXP_ADDR , portExpanderOutputLOW, 2, 1); //Output Pin P7 LOW -> LEDs aus

	//UARTprintf("IRWertRichtung Funktion step3 \n");

	//Zuordnung des Binärwerts zu einer logischen Richtung
	int sensorLinks = (receiveArray[0] & 0x40 ? 1 : 0); //wird "1" wenn schwarz
	int sensorMitte = (receiveArray[0] & 0x20 ? 1 : 0);
	int sensorRechts = (receiveArray[0] & 0x10 ? 1 : 0);


	UARTprintf("links %d\n", sensorLinks);
	UARTprintf("mittig %d\n", sensorMitte);
	UARTprintf("rechts %d\n", sensorRechts);

	//Folgender Code ist für Disziplin 1, kann für Disziplin 2 auskommentiert werden (dafür dann den unteren Code einbinden)
	if(sensorMitte == 1 && sensorLinks == 0 && sensorRechts == 0){
		lage = 1;
	} else if(sensorLinks == 1 && sensorMitte == 0 && sensorRechts == 0){
		lage = 0;
	} else if(sensorRechts == 1 && sensorMitte == 0 && sensorLinks == 0){
		lage = 2;
	}

////Folgenden Code einkommentieren für Olympiade Disziplin 2
//	if(sensorMitte == 1 && sensorLinks == 1 && sensorRechts == 0){ //geradeaus, linie links
//		lage = 1;
//		vorherLinks = 1;
//
//		if(kurveAktiv == 1){
//			kurveAktiv = 0;
//
//		}
//
//	} else if(sensorMitte == 0 && sensorLinks == 1 && sensorRechts == 0){ //links
//		lage = 0;
//
//	} else if(sensorMitte == 1 && sensorLinks == 0 && sensorRechts == 1){ //geradeaus, linie rechts
//		lage = 1;
//		vorherRechts = 1;
//
//		if(kurveAktiv == 1){
//			kurveAktiv = 0;
//		}
//
//	} else if(sensorMitte == 0 && sensorLinks == 0 && sensorRechts == 1){ //rechts
//		lage = 2;
//
//	} else if(vorherLinks == 1 && sensorMitte == 1 && sensorLinks == 1 && sensorRechts == 1){
//		lage = 2;
//		kurveAktiv = 1;
//
//	} else if(vorherRechts == 1 && sensorMitte == 1 && sensorLinks == 1 && sensorRechts == 1){
//		lage = 0;
//		kurveAktiv = 1;
//
//	} else{
//		lage = 1;
//	}

//	if(sensorMitte == 0 && sensorLinks == 0 && sensorRechts == 0){ //default: wenn alles weiß, dann fahr geradeaus
//			lage = 1;
//		}else if(sensorMitte == 0 && sensorLinks == 1 && sensorRechts == 0){//wenn schwarz links ist, dann fahr rechts
//			lage = 2;
//		}
//		else if(sensorMitte == 0 && sensorLinks == 0 && sensorRechts == 1){ //wenn schwarz rechts ist, dann fahr links
//			lage = 0;
//		} else{
//			lage = 1;
//		}



	UARTprintf("IRWertRichtung Funktion step4 \n");

	return lage;

}
