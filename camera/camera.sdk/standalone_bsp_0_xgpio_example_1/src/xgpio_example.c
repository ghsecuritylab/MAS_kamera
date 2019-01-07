/******************************************************************************
 *
 * Copyright (C) 2002 - 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
 * @file xgpio_example.c
 *
 * This file contains a design example using the AXI GPIO driver (XGpio) and
 * hardware device.  It only uses channel 1 of a GPIO device and assumes that
 * the bit 0 of the GPIO is connected to the LED on the HW board.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00a rmm  03/13/02 First release
 * 1.00a rpm  08/04/03 Removed second example and invalid macro calls
 * 2.00a jhl  12/15/03 Added support for dual channels
 * 2.00a sv   04/20/05 Minor changes to comply to Doxygen and coding guidelines
 * 3.00a ktn  11/20/09 Minor changes as per coding guidelines.
 * 4.1   lks  11/18/15 Updated to use canonical xparameters and
 *		      clean up of the comments and code for CR 900381
 * 4.3   sk   09/29/16 Modified the example to make it work when LED_bits are
 *                     configured as an output. CR# 958644
 *       ms   01/23/17 Added xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 *
 * </pre>
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"
#include <stdio.h>
#include "sleep.h"
#include "xgpio.h"
#include "camera_reg.h"

/************************** Constant Definitions *****************************/

#define IIC_ADDRESS            0x21

int I2C_write(u8 register_offset, u8 write_value);
int I2C_read(u8 register_offset, u8 *read_value);
int load_regs(struct regval_list *regs);

XIicPs Iic;
#define CAMERA_DEVICE_ID  XPAR_GPIO_1_DEVICE_ID

XGpio Gpio;
XGpio Gpio2;
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define GPIO_EXAMPLE_DEVICE_ID  XPAR_GPIO_1_DEVICE_ID


/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifdef PRE_2_00A_APPLICATION

/*
 * The following macros are provided to allow an application to compile that
 * uses an older version of the driver (pre 2.00a) which did not have a channel
 * parameter. Note that the channel parameter is fixed as channel 1.
 */
#define XGpio_SetDataDirection(InstancePtr, DirectionMask) \
        XGpio_SetDataDirection(InstancePtr, LED_CHANNEL, DirectionMask)

#define XGpio_DiscreteRead(InstancePtr) \
        XGpio_DiscreteRead(InstancePtr, LED_CHANNEL)

#define XGpio_DiscreteWrite(InstancePtr, Mask) \
        XGpio_DiscreteWrite(InstancePtr, LED_CHANNEL, Mask)

#define XGpio_DiscreteSet(InstancePtr, Mask) \
        XGpio_DiscreteSet(InstancePtr, LED_CHANNEL, Mask)

#endif

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */

XGpio Gpio; /* The Instance of the GPIO Driver */

/*****************************************************************************/
/**
 *
 * The purpose of this function is to illustrate how to use the GPIO
 * driver to turn on and off an LED.
 *
 * @param	None
 *
 * @return	XST_FAILURE to indicate that the GPIO Initialization had
 *		failed.
 *
 * @note		This function will not return if the test is running.
 *
 ******************************************************************************/
int main(void) {
	int status;

	XIicPs_Config *Config;

	u32 InputData;

	/* Initialize the GPIO driver */
	status = XGpio_Initialize(&Gpio, GPIO_EXAMPLE_DEVICE_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Gpio Initialization Failed\r\n");
		return XST_FAILURE;
	}
	XGpio_SetDataDirection(&Gpio, 1, 0x7ff);

	Config = XIicPs_LookupConfig(XPAR_XIICPS_0_DEVICE_ID);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	status = XIicPs_SelfTest(&Iic);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_SetSClk(&Iic, 100000);

	printf("PS I2C Initialized\n\r");

	status = I2C_write(0x12, 0x80);
	if (status == XST_SUCCESS)
		printf("succ\n\r");
	else
		printf("fail\n\r");

	usleep(100000);
	status = load_regs(ov7670_default_regs);
	if (status == XST_SUCCESS)
		printf("init\n\r");
	else
		printf("fail\n\r");

	usleep(100000);

	status = load_regs(ov7670_test_shift);
		if (status == XST_SUCCESS)
			printf("init\n\r");
		else
			printf("fail\n\r");

		usleep(100000);

	int h = 0,i=0;
	u32 ar[700];
	while (!(XGpio_DiscreteRead(&Gpio, 1) & 0x400));
	while ((XGpio_DiscreteRead(&Gpio, 1) & 0x400));
	while (!(XGpio_DiscreteRead(&Gpio, 1) & 0x200));
	while (1) {
		InputData = XGpio_DiscreteRead(&Gpio, 1);
		if (!(InputData & 0x200))
			break;
		if (!(InputData & 0x100))
			continue;
		h++;
		ar[i]=InputData;
		i++;
		while ((XGpio_DiscreteRead(&Gpio, 1) & 0x100));

	}
	printf("%d\n", h);

	h=0;
	while (!(XGpio_DiscreteRead(&Gpio, 1) & 0x400));
	while ((XGpio_DiscreteRead(&Gpio, 1) & 0x400));
	while (1) {
		InputData = XGpio_DiscreteRead(&Gpio, 1);
		if (InputData & 0x400)
			break;
		if (!(InputData & 0x200))
			continue;
		h++;
		while ((XGpio_DiscreteRead(&Gpio, 1) & 0x200))
			;

	}
	printf("%d\n", h);

	printf("Testing Complete\n\r");

	return 0;
}

int I2C_write(u8 register_offset, u8 write_value)

{
	int Status = XST_SUCCESS;
	u8 TxBuffer[128]; // Only need this to be size 2, but making larger for future use

	TxBuffer[0] = register_offset;  // Offset of register to write
	TxBuffer[1] = write_value;  // value to write there

	Status = XIicPs_MasterSendPolled(&Iic, TxBuffer, 2, IIC_ADDRESS);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	while (XIicPs_BusIsBusy(&Iic))
		;

	return (Status);
}

int I2C_read(u8 register_offset, u8 *read_value)

{
	int Status = XST_SUCCESS;
	u8 TxBuffer[128];
	u8 RxBuffer[128];

	TxBuffer[0] = register_offset;

	Status = XIicPs_MasterSendPolled(&Iic, TxBuffer, 1, IIC_ADDRESS);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;
	//Wait until bus is idle to start another transfer.
	while (XIicPs_BusIsBusy(&Iic)) {/* NOP */
	}

	Status = XIicPs_MasterRecvPolled(&Iic, RxBuffer, 1, IIC_ADDRESS);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if (Status == XST_SUCCESS)
		*read_value = RxBuffer[0];

	//usleep(100000); // Delay 100 ms, which is 100K us
	while (XIicPs_BusIsBusy(&Iic))
		;
	return (Status);
}

int load_regs(struct regval_list *regs) {
	int status, i = 0;

	while (1) {
		if (regs[i].reg_num == 0xff && regs[i].value == 0xff)
			break;
		status = I2C_write(regs[i].reg_num, regs[i].value);
		if (status != XST_SUCCESS)
			return status;
		i++;
		usleep(10000);
	}

	return status;
}