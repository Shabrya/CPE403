/*Shabrya Lott
 * Tiva_c Lab03
 * Usage:This is a simple program that enables toggling on the TIVA_C launchpad
 * Input: NONE
 * Output: Red, Blue and Green LED's lit in specified cycle
 */
#include <stdint.h>			//variable definitions for the C99 standard
#include <stdbool.h>		//Boolean definitions for the C99 standard
#include "inc/hw_memmap.h"		//macros defining the memory map of Tiva C Series
#include "inc/hw_types.h"		//defines common types and macros
#include "driverlib/sysctl.h"	//defines macros for System Control API of Driverlib
#include "driverlib/gpio.h"	//defines macros for GPIO API of Driverlib

uint8_t ui8PinData=2;	//unsigned 8-bt int that is used to cycle through LEDs

int main(void)
{
	//sets clock: xtal = 16Mhz,  400MHz PLL divided by 10
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
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
		//set ui8PinData to the next LED color in the sequence
		//0010 (red), 0100 (blue), 1000 (green), ...
		if(ui8PinData==8) {ui8PinData=2;} else {ui8PinData=ui8PinData*2;}
	}
}

