/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
//#include "xil_printf.h"
//#include "xparameters.h"
//#include "xiicps.h"
//#include "xiomodule.h"

/************************** Constant Definitions ******************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID


/**************************** Type Definitions ********************************/

/************************** Function Prototypes *******************************/

int IicPsSelfTestExample(u16 DeviceId);

/************************** Variable Definitions ******************************/

XIicPs Iic;			/* Instance of the IIC Device */

typedef enum
{
	DELAY = 0,
	TX_COMMAND = 1,
	RX_DATA = 2,
	POLL_SEND = 3,	// for debug purposes only
}UART_STATE;


int main()
{
    init_platform();

	int Status;

//	xil_printf("IIC Self Test Example \r\n");

	/*
	 * Run the Iic Self Test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
	Status = IicPsSelfTestExample(IIC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Self Test Failed\r\n");
		return XST_FAILURE;
	}

//	xil_printf("Successfully ran IIC Self Test Example Test\r\n");

    char echo[11];
    char status[6];
    char data[66][24];
    char linefeed[4];
	int delay_counter = 0;

    UART_STATE STATE = DELAY;

    while(1)
    {
		switch(STATE)
		{
			case DELAY:
			{
				while(delay_counter < 400)
				{
					delay_counter++;
				}

				delay_counter = 0;
				STATE = TX_COMMAND;
				break;
			}

			case TX_COMMAND:
			{
				print("L0\n");
				STATE = RX_DATA;
				break;
			}

/*			case RX_DATA:
			{
				int rx_line = 0;	// counts which data line is being received

				// receives echo
				echo[0] = inbyte();	// blocking - inbyte is polled
				xil_printf("received echo");

				// receives status
				status[0] = inbyte();	// blocking - inbyte is polled
				xil_printf("received status");

				// receives 24 data blocks
				for(rx_line = 0; rx_line < 24; rx_line++)
				{
					data[0][rx_line] = inbyte();	// blocking - inbyte is polled
				}

				linefeed[0] = inbyte();	// blocking - inbyte is polled

			    echo[10] = '\0';
			    status[5] = '\0';

			    for(rx_line = 0; rx_line < 24; rx_line++)
			    {
			    	data[65][rx_line] = '\0';	// blocking - inbyte is polled
			    }

			    linefeed[3] = '\0';

				STATE = POLL_SEND;
				break;
			}

			case POLL_SEND:
			{
				int rx_line = 0;	// counts which data line is being received

				inbyte();

				xil_printf("sending data\n");

				xil_printf("%s",echo);
				xil_printf("%s",status);
				for(rx_line = 0; rx_line < 24; rx_line++)
				{
					xil_printf("%s",data[rx_line]);
				}
				xil_printf("%s",linefeed);

				STATE = DELAY;
				break;
			}
*/

			case RX_DATA:
			{
				int rx_index = 0;	// indexes across arrays
				int rx_line = 0;	// counts which data line is being received

				// receives echo
				for(rx_index = 0; rx_index < 10; rx_index++)
				{
					echo[rx_index] = inbyte();	// blocking - inbyte is polled
				}

				// receives status
				for(rx_index = 0; rx_index < 5; rx_index++)
				{
					status[rx_index] = inbyte();	// blocking - inbyte is polled
				}

				// receives 24 data blocks
				for(rx_line = 0; rx_line < 24; rx_line++)
				{
					for(rx_index = 0; rx_index < 65; rx_index++)
					{
						data[rx_index][rx_line] = inbyte();	// blocking - inbyte is polled
					}
				}

				for(rx_index = 0; rx_index < 3; rx_index++)
				{
					linefeed[rx_index] = inbyte();	// blocking - inbyte is polled
				}

				STATE = POLL_SEND;
				break;
			}

			case POLL_SEND:
			{
				int rx_line = 0;	// counts which data line is being received

				inbyte();

				xil_printf("sending data\n");

			    echo[10] = '\0';
			    status[5] = '\0';

			    for(rx_line = 0; rx_line < 24; rx_line++)
			    {
			    	data[65][rx_line] = '\0';	// blocking - inbyte is polled
			    }

			    linefeed[3] = '\0';


				printf("%s", echo);
				printf("%s",status);
			    for(rx_line = 0; rx_line < 24; rx_line++)
			    {
			    	printf("%s",data[rx_line]);
			    }
			    printf("%s", linefeed);


/*				// transmits echo
				for(tx_index = 0; tx_index < 10; tx_index++)
				{
					print(&echo[tx_index]);
				}

				// transmits status
				for(tx_index = 0; tx_index < 5; tx_index++)
				{
					print(&status[tx_index]);
				}

				// transmits data blocks
				for(tx_line = 0; tx_line < 24; tx_line++)
				{
					for(tx_index = 0; tx_index < 65; tx_index++)
					{
						print(&data[tx_index][tx_line]);
					}
				}

				// transmits line feed
				for(tx_index = 0; tx_index < 3; tx_index++)
				{
					print(&linefeed[tx_index]);
				}
*/
				STATE = DELAY;
				break;
			}

		}
    }

    cleanup_platform();
    return 0;
}

/*****************************************************************************/
/**
*
* This function does a minimal test on the Iic device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XIicPs component.
*
*
* @param	DeviceId is the Device ID of the IicPs Device and is the
*		XPAR_<IICPS_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None.
*
*
*******************************************************************************/
int IicPsSelfTestExample(u16 DeviceId)
{
	int Status;
	XIicPs_Config *Config;

	/*
	 * Initialize the IIC driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test.
	 */
	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
