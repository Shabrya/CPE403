/*Shabrya Lott
 * Tiva_c Lab08
 * Usage: This is a simple program that reads and writes to the EEPROM
 */
#include <stdint.h>			//variable definitions for the C99 standard
#include <stdbool.h>		//Boolean definitions for the C99 standard
#include "inc/hw_types.h"		//defines common types and macros
#include "inc/hw_memmap.h"		//macros defining the memory map of Tiva C Series
#include "driverlib/sysctl.h"	//defines macros for System Control API of Driverlib
#include "driverlib/pin_map.h"	// Mapping of peripherals to pins for all parts
#include "driverlib/debug.h"	//Macros for assisting debug of the driver library
#include "driverlib/gpio.h"		//defines macros for GPIO API of Driverlib
#include "driverlib/flash.h"	//Prototypes for the flash driver
#include "driverlib/eeprom.h"	//Prototypes for the EEPROM drive

int main(void)
{
	uint32_t pui32Data[2];
	uint32_t pui32Read[2];
	pui32Data[0] = 0x12345678;
	pui32Data[1] = 0x56789abc;

	//set clock to 40MHz
	SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	//enable Port F
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//set PF1, PF2,and PF3 as outputs
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
	//clear PF1, PF2, PF3
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x00);
	//delay for about 2 sec
	SysCtlDelay(20000000);
	//erase block of flash
	FlashErase(0x10000);
	//Programs the data array to the start of the block, of the length of the array
	FlashProgram(pui32Data, 0x10000, sizeof(pui32Data));
	//set PF2 high
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x02);
	//delay for about 2 sec
	SysCtlDelay(20000000);

	//enable EEPROM peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	//performs recovery if power failed during prev write operation
	EEPROMInit();
	//erase entire EEPROM
	EEPROMMassErase();
	//reads erased values into pulRead (offset addr)
	EEPROMRead(pui32Read, 0x0, sizeof(pui32Read));
	//programs data array to the beginning of EEPROM of the length of the array
	EEPROMProgram(pui32Data, 0x0, sizeof(pui32Data));
	//reads data into array pulRead
	EEPROMRead(pui32Read, 0x0, sizeof(pui32Read));
	//Turns off red LED and turns on blue LED
	GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0x04);

	while(1)	//infinite loop
	{
	}
}
