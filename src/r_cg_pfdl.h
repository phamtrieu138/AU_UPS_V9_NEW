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
* File Name    : r_cg_pfdl.h
* Version      : CodeGenerator for RL78/G14 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for PFDL module.
* Creation Date: 10/5/2022
***********************************************************************************************************************/

#ifndef PFDL_H
#define PFDL_H

/***********************************************************************************************************************
Macro definitions (Register bit)
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "pfdl.h"            /* library header file */
#include "pfdl_types.h"      /* library header file */

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define _16_HOCO_CLOCK_MHz             (16)  /* HOCO clock value in MHz */
#define _WIDE_VOLTAGE_MODE            (1)     /* Flash Programming Mode is Wide Voltage */

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Global functions
***********************************************************************************************************************/
void R_FDL_Create(void);
pfdl_status_t R_FDL_Write(pfdl_u16 index, __near pfdl_u08* buffer, pfdl_u16 bytecount);
pfdl_status_t R_FDL_Read(pfdl_u16 index, __near pfdl_u08* buffer, pfdl_u16 bytecount);
pfdl_status_t R_FDL_Erase(pfdl_u16 blockno);
void R_FDL_Open(void);
void R_FDL_Close(void);
pfdl_status_t R_FDL_BlankCheck(pfdl_u16 index, pfdl_u16 bytecount);
pfdl_status_t R_FDL_IVerify(pfdl_u16 index, pfdl_u16 bytecount);

#endif
