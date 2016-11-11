#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/uartstdio.c"



uint32_t ui32SysClock;

#define TSL2591_VISIBLE           (2)       // channel 0 - channel 1
#define TSL2591_INFRARED          (1)       // channel 1
#define TSL2591_FULLSPECTRUM      (0)       // channel 0

#define TSL2591_ADDR              (0x29)	//address register

#define TSL2591_COMMAND_BIT       (0xA0)    // 1010 0000: bits 7 and 5 for 'command normal'
#define TSL2591_WORD_BIT          (0x20)    // 1 = read/write word (rather than byte)
#define TSL2591_BLOCK_BIT         (0x10)    // 1 = using block read/write

#define TSL2591_ENABLE_POWERON    (0x01)	//This register turns on the TSL2591
#define TSL2591_ENABLE_AEN        (0x02)    // ALS Enable. This field activates ALS function. Writing a one activates the ALS. Writing a zero disables the ALS.
#define TSL2591_ENABLE_AIEN       (0x10)    // ALS Interrupt Enable. When asserted permits ALS interrupts to be generated, subject to the persist filter.
#define TSL2591_ENABLE_NPIEN      (0x80)    // No Persist Interrupt Enable. When asserted NP Threshold conditions will generate an interrupt, bypassing the persist filter

#define TSL2591_LUX_DF            (408.0F)
#define TSL2591_LUX_COEFB         (1.64F)  // CH0 coefficient
#define TSL2591_LUX_COEFC         (0.59F)  // CH1 coefficient A
#define TSL2591_LUX_COEFD         (0.86F)  // CH2 coefficient B
#define  TSL2591_REGISTER_ENABLE        (0x00)	//enable register
#define  TSL2591_REGISTER_CONTROL       (0x01)	//control bit register
#define  TSL2591_REGISTER_DEVICE_ID       (0x12)	//device id register
#define  TSL2591_REGISTER_CHAN0_LOW       (0x14)	//ch0	low byte register
#define  TSL2591_REGISTER_CHAN0_HIGH      (0x15)	//ch0	high byte register
#define  TSL2591_REGISTER_CHAN1_LOW       (0x16)	//ch1	low byte register
#define  TSL2591_REGISTER_CHAN1_HIGH      (0x17)	//ch1	high byte register


#define  TSL2591_INTEGRATIONTIME_100MS    (0x00) //time 100ms
#define  TSL2591_GAIN_MED             (0x10)    // medium gain (25x)




const bool DebuggingMode = true;



void InitConsole(void)
{

	//enable GPIO module
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    //configure PB0  as U1Rx
    GPIOPinConfigure(GPIO_PB0_U1RX);
    //configure PB1  as U1Tx
    GPIOPinConfigure(GPIO_PB1_U1TX);
    //enable UART1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    //SET UART1 to use the precision internal oscillator
    UARTClockSourceSet(UART1_BASE, UART_CLOCK_PIOSC);
    //set PB0 and PB1 function to UART
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    //set baudrate to 115200
    UARTStdioConfig(1, 115200, 16000000);

}


void i2c0_init()
{

	//enable I2C0 module
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //configure pin muxing for I2C0 functions on Port B3
    MAP_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);
    MAP_GPIOPinConfigure(GPIO_PB3_I2C0SDA);

    //Configure the pin muxing for I2C) functions on Port B2
    MAP_GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);
    MAP_GPIOPinConfigure(GPIO_PB2_I2C0SCL);

    // Enable and initialize the I2C0 master module.  Use the system clock for
    // the I2C0 module.  The last parameter sets the I2C data transfer rate.
    // If false the data rate is set to 100kbps and if true the data rate will
    // be set to 400kbps
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false);
    //wait for MCU to finish transaction
	while (I2CMasterBusy(I2C0_BASE));


}
uint8_t i2c0_read(uint8_t addr, uint16_t reg)
{
       uint8_t  x;	//unsigned 8-bit variable that will store the data fro the sensor

       	//specify that we want to communicate to device addr with an intended write to bus
       I2CMasterSlaveAddrSet(I2C0_BASE, addr, false);
       //specify reg to be read
       I2CMasterDataPut(I2C0_BASE, reg);
       //send control byte and reg addr byte to slave device
       I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_SEND);
       //wait for MCU & device to complete transaction
       	while (I2CMasterBusy(I2C0_BASE));
       //read from the specified slave device
       I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);
       //send control byte and read from the specified register
       I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_SINGLE_RECEIVE);\
       //wait for MCU & device to complete transaction
       while (I2CMasterBusy(I2C0_BASE));
       //get data from sepcified register
       x = I2CMasterDataGet(I2C0_BASE);
       //wait for MCU & device to complete transaction
       while (I2CMasterBusy(I2C0_BASE));

       return x;
}

void i2c0_write(uint8_t addr, uint16_t reg, uint8_t data)
{

	//specify that we want to communicate to device addr with an intended write to bus
	I2CMasterSlaveAddrSet(I2C0_BASE, addr, false);
	//register to be read
	I2CMasterDataPut(I2C0_BASE, reg);
	//send control byte and reg addr byte to slave device
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
	//wait for MCU & device to complete transaction
	while (I2CMasterBusy(I2C0_BASE));
	I2CMasterSlaveAddrSet(I2C0_BASE, addr, true);
	//specify data to be written to the above mentioned reg
	I2CMasterDataPut(I2C0_BASE, data);
	//wait while checking for MCU to complete the transaction
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
	//wait for MCU & device to complete transaction
	while (I2CMasterBusy(I2C0_BASE));
}

void TSL2591_init()
{
	//i2c0_write(TSL2591_ADDR, TSL2591_ADDR, 0x00);
    i2c0_read(TSL2591_ADDR, TSL2591_COMMAND_BIT|TSL2591_REGISTER_DEVICE_ID); //read device ID

    // Set Gain and Timing
    i2c0_write(TSL2591_ADDR,TSL2591_COMMAND_BIT|TSL2591_REGISTER_CONTROL , TSL2591_INTEGRATIONTIME_100MS|TSL2591_GAIN_MED );
    //Power on the sensor
    i2c0_write(TSL2591_ADDR,TSL2591_COMMAND_BIT|TSL2591_REGISTER_ENABLE, TSL2591_ENABLE_POWERON| TSL2591_ENABLE_AEN | TSL2591_ENABLE_AIEN | TSL2591_ENABLE_NPIEN);


}
void TSL2591_disable()
{
	i2c0_write(TSL2591_ADDR,TSL2591_COMMAND_BIT|TSL2591_REGISTER_ENABLE, 0x00);
}

float getLuminosity ()
{
	float    atime = 100.0F, again=25.0F; //For 100ms integration time and med gain
	float    cpl, lux1, lux2, lux;
	uint16_t ch0 = 0;
	uint16_t ch1 = 0;

	// Get full luminosity
	//read the high byte of channel 0
	ch0 = i2c0_read(TSL2591_ADDR, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_HIGH);
	ch0 <<= 8;
	//read the low byte of channel 0
	ch0 |= i2c0_read(TSL2591_ADDR, TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW);

	//read the low byte of channel 1
	ch1 = i2c0_read(TSL2591_ADDR,TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_HIGH);
	ch1 <<= 8;
	//read the high byte of channel 1
	ch1 |= i2c0_read(TSL2591_ADDR,TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW);



	//Calculate Lux value from sensor
	cpl = (atime * again) / TSL2591_LUX_DF;	  // cpl = (ATIME * AGAIN) / DF
  	lux1 = ( (float)ch0 - (TSL2591_LUX_COEFB * (float)ch1) ) / cpl;
  	lux2 = ( ( TSL2591_LUX_COEFC * (float)ch0 ) - ( TSL2591_LUX_COEFD * (float)ch1 ) ) / cpl;
  	// The highest value is the approximate lux equivalent
  	lux = lux1 > lux2 ? lux1 : lux2;
	return lux;
}

void ftoa(float f,char *buf)
{
       /*Function acquired from forum:
        * http://e2e.ti.com/support/microcontrollers/stellaris_arm/f/471/p/44193/156824.aspx
        */
    int pos=0,ix,dp,num;
    if (f<0)
    {
        buf[pos++]='-';
        f = -f;
    }
    dp=0;
    while (f>=10.0)
    {
        f=f/10.0;
        dp++;
    }
    for (ix=1;ix<8;ix++)
    {
            num = (int)f;
            f=f-num;
            if (num>9)
                buf[pos++]='#';
            else
                buf[pos++]='0'+num;
            if (dp==0) buf[pos++]='.';
            f=f*10.0;
            dp--;
    }
}

int main(void)
{
	//52 bit string of AT command to send data to thingkSpeak, 42 instr, 6 are lux. (append \n\r at end)
	char Lux[48] = "GET /update?key=KMCQAS1XI99ID4B8&field2=";
	float lux_read;	//float variabl that will store the lux value
	//set system clock frequency
	ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |SYSCTL_CFG_VCO_480), 120000000);
	//initialize UART
	InitConsole();
	//initialize i2C0
	i2c0_init();
	//initialize sensor
	TSL2591_init();

	while(1)
	{

		//re-init sensor
		TSL2591_init();
		SysCtlDelay(5000000);
		//get luminosity from the sensor
		lux_read = getLuminosity();
		//convert luminosity from a float to a string
		ftoa(lux_read, &Lux[40]);
		Lux[47]='\0';

		UARTprintf("\n");
		SysCtlDelay(20000000);
		//set CIPMUX = 1
		UARTprintf("AT+CIPMUX=1\n\r");
		SysCtlDelay(20000000);
		//connect to thingSpeak server
		UARTprintf("AT+CIPSTART=4,\"TCP\",\"184.106.153.149\",80\n\r");
		SysCtlDelay(20000000);
		//set channel id and size of data
		UARTprintf("AT+CIPSEND=4,50\n\r");
		SysCtlDelay(20000000);
		//send data to thingSpeak
		UARTprintf("%s\n\r",&Lux);
		SysCtlDelay(20000000);
		//send data to thingSpeak
		UARTprintf("%s\n\r",&Lux);
		//disable sensor
		TSL2591_disable();

	}
}
