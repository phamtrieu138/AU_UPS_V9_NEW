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
* File Name    : r_cg_adc_user.c
* Version      : CodeGenerator for RL78/G14 V2.05.08.02 [03 Jun 2024]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for ADC module.
* Creation Date: 10/9/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_adc.h"
/* Start user code for include. Do not edit comment generated here */
#include "stdio.h"
#include "stdlib.h"
#include "r_cg_port.h"
#include <string.h>
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt r_adc_interrupt(vect=INTAD)
/* Start user code for pragma. Do not edit comment generated here */
#define CURR_SHORT_ADC	300
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
extern uint8_t outputShortFlag;
extern sysDis_t sysDis;
analogData_t HV_data, Vbat_data, out_curr_data;
uint16_t currentAve;
uint8_t currentRMSsave[100], currRMSsaveCnt = 0, doneCalcuRMS = 0;
uint8_t currentAVEsave[45], currAVEsaveCnt = 0;
uint8_t currentADCsave[5], currADCsaveCnt = 0;
uint8_t vbattRMSsave[100], vbattRMSsaveCnt = 0;
uint8_t vbattAVEsave[45], vbattAVEsaveCnt = 0;
uint8_t vppAVEsave[45], vppAVEsaveCnt = 0;
//bao ve qua tai
const uint16_t overRMSratio[8] = { 320, 210, 155, 140, 130, 120, 110, 105 };
const uint8_t overRMScntMAX[8] = { 2, 4, 6, 7, 8, 9, 11, 12 };
//bao ve ngăn mach
const uint16_t currentADCMAXDIS[4] = { 290, 275, 250, 220 };
uint16_t currentADCMAX[4] = { 0 };
uint8_t shortCntMAX[4] = { 1, 2, 3, 5 };
const uint16_t currentADCMAXCHARGE[4] = { 330, 290, 280, 260 };
const uint8_t shortCntMAXDIS[4] = { 1, 2, 3, 5 };
const uint8_t shortCntMAXCHARGE[4] = { 3, 6, 9, 15 };
uint8_t overSHORTcnt[4];
//bve AQ yếu
uint16_t vbatMIN = 1920;
uint16_t HVflt;
uint16_t battflt;
void LPF(uint16_t input, uint16_t *output, uint8_t numerator,
		uint8_t denominator);
uint16_t trieuHV;
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_adc_interrupt
* Description  : This function is INTAD interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
static void __near r_adc_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	uint8_t shortCnt;
	uint32_t guess = 0;
	static uint16_t threeTimeVppCnt = 0, vppSum = 0;
	static uint16_t threeTimeVbattCnt = 0, vbattSum = 0, saveVbattRMScnt = 0;
	switch (ADS) {
	case 1:
		Vbat_data.ADC = (ADCR >> 6);
		Vbat_data.sum += Vbat_data.ADC;
		vbattSum += Vbat_data.ADC;
		if (threeTimeVbattCnt < 2) {
			threeTimeVbattCnt++;
		} else {
			threeTimeVbattCnt = 0;
			//save ave value to array = vbatADC * 4
			if (sysDis.errCode == 0) {
				if (vbattSum < 3048)
					vbattAVEsave[vbattAVEsaveCnt] = vbattSum / 12;
				else
					vbattAVEsave[vbattAVEsaveCnt] = 0xFE;
				if (vbattAVEsaveCnt < 44)
					vbattAVEsaveCnt++;
				else
					vbattAVEsaveCnt = 0;
			}
			vbattSum = 0;
		}
		if (Vbat_data.counter < 15)
			Vbat_data.counter++;
		else {
			Vbat_data.counter = 0;
			Vbat_data.sum = Vbat_data.sum >> 4;
			Vbat_data.sum = Vbat_data.sum * 4971 / 1000;
			//Vbat_data.RMS = (uint16_t) Vbat_data.sum;
			LPF((uint16_t) Vbat_data.sum, &battflt, 1, 4);
			Vbat_data.RMS = battflt;
			Vbat_data.longSum += Vbat_data.RMS;
			Vbat_data.sum = 0;
			//save vbatt rms per 1 second = vbat * 5
			if (saveVbattRMScnt < 444) {
				saveVbattRMScnt++;
			} else {
				saveVbattRMScnt = 0;
				Vbat_data.longSum = Vbat_data.longSum / 445;
				Vbat_data.longRMS = Vbat_data.longSum;
				Vbat_data.longSum = 0;
				if (sysDis.errCode == 0) {
					if (Vbat_data.longSum < 5080) {
						vbattRMSsave[vbattRMSsaveCnt] = Vbat_data.longSum / 20;
					} else
						vbattRMSsave[vbattRMSsaveCnt] = 0xFE;
					if (vbattRMSsaveCnt < 99)
						vbattRMSsaveCnt++;
					else
						vbattRMSsaveCnt = 0;
				}
			}
		}
		ADS = out_curr_data.channel;
		break;
	case 2:
		HV_data.ADC = (ADCR >> 6);
		HV_data.sum += HV_data.ADC;
		if (threeTimeVppCnt < 2) {
			threeTimeVppCnt++;
			vppSum += HV_data.ADC;
		} else {
			threeTimeVppCnt = 0;
			vppSum += HV_data.ADC;
			//save ave value to array = 1/4 vppADC
			if (sysDis.errCode == 0) {
				if (vppSum < 3048)
					vppAVEsave[vppAVEsaveCnt] = vppSum / 12;
				else
					vppAVEsave[vppAVEsaveCnt] = 0xFE;
				if (vppAVEsaveCnt < 44)
					vppAVEsaveCnt++;
				else
					vppAVEsaveCnt = 0;
			}
			vppSum = 0;
		}
		LPF(HV_data.ADC, &HVflt, 1, 8);
		HV_data.RMS = (uint16_t) ((uint32_t) HVflt * 573 / 1000);
		//HV_data.RMS = (uint16_t)((uint32_t)HVflt* 528 / 1000);
//		if (HV_data.counter < 255)
//			HV_data.counter++;
//		else {
//			HV_data.counter = 0;
//			HV_data.sum = HV_data.sum >> 8;
//			HV_data.sum = HV_data.sum * 528 / 1000;//528
//			HV_data.RMS = (uint16_t) HV_data.sum;
//			HV_data.sum = 0;
//		}
		ADS = Vbat_data.channel;
		break;
	case 18: //150ms 1 lần lấy mẫu
		out_curr_data.ADC = (ADCR >> 6);
		if (out_curr_data.ADC > 512)
			out_curr_data.ADC = (out_curr_data.ADC - 512);
		else
			out_curr_data.ADC = (512 - out_curr_data.ADC);
		//save ADC value to array
		if (sysDis.errCode == 0) {
			if (out_curr_data.ADC < 508)
				currentADCsave[currADCsaveCnt] = out_curr_data.ADC >> 1;
			else
				currentADCsave[currADCsaveCnt] = 0xFE;
			if (currADCsaveCnt < 4)
				currADCsaveCnt++;
			else
				currADCsaveCnt = 0;
		}
		//LPF(out_curr_data.ADC, &currentAve, 1, 4);
		//diff = (int32_t)out_curr_data.ADC - (int32_t)(currentAve);            // Tinh hieu
		currentAve = out_curr_data.ADC;
		for (shortCnt = 0; shortCnt <= 3; shortCnt++) {
			if (out_curr_data.ADC >= currentADCMAX[shortCnt])
				overSHORTcnt[shortCnt]++;
			else {
				if (overSHORTcnt[shortCnt] > 0)
					overSHORTcnt[shortCnt]--;
			}
			if (overSHORTcnt[shortCnt] >= shortCntMAX[shortCnt]) {
				outputShortFlag = 1;
				memset(overSHORTcnt, 0, sizeof(overSHORTcnt));
				break;
			}
		}
		//calcu RMS current output
		out_curr_data.sum += out_curr_data.ADC * out_curr_data.ADC;
		if (out_curr_data.counter < 132) {
			out_curr_data.counter++;
		} else {
			out_curr_data.counter = 0;
			out_curr_data.sum = out_curr_data.sum / 133;
			//calcu sqrt of sum
			guess = out_curr_data.sum >> 1; // Initial guess
			if (out_curr_data.sum <= 2) {
				out_curr_data.RMS = 1;
			} else {
				while ((guess * guess) > out_curr_data.sum) {
					guess = (guess + out_curr_data.sum / guess) >> 1;
				}
				guess = guess * 10;
				out_curr_data.RMS = guess; //gia tri rms = thuc te *100;
			}
			doneCalcuRMS = 1;
			out_curr_data.sum = 0;
			//save rms value to array
			if (sysDis.errCode == 0) {
				if (out_curr_data.RMS < 1016)
					currentRMSsave[currRMSsaveCnt] = out_curr_data.RMS >> 2;
				else
					currentRMSsave[currRMSsaveCnt] = 0xFE;
				if (currRMSsaveCnt < 99)
					currRMSsaveCnt++;
				else
					currRMSsaveCnt = 0;
			}
		}
		//check output short circuit
		ADS = HV_data.channel;
		break;
	default:
		ADS = Vbat_data.channel;
		break;
	}
	/* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
uint16_t mySqrt(uint32_t x) {
	uint32_t guess = x >> 1; // Initial guess
	if (x <= 100) {
		return 10;
	}
	while ((guess * guess) > x) {
		guess = (guess + x / guess) >> 1;
	}
	return (uint16_t) guess;
}
void ADCdataInit(void) {
	HV_data.channel = 2;
	HV_data.counter = 0;
	HV_data.sum = 0;
	HV_data.RMS = 0;
	HV_data.ADC = 0;
	Vbat_data.channel = 1;
	Vbat_data.counter = 0;
	Vbat_data.sum = 0;
	Vbat_data.RMS = 0;
	Vbat_data.ADC = 0;
	out_curr_data.channel = 18;
	out_curr_data.counter = 0;
	out_curr_data.sum = 0;
	out_curr_data.RMS = 0;
	out_curr_data.ADC = 0;
}
void LPF(uint16_t input, uint16_t *output, uint8_t numerator,
		uint8_t denominator) {
	int32_t diff = (int32_t) input - (int32_t) (*output);           // Tinh hieu
	int32_t result = (*output) + (numerator * diff) / denominator; // Ap dung phep loc voi so nguyen
	// Dam bao ket qua nam trong pham vi uint16_t
	if (result < 0) {
		*output = 0;
	} else if (result > 65535) {
		*output = 65535;
	} else {
		*output = (uint16_t) result;
	}
}
/* End user code. Do not edit comment generated here */
