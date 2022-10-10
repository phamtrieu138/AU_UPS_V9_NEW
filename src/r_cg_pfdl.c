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
* Copyright (C) 2011, 2021 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_pfdl.c
* Version      : CodeGenerator for RL78/G14 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for PFDL module.
* Creation Date: 10/7/2022
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_pfdl.h"
/* Start user code for include. Do not edit comment generated here */
#include <stdio.h>
#include <string.h>
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
void commandDataFlashEXE(void);
void chainInitAllData(void);
void chainReadBlockX(uint8_t blockno);
void chainWriteBlockX(uint8_t blockno);
uint8_t checkDataFlashState(void);
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
pfdl_status_t gFdlResult;    /* Return value */
pfdl_request_t gFdlReq;      /* Control variable for PFDL */
pfdl_descriptor_t gFdlDesc;
uint8_t gFdlStatus;	         /* This indicates status of FDL library that is "close" or "open". (open=1, close=0) */
/* Start user code for global. Do not edit comment generated here */
pfdl_request_t *requestPtr = NULL;
uint8_t bufferForDataFlash[4], dataFlashState = DF_IDLE;
pfdl_request_t dataFlashStep[7];
uint8_t data1024Arr[1024];
typedef struct {
	uint8_t index;
	uint16_t pos1, pos2, len1, len2;
	uint8_t *buff1;
	uint8_t *buff2;
	uint8_t currentState;
} block_t;
block_t block0, blockX;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_FDL_Create
* Description  : This function initializes the flash data library.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_FDL_Create(void)
{
    gFdlDesc.fx_MHz_u08 = _16_HOCO_CLOCK_MHz;   /* Set an integer of the range from 1 to 32 according to GUI setting of HOCO. */
    gFdlDesc.wide_voltage_mode_u08 = _WIDE_VOLTAGE_MODE; /* Voltage mode */
}

/***********************************************************************************************************************
* Function Name: R_FDL_Write
* Description  : This function writes a data to the RL78 data flash memory.
* Arguments    : index -
*                    It is destination address of flash memory for writing a data. The address range is from 0x0000 to 0x0FFF
*                buffer -
*                    The top address of data to write
*                bytecount -
*                    The size of data to write (Unit is byte)
* Return Value : pfdl_status_t -
*                    status of write command
***********************************************************************************************************************/
pfdl_status_t R_FDL_Write(pfdl_u16 index, __near pfdl_u08* buffer, pfdl_u16 bytecount)
{
    if (gFdlStatus == 1)
    {
        gFdlReq.index_u16     = index;
        gFdlReq.data_pu08     = buffer;
        gFdlReq.bytecount_u16 = bytecount;
        gFdlReq.command_enu   = PFDL_CMD_WRITE_BYTES;
        gFdlResult = PFDL_Execute(&gFdlReq);
        /* Wait for completing command */
        while (gFdlResult == PFDL_BUSY)
        {
            NOP();
            NOP();
            gFdlResult = PFDL_Handler();     /* The process for confirming end */
        }
    }
    else
    {
        gFdlResult = PFDL_ERR_PROTECTION;
    }
    return gFdlResult;
}

/***********************************************************************************************************************
* Function Name: R_FDL_Read
* Description  : This function reads a data flash memory.
* Arguments    : index -
*                    It is destination address of flash memory for reading a data. The address range is from 0x0000 to 0x0FFF
*                buffer -
*                    The top address of data to read
*                bytecount -
*                    The size of data to read (Unit is byte)
* Return Value : pfdl_status_t -
*                    status of read command
***********************************************************************************************************************/
pfdl_status_t R_FDL_Read(pfdl_u16 index, __near pfdl_u08* buffer, pfdl_u16 bytecount)
{
    if (gFdlStatus == 1)
    {
         gFdlReq.index_u16     = index;
         gFdlReq.data_pu08     = buffer;
         gFdlReq.bytecount_u16 = bytecount;
         gFdlReq.command_enu   = PFDL_CMD_READ_BYTES;
         gFdlResult = PFDL_Execute(&gFdlReq);
     }
     else 
     {
         gFdlResult = PFDL_ERR_PROTECTION;
     }
     return gFdlResult;
}

/***********************************************************************************************************************
* Function Name: R_FDL_Erase
* Description  : This function erases a block of data flash.
* Arguments    : blockno -
*                    The block number to erase data flash. The range of block number is from 0 to 3
* Return Value : pfdl_status_t -
*                    status of erase command
***********************************************************************************************************************/
pfdl_status_t R_FDL_Erase(pfdl_u16 blockno)
{
    if (gFdlStatus == 1)
    {
        gFdlReq.index_u16     = blockno;
        gFdlReq.command_enu   = PFDL_CMD_ERASE_BLOCK;
        gFdlResult = PFDL_Execute(&gFdlReq);
        /* Wait for completing command  */
        while(gFdlResult == PFDL_BUSY)
        {
            NOP(); 
            NOP();
            gFdlResult = PFDL_Handler();     /* The process for confirming end */
        }
    }
    else 
    {
        gFdlResult = PFDL_ERR_PROTECTION;
    }
    return gFdlResult;
}

/***********************************************************************************************************************
* Function Name: R_FDL_Open
* Description  : This function opens the RL78 data flash library.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_FDL_Open(void)
{
    gFdlDesc.fx_MHz_u08 = _16_HOCO_CLOCK_MHz;   /* Set an integer of the range from 1 to 32 according to GUI setting of HOCO. */
    gFdlDesc.wide_voltage_mode_u08 = _WIDE_VOLTAGE_MODE; /* Voltage mode */
    PFDL_Open(&gFdlDesc);
    gFdlStatus = 1;
}

/***********************************************************************************************************************
* Function Name: R_FDL_Close
* Description  : This function closes the RL78 data flash library.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_FDL_Close(void)
{
    PFDL_Close();
    gFdlStatus = 0;
}

/***********************************************************************************************************************
* Function Name: R_FDL_BlankCheck
* Description  : This function blank check a data to the RL78 data flash memory.
* Arguments    : index -
*                    It is destination address of flash memory for blank check. The address range is from 0x0000 to 0x0FFF
*                bytecount -
*                    The size of data to blank check (Unit is byte)
* Return Value : pfdl_status_t -
*                    status of blank check command
***********************************************************************************************************************/
pfdl_status_t R_FDL_BlankCheck(pfdl_u16 index, pfdl_u16 bytecount)
{
    if (gFdlStatus == 1)
    {
        gFdlReq.index_u16 = index;
        gFdlReq.bytecount_u16 = bytecount;
        gFdlReq.command_enu = PFDL_CMD_BLANKCHECK_BYTES;
        gFdlResult = PFDL_Execute(&gFdlReq);
        /* Wait for completing command */
        while(gFdlResult == PFDL_BUSY)
        {
            NOP();
            NOP();
            gFdlResult = PFDL_Handler();     /* The process for confirming end */
        }
    }
    else
    {
        gFdlResult = PFDL_ERR_PROTECTION;
    }
    return gFdlResult;
}

/***********************************************************************************************************************
* Function Name: R_FDL_IVerify
* Description  : This function writes a data to the RL78 data flash memory.
* Arguments    : index -
*                    It is destination address of flash memory for iverify a data. The address range is from 0x0000 to 0x0FFF
*                bytecount -
*                    The size of data to iverify (Unit is byte)
* Return Value : pfdl_status_t -
*                    status of iverify command
***********************************************************************************************************************/
pfdl_status_t R_FDL_IVerify(pfdl_u16 index, pfdl_u16 bytecount)
{
    if (gFdlStatus == 1)
    {
        gFdlReq.index_u16 = index;
        gFdlReq.bytecount_u16 = bytecount;
        gFdlReq.command_enu = PFDL_CMD_IVERIFY_BYTES;
        gFdlResult = PFDL_Execute(&gFdlReq);
        /* Wait for completing command */
        while(gFdlResult == PFDL_BUSY)
        {
            NOP();
            NOP();
            gFdlResult = PFDL_Handler();     /* The process for confirming end */
        }
    }
    else
    {
        gFdlResult = PFDL_ERR_PROTECTION;
    }
    return gFdlResult;
}

/* Start user code for adding. Do not edit comment generated here */

//write 4 byte on-line
uint8_t chainManager(void) {
	static uint16_t pos;
	switch (dataFlashState) {
	case DF_READINGBLOCK0: //read position
		if (gFdlStatus == 0) {
			chainReadBlockX(0);
			dataFlashState = DF_STARTWRITEBLOCK0;
		}
		break;
	case DF_STARTWRITEBLOCK0: //start writing block 0
		if (gFdlStatus == 0) {
			pos = (data1024Arr[0] << 8) + data1024Arr[1];
			if (((pos % 4) > 0) || (pos == 0)) { //error or init state
				data1024Arr[4] = bufferForDataFlash[0];
				data1024Arr[5] = bufferForDataFlash[1];
				data1024Arr[6] = bufferForDataFlash[2];
				data1024Arr[7] = bufferForDataFlash[3];
				data1024Arr[0] = 0; //0
				data1024Arr[1] = 8; //8
				chainInitAllData();
				dataFlashState = DF_WAITINGTOEND;
			} else if ((pos > 4092) || (pos <= 1020)) { //return to block 0
				if (pos > 4092)
					pos = 4;
				data1024Arr[pos] = bufferForDataFlash[0];
				data1024Arr[pos + 1] = bufferForDataFlash[1];
				data1024Arr[pos + 2] = bufferForDataFlash[2];
				data1024Arr[pos + 3] = bufferForDataFlash[3];
				pos += 4;
				data1024Arr[0] = pos >> 8;
				data1024Arr[1] = (uint8_t) pos;
				chainWriteBlockX(0);
				dataFlashState = DF_WAITINGTOEND;
			} else {	//to blockX (X>0)
				pos += 4;
				if (pos == 4096)
					pos = 4;
				data1024Arr[0] = pos >> 8;
				data1024Arr[1] = (uint8_t) pos;
				chainWriteBlockX(0);
				dataFlashState = DF_WAITTOREADBLOCKX;
			}
		}
		break;
	case DF_WAITTOREADBLOCKX:
		//writing block 0
		if (gFdlStatus == 0) {
			pos = (data1024Arr[0] << 8) + data1024Arr[1];
			if (pos == 4)
				chainReadBlockX(3);
			else
				chainReadBlockX((pos - 4) >> 10);
			dataFlashState = DF_READINGBLOCKX;
		}
		break;
	case DF_READINGBLOCKX:
		//reading blockX
		if (gFdlStatus == 0) {
			if (pos == 4) {
				data1024Arr[1020] = bufferForDataFlash[0];
				data1024Arr[1021] = bufferForDataFlash[1];
				data1024Arr[1022] = bufferForDataFlash[2];
				data1024Arr[1023] = bufferForDataFlash[3];
				chainWriteBlockX(3);
			} else {
				data1024Arr[(pos - 4) & 0x03FF] = bufferForDataFlash[0];
				data1024Arr[(pos - 3) & 0x03FF] = bufferForDataFlash[1];
				data1024Arr[(pos - 2) & 0x03FF] = bufferForDataFlash[2];
				data1024Arr[(pos - 1) & 0x03FF] = bufferForDataFlash[3];
				chainWriteBlockX((pos - 4) >> 10);
			}
			dataFlashState = DF_WAITINGTOEND;
		}
		break;
	case DF_WAITINGTOEND:
		//WAITING TO END
		if (gFdlStatus == 0) {
			dataFlashState = DF_IDLE;
		}
		break;
	case DF_ONLYREADBLOCK0:
	case DF_ONLYREADBLOCK1:
	case DF_ONLYREADBLOCK2:
	case DF_ONLYREADBLOCK3:
		if (gFdlStatus == 0) {
			chainReadBlockX(dataFlashState - DF_ONLYREADBLOCK0);
			dataFlashState = DF_WAITINGTOEND;
		}
		break;
	case DF_IDLE:
		//idle State
		break;
	}
	commandDataFlashEXE();
	return (dataFlashState);
}
void commandDataFlashEXE(void) {
	if (gFdlStatus == 0) {
		if (requestPtr != NULL) {
			R_FDL_Open();
			gFdlResult = PFDL_IDLE;
		}
	} else {
		if (requestPtr == NULL)
			R_FDL_Close();
		else {
			if ((requestPtr->bytecount_u16 == 0)
					&& (requestPtr->command_enu != PFDL_CMD_ERASE_BLOCK))
				requestPtr = NULL;
			else {
				switch (gFdlResult) {
				case PFDL_IDLE:
					gFdlResult = PFDL_Execute(requestPtr);
					break;
				case PFDL_BUSY:
					gFdlResult = PFDL_Handler();
					break;
				case PFDL_OK:
					requestPtr++;
					gFdlResult = PFDL_IDLE;
					break;
				}
			}
		}
	}
}
//setup chain of command
void chainInitAllData(void) {
	uint8_t i = 0;
	dataFlashStep[i].index_u16 = 0;
	dataFlashStep[i].bytecount_u16 = 1;
	dataFlashStep[i].command_enu = PFDL_CMD_ERASE_BLOCK;
	i++;
	dataFlashStep[i].index_u16 = 1;
	dataFlashStep[i].bytecount_u16 = 1;
	dataFlashStep[i].command_enu = PFDL_CMD_ERASE_BLOCK;
	i++;
	dataFlashStep[i].index_u16 = 2;
	dataFlashStep[i].bytecount_u16 = 1;
	dataFlashStep[i].command_enu = PFDL_CMD_ERASE_BLOCK;
	i++;
	dataFlashStep[i].index_u16 = 3;
	dataFlashStep[i].bytecount_u16 = 1;
	dataFlashStep[i].command_enu = PFDL_CMD_ERASE_BLOCK;
	i++;
	dataFlashStep[i].index_u16 = 0;
	dataFlashStep[i].bytecount_u16 = 1024;
	dataFlashStep[i].data_pu08 = data1024Arr;
	dataFlashStep[i].command_enu = PFDL_CMD_WRITE_BYTES;
	i++;
	dataFlashStep[i].index_u16 = 0;
	dataFlashStep[i].bytecount_u16 = 1024;
	dataFlashStep[i].command_enu = PFDL_CMD_IVERIFY_BYTES;
	i++;
//vitual command to end.
	dataFlashStep[i].index_u16 = 0;
	dataFlashStep[i].bytecount_u16 = 0;
	dataFlashStep[i].command_enu = PFDL_CMD_WRITE_BYTES;
	requestPtr = dataFlashStep;
}
void chainWriteBlockX(uint8_t blockno) {
	uint8_t i = 0;
	dataFlashStep[i].index_u16 = blockno;
	dataFlashStep[i].bytecount_u16 = 1;
	dataFlashStep[i].command_enu = PFDL_CMD_ERASE_BLOCK;
	i++;
	dataFlashStep[i].index_u16 = (blockno << 10);
	dataFlashStep[i].bytecount_u16 = 1024;
	dataFlashStep[i].data_pu08 = data1024Arr;
	dataFlashStep[i].command_enu = PFDL_CMD_WRITE_BYTES;
	i++;
	dataFlashStep[i].index_u16 = (blockno << 10);
	dataFlashStep[i].bytecount_u16 = 1024;
	dataFlashStep[i].command_enu = PFDL_CMD_IVERIFY_BYTES;
	i++;
//vitual command to end.
	dataFlashStep[i].index_u16 = 0;
	dataFlashStep[i].bytecount_u16 = 0;
	dataFlashStep[i].command_enu = PFDL_CMD_WRITE_BYTES;
	requestPtr = dataFlashStep;
}
void chainReadBlockX(uint8_t blockno) {
	dataFlashStep[0].index_u16 = (blockno << 10);
	dataFlashStep[0].bytecount_u16 = 1024;
	dataFlashStep[0].data_pu08 = data1024Arr;
	dataFlashStep[0].command_enu = PFDL_CMD_READ_BYTES;
//vitual command to end.
	dataFlashStep[1].index_u16 = 0;
	dataFlashStep[1].bytecount_u16 = 0;
	dataFlashStep[1].command_enu = PFDL_CMD_WRITE_BYTES;
	requestPtr = dataFlashStep;
}
//send data to buffer
void sendToDataFlashBuffer(uint8_t *data) {
	memcpy(bufferForDataFlash,data,4);
	memset(data,0,4);
	dataFlashState = DF_READINGBLOCK0;
}
uint8_t checkDataFlashState(void) {
	return dataFlashState;
}
void readDataFlashBlock(uint8_t blockno) {
	dataFlashState = DF_ONLYREADBLOCK0 + blockno;
}
/* End user code. Do not edit comment generated here */
