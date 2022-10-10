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
* Creation Date: 10/7/2022
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_adc.h"
/* Start user code for include. Do not edit comment generated here */
#include "stdio.h"
#include "stdlib.h"
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
analogData_t HV_data, Vbat_data, line_vol_data, out_curr_data;
uint16_t outCurRMS[60], outCurrADC[16];
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
	uint8_t i = 0, j = 0;
	static uint8_t countCurrADC = 0;
	switch (ADS) {
	case 3:
		HV_data.ADC = (ADCR >> 6);
		HV_data.sum += HV_data.ADC;
		if (HV_data.counter < 255)
			HV_data.counter++;
		else {
			HV_data.counter = 0;
			HV_data.RMS = (uint16_t) (HV_data.sum >> 8);
			HV_data.RMS = (uint16_t) (HV_data.RMS * 0.5279);
			HV_data.sum = 0;
		}
		ADS = Vbat_data.channel;
		break;
	case 1:
		Vbat_data.ADC = (ADCR >> 6);
		Vbat_data.sum += Vbat_data.ADC;
		if (Vbat_data.counter < 15)
			Vbat_data.counter++;
		else {
			Vbat_data.counter = 0;
			Vbat_data.RMS = (uint16_t) (Vbat_data.sum >> 4);
			Vbat_data.RMS = (uint16_t) (Vbat_data.RMS * 4.97103);
			Vbat_data.sum = 0;
		}
		ADS = line_vol_data.channel;
		break;
	case 2:
		line_vol_data.ADC = (ADCR >> 6);
		line_vol_data.sum += line_vol_data.ADC;
		if (line_vol_data.counter < 399)
			line_vol_data.counter++;
		else {
			line_vol_data.counter = 0;
			line_vol_data.RMS = line_vol_data.sum / 400;
			line_vol_data.sum = 0;
		}
		ADS = out_curr_data.channel;
		break;
	case 18: //80ms 1 lần lấy trung bình
		out_curr_data.ADC = (ADCR >> 6);
		if (out_curr_data.ADC > 512) {
			outCurrADC[countCurrADC] = out_curr_data.ADC - 512;
			out_curr_data.sum += (out_curr_data.ADC - 512);
		} else {
			out_curr_data.sum += (512 - out_curr_data.ADC);
			outCurrADC[countCurrADC] = 0;
		}
		//check output shortcir
		for (i = 0; i <= 6; i++) {
			if (countCurrADC >= i) {
				if (outCurrADC[countCurrADC - i] >= CURR_SHORT_ADC)
					j++;
			} else {
				if (outCurrADC[16 + countCurrADC - i] >= CURR_SHORT_ADC)
					j++;
			}
		}
		if (j >= 5) {
			//output short circuit
			outputShortFlag = 1;
		}
		if (countCurrADC < 15)
			countCurrADC++;
		else
			countCurrADC = 0;

		if (out_curr_data.counter < 319)
			out_curr_data.counter++;
		else {
			out_curr_data.counter = 0;
			out_curr_data.RMS = (uint16_t) (out_curr_data.sum * 0.0391); //gia tri rms = thuc te *100;
			out_curr_data.sum = 0;
			for (i = 0; i < 59; i++) {
				outCurRMS[i] = outCurRMS[i + 1];
			}
			outCurRMS[59] = out_curr_data.RMS;
		}
		ADS = HV_data.channel;
		break;
	}
	/* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
uint8_t outCurrCheck(uint16_t valueToCompare, uint8_t compareType,
		uint8_t dataCnt) {
	uint8_t i = 0;
	if (dataCnt <= 60) {
		if (compareType == 1)    //compare >=
				{
			for (i = 60 - dataCnt; i <= 59; i++) {
				if (outCurRMS[i] <= valueToCompare)
					return 0;
			}
		} else              //compare <=
		{
			for (i = 60 - dataCnt; i <= 59; i++) {
				if (outCurRMS[i] >= valueToCompare)
					return 0;
			}
		}
	} else
		return 0;
	return 1;
}

uint8_t lineVolCheck(uint16_t data, uint8_t *lineFlag) {
	static uint16_t cnt = 0;
	if ((*lineFlag) == 0) {
		if (cnt < 400) {
			if (data > 45)
				cnt++;
			else
				cnt = 0;
		} else {
			cnt = 0;
			(*lineFlag) = 1;
		}
	} else {
		if (cnt < 400) {
			if (data < 35)
				cnt++;
			else
				cnt = 0;
		} else {
			cnt = 0;
			(*lineFlag) = 0;
		}
	}
	return 1;
}
uint8_t mySqrt(uint16_t dataIn) {
	uint16_t temp = dataIn;
	uint16_t result = 1, newResult = 0;
	if (dataIn < 16)
		result = 4;
	else {
		while (temp >= 16) {
			temp = temp >> 4;
			result = (result << 2);
		}
		if (temp <= 4)
			result = result << 1;
		else
			result = result * 3;
	}
	while (1) {
		newResult = (result + (dataIn / result)) >> 1;
		if (abs(result - newResult) <= 1)
			break;
		result = newResult;
	}
	return result;
}
void ADCdataInit(void) {
	HV_data.channel = 3;
	HV_data.counter = 0;
	HV_data.sum = 0;
	HV_data.RMS = 0;
	HV_data.ADC = 0;
	Vbat_data.channel = 1;
	Vbat_data.counter = 0;
	Vbat_data.sum = 0;
	Vbat_data.RMS = 0;
	Vbat_data.ADC = 0;
	line_vol_data.channel = 2;
	line_vol_data.counter = 0;
	line_vol_data.sum = 0;
	line_vol_data.RMS = 0;
	line_vol_data.ADC = 0;
	out_curr_data.channel = 18;
	out_curr_data.counter = 0;
	out_curr_data.sum = 0;
	out_curr_data.RMS = 0;
	out_curr_data.ADC = 0;
}
/* End user code. Do not edit comment generated here */
