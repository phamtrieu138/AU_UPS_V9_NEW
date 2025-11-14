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
* Copyright (C) 2011, 2025 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_port_user.c
* Version      : CodeGenerator for RL78/G14 V2.05.09.01 [28 Apr 2025]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for PORT module.
* Creation Date: 11/5/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_port.h"
/* Start user code for include. Do not edit comment generated here */
#include "r_cg_timer.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
unsigned char led[6] = {0};
/* End user code. Do not edit comment generated here */

/* Start user code for adding. Do not edit comment generated here */
void setupTestCheck(void) {
	PM1_bit.no0 = 1;
	PU1_bit.no0 = 1;
	PIM1_bit.no0 = 1;
}
void clearTestCheck(void) {
	PM1_bit.no0 = 0;
	PU1_bit.no0 = 0;
	PIM1_bit.no0 = 0;
}
void buzzerUpdate(buzzer_t state,uint8_t errCode) {
	static buzzer_t stateOld;
	static uint8_t buzzerCnt = 0;
	uint8_t buzzCntMax=0;
	switch (state) {
	case BUZZ_DIS:
		if (buzzerCnt < 60) {
			if (buzzerCnt == 0)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 20)
				R_TAU0_Channel1_Stop();
			if (buzzerCnt == 40)
				R_TAU0_Channel1_Start();
			++buzzerCnt;
		} else {
			R_TAU0_Channel1_Stop();
			BUZZER = 0;
		}
		break;
	case BUZZ_CHARGE:
		if (buzzerCnt < 90) {
			if (buzzerCnt == 0)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 30)
				R_TAU0_Channel1_Stop();
			if (buzzerCnt == 60)
				R_TAU0_Channel1_Start();
			++buzzerCnt;
		} else {
			R_TAU0_Channel1_Stop();
			BUZZER = 0;
		}
		break;
	case BUZZ_ERR:
		switch (errCode)
		{
		case 1://over RMS current - 3 bip
			buzzCntMax = 50;
			break;
		case 3://low batt - 4 bip
			buzzCntMax = 70;
			break;
		case 4: //short circuit - 5 bip
			buzzCntMax = 90;
			break;
		case 5: //low PP voltage - 6 bip
			buzzCntMax = 110;
			break;
		case 6: //Discharge over 55h - 7 bip
			buzzCntMax = 130;
			break;
		default:
			buzzCntMax = 90;
			break;
		}
		if (buzzerCnt < buzzCntMax) {
			if (buzzerCnt == 0)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 10)
				R_TAU0_Channel1_Stop();
			if (buzzerCnt == 20)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 30)
				R_TAU0_Channel1_Stop();
			if (buzzerCnt == 40)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 50)
				R_TAU0_Channel1_Stop();
			if (buzzerCnt == 60)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 70)
				R_TAU0_Channel1_Stop();
			else if (buzzerCnt == 80)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 90)
				R_TAU0_Channel1_Stop();
			else if (buzzerCnt == 100)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 110)
				R_TAU0_Channel1_Stop();
			else if (buzzerCnt == 120)
				R_TAU0_Channel1_Start();
			++buzzerCnt;
		} else {
			R_TAU0_Channel1_Stop();
			BUZZER = 0;
		}
		break;
	case BUZZ_STOP:
		if (buzzerCnt < 30) {
			if (buzzerCnt == 0)
				R_TAU0_Channel1_Start();
			else if (buzzerCnt == 10)
				R_TAU0_Channel1_Stop();
			if (buzzerCnt == 20)
				R_TAU0_Channel1_Start();
			++buzzerCnt;
		} else {
			R_TAU0_Channel1_Stop();
			BUZZER = 0;
		}
		break;
	case BUZZ_OFF:
		R_TAU0_Channel1_Stop();
		BUZZER = 0;
		break;
	}
	if (stateOld != state) {
		R_TAU0_Channel1_Stop();
		BUZZER = 0;
		buzzerCnt = 0;
		stateOld = state;
	}
}
void ledUpdate(led_t state, uint8_t errCode) {
	static uint16_t ledCnt;
	static led_t ledStateOld;
	if (state != ledStateOld) {
		ledCnt = 0;
		ledStateOld = state;
	}
	switch (state) {
	case LED_DIS:
		LED_GREEN = 1;
		LED_YELLOW = 0;
		LED_RED = 0;
		break;
	case LED_AUTO_DIS:
		LED_YELLOW = 0;
		LED_RED = 0;
		if (ledCnt == 0) {
			ledCnt = 500;
			LED_GREEN ^= 1;
		} else
			--ledCnt;
		break;
	case LED_ERRSTATE:
		if (ledCnt < 2000) {
			if (ledCnt < 400 * errCode) {
				if ((ledCnt % 200) == 0)
					LED_RED ^= 1;
			} else
				LED_RED = 0;
			ledCnt++;
		} else {
			ledCnt = 0;
		}
		LED_YELLOW = 0;
		LED_GREEN = 0;
		break;
	case LED_FULLCHARGE:
		LED_GREEN = 0;
		LED_YELLOW = 1;
		LED_RED = 0;
		break;
	case LED_CHARGE_ERR:
		LED_GREEN = 0;
		LED_YELLOW = 0;
		if (ledCnt == 0) {
			ledCnt = 200;
			LED_RED ^= 1;
		} else
			--ledCnt;
		break;
	case LED_CHARGE:
		LED_GREEN = 0;
		if (ledCnt == 0) {
			ledCnt = 200;
			LED_YELLOW ^= 1;
		} else
			--ledCnt;
		LED_RED = 0;
		break;
	case LED_OFF:
		LED_GREEN = 0;
		LED_YELLOW = 0;
		LED_RED = 0;
		break;
	case LED_IDLE:
		LED_YELLOW = 0;
		LED_RED = 0;
		if (ledCnt == 0) {
			ledCnt = 1000;
		} else {
			if (ledCnt > 900)
				LED_GREEN = 1;
			else
				LED_GREEN = 0;
			--ledCnt;
		}
		break;
	}
}
void LED_D_UPDATE(unsigned char stateOfCharge, led_t ledState1) {
    unsigned char i = 0;
    static unsigned char j = 0;
    static unsigned int ledCnt = 0;
    static led_t LED_OLD;
    switch (ledState1) {
        case LED_DIS:
            for (i = 0; i <= 3; i++) {
                if (i < stateOfCharge) led[i] = 1;
                else led[i] = 0;
            }
            led[4] = 1;
            led[5] = 1;
            break;
        case LED_AUTO_DIS:
            for (i = 0; i <= 3; i++) {
                if (i < stateOfCharge) led[i] = 1;
                else led[i] = 0;
            }
            led[5] = 0;
            if (ledCnt < 200) ledCnt++;
            else {
                ledCnt = 0;
                if (led[4] == 1) led[4] = 0;
                else led[4] = 1;
            }
            break;
        case LED_ERRSTATE:
            led[0] = 0;
            led[1] = 0;
            led[2] = 0;
            led[3] = 0;
            led[4] = 0;
            led[5] = 1;
            break;
        case LED_FULLCHARGE:
            led[0] = 1;
            led[1] = 1;
            led[2] = 1;
            led[3] = 1;
            led[4] = 1;
            led[5] = 0;
            break;
        case LED_CHARGE:
            if (ledCnt < 400) ledCnt++;
            else {
                ledCnt = 0;
                if (stateOfCharge <= 4) {
                    if (j < stateOfCharge) j++;
                    else j = 0;
                } else {
                    j = 4;
                }
                i = 0;
                while (i <= 3) {
                    if (i < j) led[i] = 1;
                    else led[i] = 0;
                    i++;
                }
            }
            led[4] = 1;
            led[5] = 0;
            break;
        case LED_CHARGE_ERR:
            led[0] = 0;
            led[1] = 0;
            led[2] = 0;
            led[3] = 0;
            led[4] = 0;
            if (ledCnt < 200) ledCnt++;
            else {
                ledCnt = 0;
                if (led[5] == 1) led[5] = 0;
                else led[5] = 1;
            }
        	break;
        case LED_OFF:
            led[0] = 0;
            led[1] = 0;
            led[2] = 0;
            led[3] = 0;
            led[4] = 0;
            led[5] = 0;
            break;
    }
    ledDmoi();
    if (ledState1 != LED_OLD) {
        ledCnt = 0;
        LED_OLD = ledState1;
    }
}void ledDmoi(void) {
    unsigned char data = 0;
    static unsigned char demled = 0;
    switch (demled) {
        case 0: //led4
            if (led[3] == 1) data = demled;
            else data = 6;
            demled++;
            break;
        case 1://led3
            if (led[2] == 1) data = demled;
            else data = 6;
            demled++;
            break;
        case 2://led2
            if (led[1] == 1) data = demled;
            else data = 6;
            demled++;
            break;
        case 3://led1
            if (led[0] == 1) data = demled;
            else data = 6;
            demled++;
            break;
        case 4://ledxanh
            if (led[demled] == 1) data = demled;
            else data = 6;
            demled++;
            break;
        case 5://leddo
            if (led[demled] == 1) data = demled;
            else data = 6;
            demled = 0;
            break;
    }
    if (data & 0x01) D0 = 1;
    else D0 = 0;
    if (data & 0x02) D1 = 1;
    else D1 = 0;
    if (data & 0x04) D2 = 1;
    else D2 = 0;
}
void PPcontrol(boolean flag) {
	if (flag)
		SD_PP = 1;
	else
		SD_PP = 0;
}
void RELAYcontrol(boolean flag) {
	if (flag)
		RELAY = 1;
	else
		RELAY = 0;
}
void setPowerMCU_ON(void) {
	ON_24V_MCU = 1;
}
void clearPowerMCU_ON(void) {
	ON_24V_MCU = 0;
}
uint8_t pushButtonCheck(uint8_t *data) {
	static uint16_t cnt = 0;
	if (cnt < 500) {
		if ((*data) != PB)
			cnt++;
		else
			cnt = 0;
	} else {
		cnt = 0;
		(*data) = PB;
	}
	return 1;
}
uint8_t lineInCheck(uint8_t *data) {
	static uint16_t cnt = 0;
	if (cnt < 500) {
		if ((*data) != LINE_IN)
			cnt++;
		else
			cnt = 0;
	} else {
		cnt = 0;
		(*data) = LINE_IN;
	}
	return 1;
}
/* End user code. Do not edit comment generated here */
