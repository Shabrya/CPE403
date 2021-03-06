/*Shabrya Lott
 * Tiva_c Lab06
 * Usage: This is a simple program that test the Tiva C in low power modes
 */
#include <stdint.h>				//variable definitions for the C99 standard
#include <stdbool.h>			//Boolean definitions for the C99 standard
#include "utils/ustdlib.h"		//Prototypes for simple standard library functions
#include "inc/hw_types.h"		//defines common types and macros
#include "inc/hw_memmap.h"		//macros defining the memory map of Tiva C Series
#include "driverlib/sysctl.h"	//defines macros for System Control API of Driverlib
#include "driverlib/pin_map.h"	//Mapping of peripherals to pins for all parts
#include "driverlib/debug.h"		//Macros for assisting debug of the driver library.
#include "driverlib/hibernate.h" //API definition for the Hibernation module
#include "driverlib/gpio.h"		//defines macros for GPIO API of Driverlib

#ifdef DEBUG
void__error__(char *pcFilename, uint32_t u132Line)
{
}
#endif
int main(void)
{
	//set clock to 40MHz
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	//enable Port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//Port F pins 1,2,3 as outputs
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	//turn on green LED
    GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x08);

    //enable hibernation mode
	SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);
	//defines the clock supplied to the hibernation module
	HibernateEnableExpClk(SysCtlClockGet());

	//enable GPIO pin state to be maintained during hibernation
	//and remain active even when waking from hibernation
	HibernateGPIORetentionEnable();
	SysCtlDelay(64000000);		//4sec delay
	HibernateRTCSet(0);			//Reset the RTC to 0
	HibernateRTCEnable();		//turn the RTC on
	HibernateRTCMatchSet(0,5);	//set the wake up time for 5sec
	HibernateWakeSet(HIBERNATE_WAKE_PIN);	//set wake condition to the wake pin

	//turn off green LED before the device goes to sleep
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_3, 0x00);
	//requests the Hibernation module to disable he external regulator
	//removing power from the processor and all peripherals
	HibernateRequest();
	while(1)// infinite loop
	{
	}
}

