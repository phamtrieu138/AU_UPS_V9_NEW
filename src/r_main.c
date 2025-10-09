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
* File Name    : r_main.c
* Version      : CodeGenerator for RL78/G14 V2.05.08.02 [03 Jun 2024]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements main function.
* Creation Date: 8/11/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_pfdl.h"
#include "r_cg_port.h"
#include "r_cg_serial.h"
#include "r_cg_adc.h"
#include "r_cg_timer.h"
/* Start user code for include. Do not edit comment generated here */
#include <string.h>
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
//uint16_t uartCnt = 0;
//extern uint8_t dataOut[16];
//variants of ADC
extern uint8_t vbattRMSsave[100], vbattRMSsaveCnt;
extern uint8_t vbattAVEsave[45], vbattAVEsaveCnt;
extern uint8_t currentRMSsave[100], currRMSsaveCnt, doneCalcuRMS;
extern uint8_t currentAVEsave[45], currAVEsaveCnt;
extern uint8_t currentADCsave[5], currADCsaveCnt;
extern uint8_t vppAVEsave[45], vppAVEsaveCnt;
extern analogData_t HV_data, Vbat_data, out_curr_data;
extern uint16_t currentMAX[12];
//variants of data flash
extern uint8_t dataEEP[1024];
extern uint8_t gFdlStatus;
extern uint8_t uartBlockReadFlag;
//variants of uart
extern uint8_t RXdataReceiver;
sysCommon_t sysCom;
uint8_t timeOut10ms = 9;
uint16_t varCntTest = 0;
sysDis_t sysDis;
const uint16_t current_ADC_DIS_MAX[12] = { 205, 169, 139, 126, 120, 118, 116, 114, 112, 110, 108, 106 };
uint8_t outputShortFlag = 0;
const uint8_t overRMSratio[30] = { 100, 90, 80, 70, 63, 56, 49, 45, 41, 37, 34, 31, 28, 25, 23, 21, 19, 17, 15, 13, 11, 9, 7, 6, 5, 4, 3, 2, 1, 0 };
sysCharge_t sysCharg;
const uint16_t current_ADC_CHARGE_MAX[12] = { 307, 253, 208, 189, 180, 177, 174, 171, 168, 165, 162, 159 };
uint16_t Dpart = 0;
float Ipart = 0;
uint8_t timeOfChargeState = 15, SOC = 1;
void disSOCCalcu(unsigned char *state);
boolean checkPP(boolean checkEnable);
uint8_t saveDataToEEP(uint8_t errCode);
void saveDataTo_dataEEP(uint8_t errCode);
/* End user code. Do not edit comment generated here */
void R_MAIN_UserInit(void);

/***********************************************************************************************************************
* Function Name: main
* Description  : This function implements main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void main(void)
{
    R_MAIN_UserInit();
    /* Start user code. Do not edit comment generated here */
	BUZZER = 0;
	RELAY = 0;
	ON_24V_MCU = 0;
	SD_PP = 0;
	LED_GREEN = 1;
	LED_YELLOW = 1;
	LED_RED = 0;
	sysCom.state = SYS_STARTUP;
	sysCom.stateOld = SYS_STARTUP;
	while (1U) {
		if (checkTAU0_2flag() == 0) {
			switch (sysCom.state) {
			case SYS_STARTUP: //delay 400mS to wait stable VCC for MCU
				setupTestCheck();
				if (sysCom.cnt < 500) {
					sysCom.cnt++;
					if (sysCom.cnt > 100) {
						if (P1_bit.no0 == 0)
							varCntTest++;
					}
				} else {
					if (varCntTest > 350)
						timeOfChargeState = 0;
					else
						timeOfChargeState = 15; //15 v9.1.0
					clearTestCheck();
					sysCom.state = SYS_INIT;
				}
				break;
			case SYS_INIT:
				//answer read block command from uart
				if (uartBlockReadFlag < 4) {
					startReadFromEEP1(1024, uartBlockReadFlag * 1024);
					uartBlockReadFlag = 5;
				} else if (uartBlockReadFlag == 5) {
					R_UART0_Send(dataEEP, 1024);
					uartBlockReadFlag = 4;
				}
				//check lineFlag or buttonFlag
				if (sysDis.pushButton) {
					if (Vbat_data.RMS > 1800) {
						SOC = 5;
						setPowerMCU_ON();
						memcpy(currentMAX, current_ADC_DIS_MAX, 12);
						sysCom.state = SYS_DIS;
					}
				} else if (sysCharg.lineFlag == 1) {
					SOC = 1;
					setPowerMCU_ON();
					memcpy(currentMAX, current_ADC_CHARGE_MAX, 12);
					sysCom.state = SYS_CHARGE;
				}
				break;
			case SYS_DIS:
				setPowerMCU_ON();
				if (sysCom.cnt <= 60000) {
					if (sysCom.cnt == 0) {
						sysDis.state = DIS_WAIT;
						doneCalcuRMS = 0;
						sysDis.overCurrStep = 0;
						memcpy(currentMAX, current_ADC_DIS_MAX, 12);
					}
					sysCom.cnt++;
				}
				switch (sysDis.state) {
				case DIS_WAIT:
					sysDis.PWMflag = FALSE;
					sysDis.checkPPflag = FALSE;
					sysDis.PPflag = FALSE;
					sysDis.underBattVolCnt = 0;
					sysDis.noLoadCnt = 0;
					sysCom.buzzState = BUZZ_DIS;
					sysCom.ledState = LED_DIS;
					setDutyDEC(0);
					outVoltageSet(230);
					if (sysCom.cnt >= SYS_DELAY_RELAY_DIS)
						sysDis.state = DIS_RELAY_ON;
					break;
				case DIS_RELAY_ON:
					sysDis.relayFlag = FALSE;
					if (sysCom.cnt > SYS_DELAY_PP)
						sysDis.state = DIS_PP_ON;
					break;
				case DIS_PP_ON:
					sysDis.PPflag = TRUE;
					if (sysCom.cnt > SYS_DELAY_CHECK_PP)
						sysDis.state = DIS_CHECK_PP_ON;
					break;
				case DIS_CHECK_PP_ON:
					sysDis.checkPPflag = TRUE;
					if (sysCom.cnt > SYS_DELAY_PWM)
						sysDis.state = DIS_PWM_ON;
					break;
				case DIS_PWM_ON:
					sysDis.PWMflag = TRUE;
					if (sysCom.cnt > SYS_DELAY_CHECK_LINE) {
						sysDis.state = DIS_CHECK_LINE;
					}
					break;
				case DIS_CHECK_LINE:
					if (sysCharg.lineFlag == 1)
						sysCom.state = SYS_CHARGE;
					break;
				}
				//bao ve qua tai 1, check after 1 cycle of AC output (calculated rms current)
				if (doneCalcuRMS == 1) {
					/*switch (sysDis.overCurrStep) {
					case 0: //while do not have over RMS
						sysDis.checkOverCurCnt = 0;
						if (out_curr_data.RMS > OUTPUT_DIS_OVERLOAD2)
							sysDis.overCurrStep++;
						break;
					case 1: //from 1-29 cycles after Irms>OUTPUT_DIS_OVERLOAD2
						if (out_curr_data.RMS > ((uint16_t) overRMSratio[sysDis.checkOverCurCnt] * OUTPUT_DIS_OVERLOAD2 / 100 + OUTPUT_DIS_OVERLOAD2)) {
							sysCom.state = SYS_DIS_ERR;
							sysDis.errCode = 0x01;
							saveDataTo_dataEEP(sysDis.errCode);
							sysDis.errCnt++;
						}
						if (sysDis.checkOverCurCnt < 29)
							sysDis.checkOverCurCnt++;
						else
							sysDis.overCurrStep++;
						break;
					case 2: //from 30-39 cycles after Irms> OUTPUT_DIS_OVERLOAD2
						if (out_curr_data.RMS > OUTPUT_DIS_OVERLOAD2) {
							sysCom.state = SYS_DIS_ERR;
							sysDis.errCode = 0x01;
							saveDataTo_dataEEP(sysDis.errCode);
							sysDis.errCnt++;
						}
						if (sysDis.checkOverCurCnt < 39)
							sysDis.checkOverCurCnt++;
						else
							sysDis.overCurrStep = 0;
						break;
					}*/
					doneCalcuRMS = 0;
				}
				//bao ve AQ yeu
				if (Vbat_data.RMS < 1900) {
					if (sysDis.underBattVolCnt < 2000) {
						sysDis.underBattVolCnt++;
					} else {
						sysCom.state = SYS_DIS_ERR;
						sysDis.errCode = 0x03;
						saveDataTo_dataEEP(sysDis.errCode);
						sysDis.errCnt += 2;
					}
				} else
					sysDis.underBattVolCnt = 0;
				// output short circuit protect
				if (outputShortFlag) {
					sysCom.state = SYS_DIS_ERR;
					sysDis.errCode = 0x04;
					saveDataTo_dataEEP(sysDis.errCode);
					sysDis.errCnt++;
				}
				//low PP voltage
				if (checkPP(sysDis.checkPPflag) == TRUE) {
					sysCom.state = SYS_DIS_ERR;
					sysDis.errCode = 0x05;
					saveDataTo_dataEEP(sysDis.errCode);
					sysDis.errCnt++;
				}
				//protect over time discharge
				if (timeOut10ms == 0) {
					if (sysDis.disCnt < MAX_TIME_DIS)
						sysDis.disCnt++;
					else {
						sysCom.state = SYS_DIS_ERR;
						sysDis.errCode = 0x06;
						saveDataTo_dataEEP(sysDis.errCode);
						sysDis.errCnt = 3;
					}
				}
				// calculate SOC for D ups display
				disSOCCalcu(&SOC);
				//reset bien chargeState
				if (out_curr_data.RMS > 100) {
					sysDis.noLoadCnt = 0;
					if (Vbat_data.longRMS < 2400)
						sysCharg.chargeState = 0;
				} else {
					if (sysDis.noLoadCnt < 300000)
						++sysDis.noLoadCnt;
					else {
						if (Vbat_data.longRMS < 2550)
							sysCharg.chargeState = 0;
					}
				}
				//reset time counter online if battery not full
				if (Vbat_data.longRMS < 2550) {
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
				}
				break;
			case SYS_DIS_ERR:
				setPowerMCU_ON();
				outputShortFlag = 0;
				if (sysCom.cnt < 200) {
					if (saveDataToEEP(sysDis.errCode) == 1)
						sysCom.cnt = 200;
				}
				sysDis.PPflag = FALSE;
				sysDis.PWMflag = FALSE;
				sysCom.ledState = LED_ERRSTATE;
				sysCom.buzzState = BUZZ_ERR;
				if (sysCom.cnt < 7002) {
					++sysCom.cnt;
				} else {
					if (sysDis.errCnt >= 3)
						sysCom.state = SYS_STOP;
					else {
						sysCom.state = SYS_DIS;
						sysDis.errCode = 0;
					}
				}
				if ((sysCharg.lineFlag == 1) && (sysCom.cnt >= 1000))
					sysCom.state = SYS_CHARGE;
				break;
			case SYS_STOP:
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysDis.relayFlag = FALSE;
				sysCom.ledState = LED_OFF;
				setPowerMCU_ON();
				if (sysCom.cnt < 301) {
					sysCom.cnt++;
				} else
					sysCom.state = SYS_OFF;
				sysCom.buzzState = BUZZ_STOP;
				break;
			case SYS_OFF:
				clearPowerMCU_ON();
				if (sysCharg.lineFlag == 1) {
					sysCom.state = SYS_CHARGE;
				}
				sysDis.disCnt = 0;
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysCom.buzzState = BUZZ_OFF;
				sysCom.ledState = LED_OFF;
				break;
			case SYS_CHARGE:
				sysDis.disCnt = 0;
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysDis.errCnt = 0;
				sysCom.buzzState = BUZZ_CHARGE;
				setPowerMCU_ON();
				if (sysCom.cnt < 65000) {
					if (sysCom.cnt == 0)
						memcpy(currentMAX, current_ADC_CHARGE_MAX, 12);
					sysCom.cnt++;
				}
				if (sysCom.cnt < SYS_DELAY_RELAY_CHARGE) {
					if (sysCom.cnt == 0) {
						doneCalcuRMS = 0;
						sysCharg.overCurrStep = 0;
						Dpart = 0;
						Ipart = 0;
					}
					sysCharg.resetChargCnt = 0;
					outputShortFlag = 0;
				} else {
					sysDis.relayFlag = TRUE;
					//bao ve Ngắn mạch
					if (outputShortFlag)
						sysCom.state = SYS_CHARGE_ERR;
					//bao ve qua tai 1, check after 1 cycle of AC output (calculated rms current)
					if (doneCalcuRMS == 1) {
						switch (sysCharg.overCurrStep) {
						case 0: //while do not have over RMS
							sysCharg.checkOverCurCnt = 0;
							if (out_curr_data.RMS > OUTPUT_GRID_OVERLOAD)
								sysCharg.overCurrStep++;
							break;
						case 1: //from 1-29 cycles after Irms>OUTPUT_DIS_OVERLOAD2
							if (out_curr_data.RMS > ((uint16_t) overRMSratio[sysCharg.checkOverCurCnt] * OUTPUT_GRID_OVERLOAD / 100 + OUTPUT_GRID_OVERLOAD)) {
								sysCom.state = SYS_CHARGE_ERR;
							}
							if (sysCharg.checkOverCurCnt < 29)
								sysCharg.checkOverCurCnt++;
							else
								sysCharg.overCurrStep++;
							break;
						case 2: //from 30-39 cycles after Irms> OUTPUT_DIS_OVERLOAD2
							if (out_curr_data.RMS > OUTPUT_GRID_OVERLOAD) {
								sysCom.state = SYS_CHARGE_ERR;
							}
							if (sysCharg.checkOverCurCnt < 39)
								sysCharg.checkOverCurCnt++;
							else
								sysCharg.overCurrStep = 0;
							break;
						}
						doneCalcuRMS = 0;
					}
					//check if GRID off, change to DISCHARGE STATE
					if ((sysCom.cnt >= SYS_DELAY_RELAY_CHARGE + 400) && (sysCharg.lineFlag == 0))
						sysCom.state = SYS_DIS;
				}
				switch (sysCharg.chargeState) {
				case 0:			//charge const power state
					sysCom.ledState = LED_CHARGE;
					sysCharg.floatVoltageSet = 3300;
					if (Vbat_data.longRMS < 2484)
						SOC = 1;
					else if (Vbat_data.longRMS < 2600)
						SOC = 2;
					else if (Vbat_data.longRMS < 2900)
						SOC = 3;
					else {
						sysCharg.timeStartState1 = sysCom.clock.per20minutes;
						sysCharg.chargeState = 1;
						Dpart = 0;
						Ipart = 0;
					}
					break;
				case 1:		//charge high const voltage state
					SOC = 4;
					sysCom.ledState = LED_CHARGE;
					sysCharg.floatVoltageSet = 3000;
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
					if (sysCharg.timeStartState1 < (0xFFFF - timeOfChargeState)) {
						if (sysCom.clock.per20minutes >= (sysCharg.timeStartState1 + timeOfChargeState)) {
							sysCharg.chargeState = 2;
						}
					} else {
						if (sysCom.clock.per20minutes >= (30 - (0xFFFF - sysCharg.timeStartState1))) {
							sysCharg.chargeState = 2;
							Dpart = 0;
							Ipart = 0;
						}
					}
					break;
				case 2:	//charge floating voltage state
					SOC = 5;
					sysCom.ledState = LED_FULLCHARGE;
					sysCharg.floatVoltageSet = 2730;
					//check if charge over 30 days
					if (sysCharg.timeOnline.day < 30)
						timeOnlineContinuos();
					else {
						sysCom.state = SYS_AUTO_DIS;
						sysCom.autoDisState = 0;
					}
					break;
				}
				//PI to conftrol float voltage
				chargeVolADJ(sysCharg.floatVoltageSet);
				//reset chargeState if Vbat<26.5 over 60s
				if (sysCharg.resetChargCnt < 60000) {
					if (Vbat_data.longRMS < 2650)
						sysCharg.resetChargCnt++;
					else
						sysCharg.resetChargCnt = 0;
				} else {
					sysCharg.resetChargCnt = 0;
					sysCharg.chargeState = 0;
				}
				break;
			case SYS_AUTO_DIS:
				setPowerMCU_ON();
				switch (sysCom.autoDisState) {
				case 0: //delay 50ms
					sysDis.PPflag = FALSE;
					sysDis.PWMflag = FALSE;
					sysDis.checkPPflag = FALSE;
					sysDis.underBattVolCnt = 0;
					sysDis.overCurrStep = 0;
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
					sysCom.buzzState = BUZZ_DIS;
					sysCom.ledState = LED_AUTO_DIS;
					doneCalcuRMS = 0;
					memcpy(currentMAX, current_ADC_DIS_MAX, 12);
					if (sysCom.cnt < 50)
						++sysCom.cnt;
					else {
						sysCom.cnt = 0;
						++sysCom.autoDisState;
					}
					break;
				case 1:
					sysDis.PPflag = TRUE;
					if (sysCom.cnt < 200) {
						sysCom.cnt++;
						if (sysCom.cnt >= 150) {
							sysDis.checkPPflag = TRUE;
						} else
							sysDis.checkPPflag = FALSE;
					} else {
						sysCom.cnt = 0;
						++sysCom.autoDisState;
					}
					break;
				case 2:
					sysDis.PPflag = FALSE;
					sysDis.checkPPflag = FALSE;
					if (sysCom.cnt < 50)
						sysCom.cnt++;
					else {
						sysDis.relayFlag = FALSE;
						sysCom.cnt = 0;
						++sysCom.autoDisState;
					}
					break;
				case 3:
					if (sysCom.cnt >= SYS_DELAY_CHECK_PP - SYS_DELAY_RELAY_DIS) {
						sysDis.checkPPflag = TRUE;
					} else {
						sysDis.checkPPflag = FALSE;
					}
					if (sysCom.cnt < SYS_DELAY_PWM - SYS_DELAY_RELAY_DIS) {
						++sysCom.cnt;
						if (sysCom.cnt < SYS_DELAY_PP - SYS_DELAY_RELAY_DIS)
							sysDis.PPflag = FALSE;
						else {
							sysDis.PPflag = TRUE;
						}
						sysDis.PWMflag = FALSE;
					} else {
						sysDis.PWMflag = TRUE;
					}
					if (sysCharg.timeOnline.day >= 1) {
						backToChargeSystem();
						break;
					} else                 // QUA 1 NGAY
					{
						timeOnlineContinuos();
						// kiem tra AQ yeu
						if (Vbat_data.longRMS < 2250) {
							if (sysDis.underBattVolCnt < 2000) {
								sysDis.underBattVolCnt++;
							} else {
								backToChargeSystem();
							}
						} else
							sysDis.underBattVolCnt = 0;
						//bao ve qua tai 1, check after 1 cycle of AC output (calculated rms current)
						if (doneCalcuRMS == 1) {
							switch (sysDis.overCurrStep) {
							case 0: //while do not have over RMS
								sysDis.checkOverCurCnt = 0;
								if (out_curr_data.RMS > OUTPUT_DIS_OVERLOAD2)
									sysDis.overCurrStep++;
								break;
							case 1: //from 1-29 cycles after Irms>OUTPUT_DIS_OVERLOAD2
								if (out_curr_data.RMS > ((uint16_t) overRMSratio[sysDis.checkOverCurCnt] * OUTPUT_DIS_OVERLOAD2 / 100 + OUTPUT_DIS_OVERLOAD2))
									backToChargeSystem();
								if (sysDis.checkOverCurCnt < 29)
									sysDis.checkOverCurCnt++;
								else
									sysDis.overCurrStep++;
								break;
							case 2: //from 30-39 cycles after Irms> OUTPUT_DIS_OVERLOAD2
								if (out_curr_data.RMS > OUTPUT_DIS_OVERLOAD2)
									backToChargeSystem();
								if (sysDis.checkOverCurCnt < 39)
									sysDis.checkOverCurCnt++;
								else
									sysDis.overCurrStep = 0;
								break;
							}
							doneCalcuRMS = 0;
						}
						// output short circuit protect
						if (outputShortFlag) {
							outputShortFlag = 0;
							backToChargeSystem();
						}
					}
					break;
				}
				disSOCCalcu(&SOC);
				setDutyDEC(0x320);
				if (sysCharg.lineFlag == 0) {
					sysCom.state = SYS_DIS;
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
				} else {
					if (checkPP(sysDis.checkPPflag) == TRUE)
						backToChargeSystem();
				}
				break;
			case SYS_CHARGE_ERR:
				setPowerMCU_ON();
				outputShortFlag = 0;
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysDis.relayFlag = FALSE;
				sysCom.ledState = LED_CHARGE_ERR;
				if (sysCharg.lineFlag == 0)
					sysCom.state = SYS_DIS;
				else {
					if (sysCom.cnt < 5002) {
						++sysCom.cnt;
					} else
						sysCom.state = SYS_CHARGE;
				}
				break;
			}
			if (sysCom.state != SYS_STARTUP) {
				if (timeOut10ms == 0) {
					buzzerUpdate(sysCom.buzzState);
					timeOut10ms = 9;
				} else
					--timeOut10ms;
				if ((D_TYPE == 0) || (varCntTest > 350))
					ledUpdate(sysCom.ledState, sysDis.errCode);
				else
					LED_D_UPDATE(SOC, sysCom.ledState);
				if ((sysCom.state == SYS_DIS) || (sysCom.state == SYS_DIS_ERR)) {
					if (sysDis.pushButton == 0)
						sysDis.released = TRUE;
					if ((sysDis.released == TRUE) && (sysDis.pushButton))
						sysCom.state = SYS_STOP;
				}
				pushButtonCheck(&(sysDis.pushButton)); 			//check push button
				lineInCheck(&sysCharg.lineFlag);
				PPcontrol(sysDis.PPflag); 				//control ON/OFF push pull
				RELAYcontrol(sysDis.relayFlag); 			// control ON/OFF relay
				phaseCalcu(HV_data.RMS);				//calcu phase for PWM adjust
				//setDataOut();				//set data to arr to read out by UART
				dataFlashHandle1();
				if (sysCom.state != sysCom.stateOld) {
					sysCom.stateOld = sysCom.state;
					sysCom.cnt = 0;
				}
			}
			/*if (uartCnt < 1000) {
			 uartCnt++;
			 } else {
			 uartCnt = 0;
			 dataOut[0] = ' ';
			 dataOut[1] = (out_curr_data.RMS % 1000) / 100 + 48;
			 dataOut[2] = (out_curr_data.RMS % 100) / 10 + 48;
			 dataOut[3] = (out_curr_data.RMS % 10) / 1 + 48;
			 R_UART0_Send(dataOut, 4);
			 }*/
			PWMcontrol(sysDis.PWMflag);			//control ON/OFF-adjust duty PWM
			setTAU0_2flag();
		}
		if (gFdlStatus == 0)
			__halt();
	}
	/* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: R_MAIN_UserInit
* Description  : This function adds user code before implementing main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_MAIN_UserInit(void)
{
    /* Start user code. Do not edit comment generated here */
	R_TAU0_Channel0_Start();
	R_TAU0_Channel2_Start();
	ADCdataInit();
	R_ADC_Set_OperationOn();
	R_UART0_Start();
	R_UART0_Receive(&RXdataReceiver, 1);
	EI();
	/* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
///////////////////////////
void timeOnlineContinuos(void) {
	static uint16_t tick = 0;
	if (sysCharg.timeOnline.minute < 1440) {
		if (tick < 60000)
			tick++;
		else  //60809
		{
			sysCharg.timeOnline.minute++;
			tick = 0;
		}
	} else {
		sysCharg.timeOnline.minute = 0;
		tick = 0;
		sysCharg.timeOnline.day++;
	}
}
uint8_t saveDataToEEP(uint8_t errCode) {
	switch (errCode) {
	case 0x01:                 //over RMS current
		return startWriteToEEP1(151);
	case 0x02:
		break;
	case 0x03:                 //low batt while discharge
		return startWriteToEEP1(106);
	case 0x04:                 //output short circuit while discharge
		return startWriteToEEP1(201);
	case 0x05:                 //low PP voltage
		if (sysDis.state >= DIS_PWM_ON)
			return startWriteToEEP1(201);
		else
			return startWriteToEEP1(6);
	case 0x06:                 //over discharge time
		return startWriteToEEP1(6);
	}
	return 1;
}
void saveDataTo_dataEEP(uint8_t errCode) {
	switch (errCode) {
	case 0x01:                 //over RMS current
		dataEEP[0] = 0xFF;
		dataEEP[1] = errCode;
		dataEEP[2] = sysDis.disCnt >> 24;
		dataEEP[3] = (sysDis.disCnt & 0xFFFFFF) >> 16;
		dataEEP[4] = (sysDis.disCnt & 0xFFFF) >> 8;
		dataEEP[5] = (sysDis.disCnt & 0xFF);
		//copy 100 rms current value to EEP
		memcpy(dataEEP + 6, currentRMSsave + currRMSsaveCnt, 100 - currRMSsaveCnt);
		memcpy(dataEEP + 106 - currRMSsaveCnt, currentRMSsave, currRMSsaveCnt);
		//copy 45 ave Vpp value to EEP
		memcpy(dataEEP + 106, vppAVEsave + vppAVEsaveCnt, 45 - vppAVEsaveCnt);
		memcpy(dataEEP + 151 - vppAVEsaveCnt, vppAVEsave, vppAVEsaveCnt);
		break;
	case 0x02:
		break;
	case 0x03:                 //low batt while discharge
		dataEEP[0] = 0xFF;
		dataEEP[1] = errCode;
		dataEEP[2] = sysDis.disCnt >> 24;
		dataEEP[3] = (sysDis.disCnt & 0xFFFFFF) >> 16;
		dataEEP[4] = (sysDis.disCnt & 0xFFFF) >> 8;
		dataEEP[5] = (sysDis.disCnt & 0xFF);
		memcpy(dataEEP + 6, vbattRMSsave + vbattRMSsaveCnt, 100 - vbattRMSsaveCnt);
		memcpy(dataEEP + 106 - vbattRMSsaveCnt, vbattRMSsave, vbattRMSsaveCnt);
		break;
	case 0x04:                 //output short circuit while discharge
		dataEEP[0] = 0xFF;
		dataEEP[1] = errCode;
		dataEEP[2] = sysDis.disCnt >> 24;
		dataEEP[3] = (sysDis.disCnt & 0xFFFFFF) >> 16;
		dataEEP[4] = (sysDis.disCnt & 0xFFFF) >> 8;
		dataEEP[5] = (sysDis.disCnt & 0xFF);
		//copy 100 rms current value to EEP
		memcpy(dataEEP + 6, currentRMSsave + currRMSsaveCnt, 100 - currRMSsaveCnt);
		memcpy(dataEEP + 106 - currRMSsaveCnt, currentRMSsave, currRMSsaveCnt);
		//copy 45 ave current value to EEP
		memcpy(dataEEP + 106, currentAVEsave + currAVEsaveCnt, 45 - currAVEsaveCnt);
		memcpy(dataEEP + 151 - currAVEsaveCnt, currentAVEsave, currAVEsaveCnt);
		//copy 5 adc current value to EEP
		memcpy(dataEEP + 151, currentADCsave + currADCsaveCnt, 5 - currADCsaveCnt);
		memcpy(dataEEP + 156 - currADCsaveCnt, currentADCsave, currADCsaveCnt);
		//copy 45 ave Vpp value to EEP
		memcpy(dataEEP + 156, vppAVEsave + vppAVEsaveCnt, 45 - vppAVEsaveCnt);
		memcpy(dataEEP + 201 - vppAVEsaveCnt, vppAVEsave, vppAVEsaveCnt);
		break;
	case 0x05:                 //low PP voltage
		dataEEP[0] = 0xFF;
		dataEEP[1] = errCode;
		dataEEP[2] = sysDis.disCnt >> 24;
		dataEEP[3] = (sysDis.disCnt & 0xFFFFFF) >> 16;
		dataEEP[4] = (sysDis.disCnt & 0xFFFF) >> 8;
		dataEEP[5] = (sysDis.disCnt & 0xFF);
		if (sysDis.state >= DIS_PWM_ON) {
			//copy 100 rms current value to EEP
			memcpy(dataEEP + 6, currentRMSsave + currRMSsaveCnt, 100 - currRMSsaveCnt);
			memcpy(dataEEP + 106 - currRMSsaveCnt, currentRMSsave, currRMSsaveCnt);
			//copy 45 ave current value to EEP
			memcpy(dataEEP + 106, currentAVEsave + currAVEsaveCnt, 45 - currAVEsaveCnt);
			memcpy(dataEEP + 151 - currAVEsaveCnt, currentAVEsave, currAVEsaveCnt);
			//copy 5 adc current value to EEP
			memcpy(dataEEP + 151, currentADCsave + currADCsaveCnt, 5 - currADCsaveCnt);
			memcpy(dataEEP + 156 - currADCsaveCnt, currentADCsave, currADCsaveCnt);
			//copy 45 ave Vpp value to EEP
			memcpy(dataEEP + 156, vbattAVEsave + vbattAVEsaveCnt, 45 - vbattAVEsaveCnt);
			memcpy(dataEEP + 201 - vbattAVEsaveCnt, vbattAVEsave, vbattAVEsaveCnt);
		}
		break;
	case 0x06:                 //over discharge time
		dataEEP[0] = 0xFF;
		dataEEP[1] = errCode;
		dataEEP[2] = sysDis.disCnt >> 24;
		dataEEP[3] = (sysDis.disCnt & 0xFFFFFF) >> 16;
		dataEEP[4] = (sysDis.disCnt & 0xFFFF) >> 8;
		dataEEP[5] = (sysDis.disCnt & 0xFF);
		break;
	}
}
///////////////////////////
void backToChargeSystem(void) {
	sysCharg.timeOnline.day = 0;
	sysCharg.timeOnline.minute = 0;
	sysCharg.chargeState = 0;
	sysCom.state = SYS_CHARGE;
}
boolean checkPP(boolean checkEnable) {
	static uint8_t PPCheckCnt = 0;
	boolean resultTemp = FALSE;
	if (checkEnable == TRUE) {
		if (PPCheckCnt < 10) {
			if (HV_data.ADC < 62)
				PPCheckCnt++;
			else
				PPCheckCnt = 0;
		} else
			resultTemp = TRUE;
	} else
		PPCheckCnt = 0;
	return resultTemp;
}
uint8_t chargeVolADJ(uint16_t volSet) {
	static uint16_t timeADJ = 0;
	uint16_t errFloat;
	if (volSet >= 3300) {
		setDutyDEC(0x00);
	} else {
		if (timeADJ < 100)
			timeADJ++;
		else {
			timeADJ = 0;
			if (Vbat_data.RMS < volSet) {
				errFloat = volSet - Vbat_data.RMS;
				Dpart = errFloat / 2;
				Ipart -= ((float) errFloat) / 6;
				if (Ipart > Dpart)
					setDutyDEC((uint16_t) Ipart - Dpart);
				else
					setDutyDEC(0);
			} else {
				errFloat = Vbat_data.RMS - volSet;
				Dpart = errFloat / 2;
				Ipart += (float) errFloat / 6;
				Dpart += (uint16_t) Ipart;
				setDutyDEC(Dpart);
			}
		}
	}
	return 1;
}
void disSOCCalcu(unsigned char *state) {
	unsigned char *socState;
	socState = state;
	switch (*socState) {
	case 1:
		if (Vbat_data.RMS < 2000)
			(*socState = 0);
		break;
	case 2:
		if (Vbat_data.RMS < 2150)
			(*socState) = 1;
		break;
	case 3:
		if (Vbat_data.RMS < 2200)
			(*socState) = 2;
		break;
	case 4:
	case 5:
		if (Vbat_data.RMS < 2300)
			(*socState) = 3;
		break;
	}
}
/* End user code. Do not edit comment generated here */
