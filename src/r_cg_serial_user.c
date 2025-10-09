/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2024 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_serial_user.c
* Version      : CodeGenerator for RL78/G14 V2.05.08.02 [03 Jun 2024]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for Serial module.
* Creation Date: 8/11/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_serial.h"
/* Start user code for include. Do not edit comment generated here */
#include "r_cg_adc.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt r_uart0_interrupt_send(vect=INTST0)
#pragma interrupt r_uart0_interrupt_receive(vect=INTSR0)
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
extern volatile uint8_t * gp_uart0_tx_address;         /* uart0 send buffer address */
extern volatile uint16_t  g_uart0_tx_count;            /* uart0 send data number */
extern volatile uint8_t * gp_uart0_rx_address;         /* uart0 receive buffer address */
extern volatile uint16_t  g_uart0_rx_count;            /* uart0 receive data number */
extern volatile uint16_t  g_uart0_rx_length;           /* uart0 receive data length */
/* Start user code for global. Do not edit comment generated here */
uint8_t uartBlockReadFlag = 4;
uint8_t dataOut[16];
uint8_t dataReadOut[10][4];
uint8_t receiveArray[8];
uartData_t receivedData;
uint8_t RXdataReceiver;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_uart0_interrupt_receive
* Description  : This function is INTSR0 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_uart0_interrupt_receive(void)
{
    volatile uint8_t rx_data;
    volatile uint8_t err_type;
    
    err_type = (uint8_t)(SSR01 & 0x0007U);
    SIR01 = (uint16_t)err_type;
    rx_data = RXD0;

    if (g_uart0_rx_length > g_uart0_rx_count)
    {
        *gp_uart0_rx_address = rx_data;
        gp_uart0_rx_address++;
        g_uart0_rx_count++;

        if (g_uart0_rx_length == g_uart0_rx_count)
        {
            r_uart0_callback_receiveend();
        }
    }
}

/***********************************************************************************************************************
* Function Name: r_uart0_interrupt_send
* Description  : This function is INTST0 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_uart0_interrupt_send(void)
{
    if (g_uart0_tx_count > 0U)
    {
        TXD0 = *gp_uart0_tx_address;
        gp_uart0_tx_address++;
        g_uart0_tx_count--;
    }
}

/***********************************************************************************************************************
* Function Name: r_uart0_callback_receiveend
* Description  : This function is a callback function when UART0 finishes reception.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void r_uart0_callback_receiveend(void)
{
    /* Start user code. Do not edit comment generated here */
	uartReceiveHandle(RXdataReceiver);
	R_UART0_Receive(&RXdataReceiver, 1);
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
void uartSendDataCallBack(uint8_t length, uint8_t command, uint8_t address,
		uint8_t *dataPtr) {
	uint8_t i = 0;
	uint8_t *ptr;
	ptr = dataPtr;
	if ((command <= 3) && (length >= 4)) {
		dataOut[0] = 0xBD;
		dataOut[1] = length;
		dataOut[1] = command;
		dataOut[2] = address;
		for (i = 3; i <= length - 1; i++) {
			dataOut[i] = *ptr;
			ptr++;
		}
		dataOut[length + 1] = checkSumCalcu(dataOut, length + 1);
		dataOut[length + 2] = 0xED;
		R_UART0_Send(dataOut, length + 3);
	}
}
void uartSendSysCallBack(uint8_t address, uint8_t data1, uint8_t data2) {
	dataOut[0] = 0xBD;
	dataOut[1] = 5;
	dataOut[2] = UART_COMMAND_SEND;
	dataOut[3] = address;
	dataOut[4] = data1;
	dataOut[5] = data2;
	dataOut[6] = checkSumCalcu(dataOut, 6);
	dataOut[7] = 0xED;
	R_UART0_Send(dataOut, 8);
}
void uartReceiveHandle(uint8_t data) {
	static uint8_t uartReceiveState = 0, i;
	static uint8_t length;
	uint8_t checkSum;
	receivedData.data = receiveArray;
	switch (uartReceiveState) {
	case 0: //check begin
		if (data == 0xBD)
			uartReceiveState = 1;
		break;
	case 1: //check length
		if ((data >= 3) && (data <= 7)) {
			length = data;
			receiveArray[0] = data;
			uartReceiveState = 2;
			i = 1;
		} else
			uartReceiveState = 0;
		break;
	case 2: //receive Data
		if (i <= length) {
			receiveArray[i] = data;
			i++;
		} else {
			if (data == 0xED) { //check data
				checkSum = 0xBD;
				for (i = 0; i < length; i++) {
					checkSum ^= receiveArray[i];
				}
				if (checkSum == receiveArray[length]) {
					//da check checksum OK
					receivedData.length = length - 3;
					receivedData.command = receiveArray[1];
					receivedData.address = receiveArray[2];
					for (i = 0; i < receivedData.length; i++)
						*receivedData.data = receiveArray[i + 3];
					switch (receivedData.command) {
					case UART_COMMAND_READ:
						if (receivedData.address != UART_ADDRESS_DATA) {
							dataOut[0] = 0xBD;
							dataOut[1] = dataReadOut[receivedData.address][0]; //length
							dataOut[2] = UART_COMMAND_SEND;
							dataOut[3] = receivedData.address;
							for (i = 4;
									i <= dataReadOut[receivedData.address][0];
									i++) {
								dataOut[i] = dataReadOut[receivedData.address][i
										- 3];
							}
							dataOut[i] = checkSumCalcu(dataOut, i);
							dataOut[i + 1] = 0xED;
							R_UART0_Send(dataOut, i + 2);
						} else {
							if (receivedData.data[0] <= 3) {
								uartBlockReadFlag = receivedData.data[0];
							}
						}
						break;
					case UART_COMMAND_SEND:
						break;
					}
				}
			}
			uartReceiveState = 0;
		}
		break;
	}
}
/***********************************************************************************************************************
 * Function Name: checkSumCalcu
 * Description  : This function calculate check sum of data in array with length.
 * Arguments    :
 1.arr: pointer to array to calcu checksum
 2.length: num of bytes to calcu checksum
 * Return Value : checksum of bytes
 ***********************************************************************************************************************/
uint8_t checkSumCalcu(uint8_t *arr, uint16_t length) {
	uint8_t *ptr;
	uint8_t checkSum = 0;
	uint16_t i = 0;
	ptr = arr;
	for (i = 0; i < length; i++) {
		checkSum ^= (*ptr);
		ptr++;
	}
	return checkSum;
}
/* End user code. Do not edit comment generated here */
