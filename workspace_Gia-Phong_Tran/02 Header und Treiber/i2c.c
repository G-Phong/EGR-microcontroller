// Dies ist eine verlinkte Datei

/*

 * i2c.c
 *
 *  Created on: 13.12.2013
 *      Author: Coy
 *		Copyright: TUM MiMed 2013
 *		beaglebone EGR
 *		Version: 1.0
 */
#include <hw_types.h>
#include <hw_hsi2c.h>
#include "delay_ms.h"
#include "i2c.h"
#include "Conf_mod.h"
#include <uartStdio.h>

//SOC_PRCM_REGS stattdessen nutzen!
#define BASE_ADDR_I2C_MODULE 0x44E00000
#define I2C_ENABLE_BIT 15

//schon existent
//#define I2C_CON 0xA4
//#define I2C_PSC 0xB0
//#define I2C_SCLH 0xB8
//#define I2C_SCLL 0xB4

volatile unsigned int transCount;
volatile unsigned int recvCount;
volatile unsigned char dataSnd[50];
volatile unsigned char dataRecv[50];

void
initi2c()
{
  //Enable interrupts
  IntMasterIRQEnable();

  IntAINTCInit();

  IntRegister(SYS_INT_I2C1INT, I2CIsr);

  IntPrioritySet(SYS_INT_I2C1INT, 0, AINTC_HOSTINT_ROUTE_IRQ);

  IntSystemEnable(SYS_INT_I2C1INT);

  //Pinmux I2C_SCLK and I2C_SDA
  PinMuxing(0x958,PULL_DISABLE, PULL_DOWN, 2); // (conf_spi0_d1) Pin 2 muxen -> SPI _SDA Pin laut Datenblatt
  PinMuxing(0x95C,PULL_DISABLE, PULL_DOWN, 2); // (conf_spi0_cs0) Pin 2 muxen -> SPI _SCL Pin laut Datenblatt

  //Enable I2C1 Module Clock
  //HWREG(0x44E00000 + 0x48) |= 0x2u;
  HWREG(SOC_PRCM_REGS + CM_PER_I2C1_CLKCTRL) |= CM_PER_I2C1_CLKCTRL_MODULEMODE_ENABLE;

  //Disable I2C Module for Configuration
  //Base address I2C Module: 0x44E0_B000
  //I2C-CON -> 0xA4
  //Bit 15 -> I2C_CON (offset: 0xA4) -> this bit enables/disables the I2C Module -> clear bit to disable
  HWREG(SOC_I2C_1_REGS + I2C_CON) &=~ (1<<I2C_ENABLE_BIT);

  //Prescale functional clock to internal i2c clock
  //Prescaler soll Geschw. auf 12 MHz reduzieren, verbundene Clock hat 48 MHz
  HWREG(SOC_I2C_1_REGS + I2C_PSC) &=~ ((1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));
  HWREG(SOC_I2C_1_REGS + I2C_PSC) |= (1<<0)|(1<<1); //0x3 in das Prescaler-Reg schreiben (= Prescaler-Faktor 4)

  //Comparematch for low-time
  //Bus-Geschw. von 400 kHz -> "8" in das I2C_SCLL schreiben;
  HWREG(SOC_I2C_1_REGS + I2C_SCLL) &=~ ((1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));
  HWREG(SOC_I2C_1_REGS + I2C_SCLL) |= 0b1000;

  //Comparematch for high-time -> "10" in das I2C_SCLH schreiben
  HWREG(SOC_I2C_1_REGS + I2C_SCLH) &=~ ((1<<7)|(1<<6)|(1<<5)|(1<<4)|(1<<3)|(1<<2)|(1<<1)|(1<<0));
  HWREG(SOC_I2C_1_REGS + I2C_SCLH) |= 0b1010;

  //Enable I2C Module again
  HWREG(SOC_I2C_1_REGS + I2C_CON) |= (1<<I2C_ENABLE_BIT);

  UARTprintf("I2C initialized! \n");
}

void
writetoi2c(unsigned int sla_address, unsigned char*data, unsigned char count,
    char stp)
{

	//UARTprintf("writeToI2C funktion läuft \n");
  int i = 0;

  HWREG(SOC_I2C_1_REGS + I2C_SA) = sla_address;

  for (i = 0; i < count + 1; ++i)
    {
      dataSnd[i] = data[i]; //deep copy vom array wegen Seiteneffekt
    }

  //UARTprintf("writeToI2C funktion läuft step1 \n");

  //set datalength
  HWREG(SOC_I2C_1_REGS + I2C_CNT) = (unsigned char) count;

  //reset golbal variable
  transCount = 0; //einfacher Zähler

  //enable master transmit Mode
  HWREG(SOC_I2C_1_REGS + I2C_CON) = I2C_CON_TRX | I2C_CON_MST | I2C_CON_I2C_EN;

  //enable transmit interrupt
  HWREG(SOC_I2C_1_REGS + I2C_IRQENABLE_SET) |= I2C_IRQSTATUS_XRDY; //NACHPRÜFEN!

  //Start communiction with Startcondition
  HWREG(SOC_I2C_1_REGS + I2C_CON) |= I2C_CON_STT; //query start condition (PDF s. 182)
  HWREG(SOC_I2C_1_REGS + I2C_CON) |= ((stp & 0x1) << I2C_CON_STP_SHIFT);

 // UARTprintf("writeToI2C funktion läuft step2 \n");
  //UARTprintf("writeToI2C: count: %d; transCount: %d \n", count, transCount);

  //Wait for all data to be transmitted
  while (transCount < count){
	  UARTprintf("writeToI2C: (WARTEFUNKTION) count: %d; transCount: %d \n", count, transCount);
  };

  //UARTprintf("writeToI2C funktion läuft step3 \n");

  //Disable Interrupt
  HWREG(SOC_I2C_1_REGS + I2C_IRQENABLE_CLR) = I2C_IRQSTATUS_XRDY;

  //UARTprintf("writeToI2C funktion läuft step4 \n");

  //Wait for Bus to close Communication
  delay_us(50);

}

void
readfromi2c(unsigned int sla_address, char*data, unsigned char count, char stp)
{
	int i;
  // TODO: implement read function here

  //set slave address
	HWREG(SOC_I2C_1_REGS + I2C_SA) = sla_address;

//	  for (i = 0; i < count + 1; ++i)
//		{
//		  dataRecv[i] = data[i]; //deep copy vom array wegen Seiteneffekt
//		}


  //set length of expected data
	HWREG(SOC_I2C_1_REGS + I2C_CNT) = (unsigned char) count;
  
  //reset global variables
	recvCount = 0;
	transCount = 0;

  //configure I2C controller in Master Receiver mode
	//HWREG(SOC_I2C_1_REGS + I2C_CON) &=~ ((1<<15)|(1<<10)|(1<<9));
	//HWREG(SOC_I2C_1_REGS + I2C_CON) |= (1<<15)|(1<<10); //bit 15 auf 1, bit 10 auf 1, bit 9 auf 0
	HWREG(SOC_I2C_1_REGS + I2C_CON) = I2C_CON_TRX_RCV | I2C_CON_MST | I2C_CON_I2C_EN;

  //enable receive interrupt
	//HWREG(SOC_I2C_1_REGS + I2C_IRQENABLE_SET) |= (1<<3); //Bit 3 auf 1 setzen -> ist RRDY_IE -> PDF S. 175
	HWREG(SOC_I2C_1_REGS + I2C_IRQENABLE_SET) |= I2C_IRQSTATUS_RRDY;

  //generate Start Condition over I2C bus
	HWREG(SOC_I2C_1_REGS + I2C_CON) |= I2C_CON_STT; //query start condition (PDF s. 182)
	HWREG(SOC_I2C_1_REGS + I2C_CON) |= ((stp & 0x1) << I2C_CON_STP_SHIFT); //stop condition

  //wait till data read
  while (recvCount < count){}; //hier wird die ISR ausgelöst

  //disable receive interrupt
  HWREG(SOC_I2C_1_REGS + I2C_IRQENABLE_CLR) = (1<<3); //clear receive data interrupt
	
  // copy received data from slave
  for (i = 0; i < count; i++)
    {
	  data[i] = dataRecv[i]; //deep copy vom array wegen Seiteneffekt
    }
	
  //Wait till Bus is free
  while ((HWREG(SOC_I2C_1_REGS + I2C_IRQSTATUS_RAW) & I2C_IRQSTATUS_RAW_BB)!= 0);

  delay_us(50);
}

void
CleanUpInterrupts(void)
{
  //clear and disable all Interrupts
  HWREG(SOC_I2C_1_REGS + I2C_IRQENABLE_SET) |= 0x7FF;
  HWREG(SOC_I2C_1_REGS + I2C_IRQSTATUS) = 0x7FF;
  HWREG(SOC_I2C_1_REGS + I2C_IRQENABLE_CLR) = 0x7FF;
}

void
I2CIsr(void)
{
  unsigned int status = 0;

  /* Get only Enabled interrupt status */
  status = ((HWREG(SOC_I2C_1_REGS + I2C_IRQSTATUS) ));

  /*
   ** Clear all enabled interrupt status except receive ready and
   ** transmit ready interrupt status
   */HWREG(SOC_I2C_1_REGS + I2C_IRQSTATUS) = (status
      & ~(I2C_IRQSTATUS_RRDY | I2C_IRQSTATUS_XRDY));

  if (status & I2C_IRQSTATUS_XRDY) //transmit-case
    {
      /* Put data to data transmit register of i2c */
      HWREG(SOC_I2C_1_REGS + I2C_DATA) = dataSnd[transCount++];
      //UARTprintf("TRANSMIT I2C INTERRUPT \n");
      /* Clear Transmit interrupt status */
      HWREG(SOC_I2C_1_REGS + I2C_IRQSTATUS) = I2C_IRQSTATUS_XRDY;
    }
	
  if (status & I2C_IRQSTATUS_RRDY) //receive-case
    {

	  // TODO: implement I2CIsr for data receive
	  /* Receive data from data receive register */
	  dataRecv[recvCount++] = HWREG(SOC_I2C_1_REGS + I2C_DATA); //count bestimmt Anzahl an Bytes
	  //UARTprintf("RECEIVE I2C INTERRUPT \n");

	  //HWREG(SOC_I2C_1_REGS + I2C_DATA) = dataRecv[recvCount++]; //Wie kopiert man REG-inhalt
	  //HWREG(SOC_I2C_1_REGS + I2C_DATA); //lesen aus dem data receive register (???)

	  /* Clear receive ready interrupt status */
	  HWREG(SOC_I2C_1_REGS + I2C_IRQSTATUS) = (1<<3);

    }
}

