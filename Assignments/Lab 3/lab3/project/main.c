/*Shabrya Lott
  CPE 403
  Lab 3*/
#include <stdint.h>			//variable definitions for the C99 standard
#include <stdbool.h>		//Boolean definitions for the C99 standard
#include "inc/hw_memmap.h"		//macros defining the memory map of Tiva C Series
#include "inc/hw_types.h"		//defines common types and macros
#include "driverlib/sysctl.h"	//defines macros for System Control API of Driverlib
#include "driverlib/gpio.h"	//defines macros for GPIO API of Driverlib

uint8_t ui8PinData=6;	//unsigned 8-bt int that is used to cycle through LEDs

int main(void)
{
	//2000000 loop * 3 CPU CYCLES = 6000000 CPU CYCLE
	//6000000/ freq = 0.333sec => freq = 6000000/0.333 sec = 18.18MHZ
	//sets clock: xtal = 16Mhz,  400MHz PLL divided by 22
	SysCtlClockSet(SYSCTL_SYSDIV_11|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	//enables PORT F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//set 3 GPIO pins conneced to the LEDs as ouputs
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);

	while(1)
	{
		//turn on the LED specified in ui8PinData
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, ui8PinData);
		SysCtlDelay(2000000); //delay =  2000000 * 3  = 6000000 CPU cycles
		// turn all LEDs off
		GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);

		SysCtlDelay(2000000); //delay =  2000000 * 3  = 6000000 CPU cycles

		//blink two LEDs simultaneously in a sequence
		if(ui8PinData==6) {ui8PinData=10;} 			//	0110 (BR-purple)
		else if (ui8PinData==10) {ui8PinData=12;}	//  1010(RG-yellow)
		else {ui8PinData=6;} 						//	1100(GB-aqua)
	}
}

