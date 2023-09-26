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
* File Name    : r_cg_timer_user.c
* Version      : CodeGenerator for RL78/G14 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for TAU module.
* Creation Date: 26/09/2023
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_timer.h"
/* Start user code for include. Do not edit comment generated here */
#define NUM_PREAMBLE	20
#define NUM_DATA		60
#define TE_MIN			140
#define TE_MAX			320
#include "r_cg_adc.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt r_tau0_channel0_interrupt(vect=INTTM00)
#pragma interrupt r_tau0_channel2_interrupt(vect=INTTM02)
#pragma interrupt r_tmr_rd0_interrupt(vect=INTTRD0)
#pragma interrupt r_tmr_rd1_interrupt(vect=INTTRD1)
/* Start user code for pragma. Do not edit comment generated here */
uint16_t phase = 3300; //5000 tg duong 50%
uint16_t outVoltage = 230;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
uint8_t TAU0_2flag = 0;
extern sysCommon_t sysCom;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_tau0_channel0_interrupt
* Description  : This function is INTTM00 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_tau0_channel0_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	/* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tau0_channel2_interrupt
* Description  : This function is INTTM02 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_tau0_channel2_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	R_ADC_Start();
	sysClockRun();
	if (TAU0_2flag)
		TAU0_2flag--;
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tmr_rd0_interrupt
* Description  : This function is INTTRD0 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_tmr_rd0_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	if (TRDSR0 & 0x08) {
		if (TRDGRA0 == _0200_TMRD_TRDGRA0_VALUE) {
			TRDGRA0 = _0200_TMRD_TRDGRA0_VALUE - 26;
		} else {
			TRDGRA0 = _0200_TMRD_TRDGRA0_VALUE;
		}
		TRDGRB0 = _01F3_TMRD_TRDGRB0_VALUE;
		TRDSR0 = 0;
	}
	/* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: r_tmr_rd1_interrupt
* Description  : This function is INTTRD1 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_tmr_rd1_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	if (TRDSR1 & 0x08) {
		if (TRDGRA1 > TRDGRB1) {
			TRDGRA1 = TRDGRB1 - 13;
		} else {
			TRDGRB1 = _01F3_TMRD_TRDGRB0_VALUE + phase;
			TRDGRA1 = TRDGRB1 + 13;
		}
		TRDSR1 = 0;
	}
	/* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
uint16_t readOutVoltage(void) {
	return outVoltage;
}
void outVoltageSet(uint16_t RMS) {
	if (RMS > 240)
		outVoltage = 240;
	else if (RMS < 200)
		outVoltage = 200;
	else
		outVoltage = RMS;
}
void phaseCalcu(uint16_t VPP) {
	phase = ((uint32_t) outVoltage * (uint32_t) outVoltage * 5000 / VPP / VPP);
	if (phase < 1000)
		phase = 1000;
	if (phase > 4200)
		phase = 4200;
}
uint8_t PWMcontrol(uint8_t PWMflag) {
	static uint8_t PWMstate = 1;
	if (PWMflag == 0) {
		if (PWMstate >= 1) {
			R_TMR_RD0_Stop();
			R_TMR_RD1_Stop();
			TRDGRA0 = _0200_TMRD_TRDGRA0_VALUE;
			TRDGRB0 = _01F3_TMRD_TRDGRB0_VALUE;
			TRDGRB1 = _0BB7_TMRD_TRDGRB1_VALUE;
			TRDGRA1 = _0BC4_TMRD_TRDGRA1_VALUE;
			TRD0 = 0;
			TRD1 = 0;
			TRDOER1 = 0xFF;
			P1_bit.no7 = 0;
			P1_bit.no5 = 1;
			P1_bit.no2 = 1;
			P1_bit.no3 = 0;
			PWMstate = 0;
		}
	} else {
		if (PWMstate == 0) {
			P1_bit.no7 = 0;
			P1_bit.no5 = 0;
			P1_bit.no2 = 0;
			P1_bit.no3 = 0;
			R_TMR_RD0_Create1();
			R_TMR_RD1_Create1();
			PWMstate = 1;
		} else if (PWMstate < 10) {
			PWMstate++;
		} else if (PWMstate == 10) {
			R_TMR_RD0_Start();
			R_TMR_RD1_Start();
			PWMstate = 11;
		}
	}
	return 1;
}
void setDutyDEC(uint16_t duty) {
	if (duty <= 0x320)
		TDR03 = duty;
	else
		TDR03 = 0x320;
}
uint8_t checkTAU0_2flag(void) {
	return TAU0_2flag;
}
void setTAU0_2flag(void) {
	TAU0_2flag = 20;
}
void sysClockRun(void) {
	if (sysCom.clock.tic < 20000)
		++sysCom.clock.tic;
	else {
		sysCom.clock.tic = 0;
		sysCom.clock.sec++;
		if (sysCom.clock.sec >= 1200) {		//1200
			if (sysCom.clock.per20minutes < 65535)
				sysCom.clock.per20minutes++;
			else
				sysCom.clock.per20minutes = 0;
			sysCom.clock.sec = 0;
		}
	}
}
/* End user code. Do not edit comment generated here */
