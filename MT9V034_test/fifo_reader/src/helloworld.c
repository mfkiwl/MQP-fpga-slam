/*
 * Copyright (c) 2009 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 */

#include <stdio.h>
#include "platform.h"
#include "xparameters.h"
#include "xiomodule.h"

// GPO1
#define GETDATA (1<<2) // load a new line of pixels into the FPGA buffer
#define RRST (1<<1) // reset to address 0
#define LED (1<<0) // LED indicator
// GPI2
#define SW_READ (1<<1)
#define BUF_READY (1<<0)

void print(char *str);
void _EXFUN(xil_printf, (const char*, ...));


int main()
{
    init_platform();
    int pixel_position = 0,row = 0;;
    u8 data=0x00,GPO1=0x00,GPI2=0x00,swState=0x00,prevState=0x00;

    XIOModule gpi;
    XIOModule gpo;

    // GPI1 = pixel_value(7:0)
    XIOModule_Initialize(&gpi, XPAR_IOMODULE_0_DEVICE_ID);
    XIOModule_Start(&gpi);

    // GPO1 = (GETDATA)|(RRST)|(LED)
    XIOModule_Initialize(&gpo, XPAR_IOMODULE_0_DEVICE_ID);
    XIOModule_Start(&gpo);

    print("\n\rMT9V034 controller and AL422B FIFO reader\n\r");
    while(1)
    {
    	// get switch position
    	GPI2 = XIOModule_DiscreteRead(&gpi,2);

    	if((GPI2&SW_READ)!=0){
			swState = 1;
			if (row >= 480) GPO1 &=~(LED);
			else GPO1 |= LED;
			GPO1 &=~(RRST|GETDATA);
		    XIOModule_DiscreteWrite(&gpo,1,GPO1);
    	}else{
			GPO1 &= ~(RRST|LED|GETDATA);
			XIOModule_DiscreteWrite(&gpo,1,GPO1);
			row = 0;
			swState = 0;
    	}

    	// code below runs only once based on SW state change
    	if (prevState != swState){
    		if(swState){
    			print("\n\rReading from FIFO...\n\r");

    			GPO1 |= (RRST); // reset FIFO position to 0th index
    			GPO1 &=~ (GETDATA); // make sure we're not trying to read data
    		    XIOModule_DiscreteWrite(&gpo,1,GPO1);
    			pixel_position = 0;
    			GPO1 &=~(RRST);
    			XIOModule_DiscreteWrite(&gpo,1,GPO1);
    			GPO1 |= (GETDATA); // make sure we're not trying to read data
    			XIOModule_DiscreteWrite(&gpo,1,GPO1);

    			u32 pixelsRead = 0;

    			while(row<480){
    				// make sure read_sw hasn't been turned off
    				GPI2 = XIOModule_DiscreteRead(&gpi,2);
    			    if ((GPI2&SW_READ)==0) break;

    				u8 i=0;
    				// check to see if BUF_READY is good to go
    		    	GPI2 = XIOModule_DiscreteRead(&gpi,2);
    		    	// wait until it is
    				while((GPI2&BUF_READY)==0){
    					if(i==0){
    						i++;
    						//print("\n\t buffer not ready \n\r");
    					}
    					GPI2 = XIOModule_DiscreteRead(&gpi,2);
    				}
    				GPO1 &=~ (GETDATA); // make sure we're not trying to read data
    				XIOModule_DiscreteWrite(&gpo,1,GPO1);

    				while (pixel_position < 752){
    					// update pixel position for FPGA buffer
    					XIOModule_DiscreteWrite(&gpo,2,pixel_position);

    					// make sure read_sw hasn't been turned off
        		    	GPI2 = XIOModule_DiscreteRead(&gpi,2);
        		    	if ((GPI2&SW_READ)==0) break;

        		    	// read value at pixel position from FPGA buffer
						data = XIOModule_DiscreteRead(&gpi,1);

						//print the value
						xil_printf("%d\n\r",data);

						// increment to the next pixel position
						pixel_position++;
						pixelsRead ++;
    				}
    				// moved up from after GPO1 write to add a slight delay
    				pixel_position = 0;
    				row++;
    				// signal to the FPGA that we want more data!
    				GPO1 |= (GETDATA);
    				XIOModule_DiscreteWrite(&gpo,1,GPO1);

    				//xil_printf("Row: %d",row);

    			}
    			//xil_printf("%d Pixels Read by MCS",pixelsRead);
    		} else {
    			GPO1 &=~(GETDATA);
    			GPO1 |= RRST;
    		    XIOModule_DiscreteWrite(&gpo,1,GPO1);
    			print("\n\rReset for new sequence\n\r");
    		}
    	}

    	prevState = swState; // update prev switch position
    }
    cleanup_platform();
    return 0;
}
