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

#include  <hw_tsc_adc_ss.h>

int main() {

	//__________________________________ADC CONFIG________________________________________

	//Clock aktivieren -> durch Clock-Zuweisung wird ADC-Modul aktiviert
	HWREG(SOC_CM_WKUP_REGS + (0xbc)) &=~ (0b11); //Bit 0 und 1 löschen
	HWREG(SOC_CM_WKUP_REGS + (0xbc)) |= (1<<1); //Bit 1 setzen

	//Schreibschutz der ADC-TSC Konfiguration deaktivieren
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_CTRL) |= (1 << 2); //Bit 2 -> writable


	//STEP aktivieren
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPENABLE) &=~ ((1<<2)|(1<<1)|(1<<0)); //STEP1 aktivieren -> 1 Step = 1 Analogspannung messen
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPENABLE) |= (1<<1); //STEP1 aktivieren -> 1 Step = 1 Analogspannung messen

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPENABLE) |= (1<<2); //so würde man STEP2 aktivieren


	//Jeden Step einzeln einstellen:
	//STEP1: STEPCONFIG
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) &=~ ((1<<12)|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0)); //alles löschen

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) |= (1<<0); //STEP 1: "SW enabled, continuous"

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) |= (1<<4); //STEP 1: "16 samples average"

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) &=~ ((1<<14)|(1<<13)|(1<<12)); //STEP 1: "VDDA"

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) &=~ ((1<<22)|(1<<21)|(1<<20)|(1<<19)); //STEP 1: "Wahl des Analogpins" (AIN1 -> Port 1)
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) |= (1<<19); //STEP 1: AIN1 -> Channel 2 -> Setzen einer 1

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) &=~ (1<<26); //STEP 1: Store Data in FIFO1 -> Bit 26 setzen
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(0)) |= (1<<26); //STEP 1: Store Data in FIFO1 -> Bit 26 setzen

//	//STEP2: STEPCONFIG
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(1)) &=~ ((1<<12)|(1<<11)|(1<<10)|(1<<9)|(1<<8)|(1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0)); //alles löschen

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(1)) |= (1<<0); //STEP 2: "SW enabled, continuous"

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(1)) |= (1<<4); //STEP 2: "16 samples average"

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(1)) &=~ ((1<<14)|(1<<13)|(1<<12)); //STEP 1: "VDDA"

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(1)) &=~ ((1<<22)|(1<<21)|(1<<20)|(1<<19)); //STEP 1: "Wahl des Analogpins" (AIN1 -> Port 1)
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(1)) |= (1<<20)|(1<<19); //STEP 2: AIN3 -> Channel 4 -> Setzen einer 3

	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_STEPCONFIG(1)) &=~ (1<<26); //STEP 2: Store Data in FIFO0 -> Bit 26 löschen

	//ADC aktivieren
	HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_CTRL) |= (1<<0); //ADC enablen

	//__________________________________ADC CONFIG________________________________________



	unsigned int meinADCWert = 0;
	unsigned int meinADCWert2 = 0;


	while (1)
	{

		//Aktuellen FIFO-Wert in eigene Variable schreiben
		meinADCWert = HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_FIFODATA(1));
		meinADCWert2 = HWREG(SOC_ADC_TSC_0_REGS + TSC_ADC_SS_FIFODATA(0));

		UARTprintf("ADC Wert: %d\n", meinADCWert);
		UARTprintf("ADC Wert2: %d\n", meinADCWert2);
		UARTprintf("\n");


		delay_ms(1000);


	}
	return 0;
}
