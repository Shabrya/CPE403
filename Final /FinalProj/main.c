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

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "ThingSpeakLinux.h"

void TSL2591_Init();				//Function initializes the sensor
void i2c_write(int  fd2, unsigned char buf);	//Function writes the i2c BUS
void i2c_read(int  fd2, unsigned char* buf);	//Function  reads the i2c BUS
float getFullLuminosity();			//Reads the Lux raw value from the sensor
void ftoa(float f, char *buf);
void pauseSec(int sec);				//Delay function

int fd1,fd2;
int main(void)
{
	int data[1];
	float lux_read;
	if((fd2 =  open("/dev/i2c-1", O_WRONLY|O_NOCTTY)) <0)
		printf("Unable to open Uart1 at tty01\n");

	if((fd2 =  open("/dev/i2c-1", O_RDWR)) <0)
		printf("i2c failed to open \n");
	if(ioctl(fd2, I2C_SLAVE, TSL2591_ADDR)<0)
		printf("failed to aquire bus \n");

	while(1)
	{
        	pauseSec(1);
		//Initialize sensor
  		TSL2591_Init();
		//Get  luminosity
		lux_read = getFullLuminosity();
		data[0] = (int)lux_read;
		//send data to ThingSpeak.com
		printf("Sending Data to ThingSpeak.com...\n");
		//send data and API key to thingSpeak Function
		SendDataToThingSpeak(1, &data[0],"KMCQAS1XI99ID4B8",16);		
		pauseSec(10);			//delay
		printf("Data Sent \n");
	
	}
	return 0;
}

void TSL2591_Init()
{
	unsigned char buf[2] = {0x00, 0x00};
	//read device ID
	buf[0] = TSL2591_COMMAND_BIT|TSL2591_REGISTER_DEVICE_ID; 
	i2c_write(fd2, buf[0]);
	i2c_read(fd2, buf);
	printf("Device ID = %d\n", buf[0]);

	//set Gain and Timing
	buf[0] = TSL2591_COMMAND_BIT|TSL2591_REGISTER_CONTROL;
	buf[1] = TSL2591_INTEGRATIONTIME_100MS|TSL2591_GAIN_MED;
	i2c_write(fd2, buf[0]);
	pauseSec(1);
	i2c_write(fd2, buf[1]);

	//Power on the sensor
	buf[0]= TSL2591_COMMAND_BIT|TSL2591_REGISTER_ENABLE;
	buf[1]=TSL2591_ENABLE_POWERON| TSL2591_ENABLE_AEN;
	i2c_write(fd2, buf[0]);
        pauseSec(1);
        i2c_write(fd2, buf[1]);
}

void i2c_write(int  fd2, unsigned char buf)
{
	//write 1 byte
	if(write(fd2, &buf,1) !=1)
	{
		printf("i2c write failed\n");
	}
}
void i2c_read(int  fd2, unsigned char* buf)
{
	//read 1 byte
	if(read(fd2,buf,1)!=1)
	{
		printf("i2c read failed\n");
	}
}
float getFullLuminosity()
{
	float    atime = 100.0F, again=25.0F; //For 100ms integration time and med gain
	float    cpl, lux1, lux2, lux;
	uint16_t ch0, ch1;
	unsigned char buf[2];
	//set channel 0 registers to  be read
	buf[0] = TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_HIGH;
        buf[1] = TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW;
	//Read High Byte of Channel 0
	i2c_write(fd2, buf[0]);
        i2c_read(fd2, buf);
	ch0 = buf[0];
	ch0 <<= 8;
	//Read Low Byte of Channel 0
	i2c_write(fd2, buf[1]);
        i2c_read(fd2, buf);
	ch0 |= buf[0];
	//set  channel 0 register to be read
	buf[0] = TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_HIGH;
        buf[1] = TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN1_LOW;
        //Read High Byte of Channnel 1
	i2c_write(fd2, buf[0]);
        i2c_read(fd2, buf);
        ch1 = buf[0];
        ch1 <<= 8;
	//Read Low Byte of Channel 1
        i2c_write(fd2, buf[1]);
        i2c_read(fd2, buf);
        ch1 |= buf[0];
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
void pauseSec(int sec)
{
	time_t now,later;

	now = time(NULL);
	later = time(NULL);

	while((later - now) < (double)sec)
		later = time(NULL);
}
