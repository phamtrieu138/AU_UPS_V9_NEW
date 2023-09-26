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
* File Name    : r_cg_adc_user.c
* Version      : CodeGenerator for RL78/G14 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for ADC module.
* Creation Date: 26/09/2023
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
uint16_t currentMAX[12] = { 205, 169, 139, 126, 120, 118, 116, 114, 112, 110, 108, 106 };
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
	uint32_t guess = 0;
	static uint8_t countShortCurr = 0;
	static uint16_t threeTimeCurrCnt = 0, curSum = 0;
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
			Vbat_data.RMS = (uint16_t) Vbat_data.sum;
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
		if (HV_data.counter < 255)
			HV_data.counter++;
		else {
			HV_data.counter = 0;
			HV_data.sum = HV_data.sum >> 8;
			HV_data.sum = HV_data.sum * 528 / 1000;
			HV_data.RMS = (uint16_t) HV_data.sum;
			HV_data.sum = 0;
		}
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
		// calculate ave value (3 times)
		curSum += out_curr_data.ADC;
		if (threeTimeCurrCnt < 2) {
			threeTimeCurrCnt++;
		} else {
			threeTimeCurrCnt = 0;
			currentAve = curSum / 3;
			curSum = 0;
			//save ave value to array
			if (sysDis.errCode == 0) {
				if (currentAve < 508)
					currentAVEsave[currAVEsaveCnt] = currentAve >> 1;
				else
					currentAVEsave[currAVEsaveCnt] = 0xFE;
				if (currAVEsaveCnt < 44)
					currAVEsaveCnt++;
				else
					currAVEsaveCnt = 0;
			}
			// kiem tra ngan mach
			if (currentAve < 60)
				countShortCurr = 0;
			else {
				if (currentAve > currentMAX[countShortCurr]) {
					outputShortFlag = 1;
					sysDis.errCode = 0x04;
				}
				if (countShortCurr < 11)
					countShortCurr++; //10ms
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
uint8_t checkShortCircuit(uint16_t dataADCave) {
	static uint8_t count;
	if (dataADCave < 60)
		count = 0;
	else {
		if (dataADCave > currentMAX[count]) {
			return 1;
		}
		if (count < 11)
			count++; //10ms
	}
	return 0;
}
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
/* End user code. Do not edit comment generated here */
