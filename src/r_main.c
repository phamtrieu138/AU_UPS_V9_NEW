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
* File Name    : r_main.c
* Version      : CodeGenerator for RL78/G14 V2.05.09.01 [28 Apr 2025]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements main function.
* Creation Date: 11/24/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_port.h"
#include "r_cg_serial.h"
#include "r_cg_adc.h"
#include "r_cg_timer.h"
#include "r_cg_wdt.h"
/* Start user code for include. Do not edit comment generated here */
#include <string.h>
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
uint8_t dataUartOut[20];
void uartSendData(void);
extern uint16_t trieuHV;
extern uint16_t currentAve;
extern uint16_t phase;
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
//variants of uart
extern uint8_t RXdataReceiver;
sysCommon_t sysCom;
uint8_t timeOut10ms = 9;
uint16_t varCntTest = 0;
sysDis_t sysDis;
uint8_t outputShortFlag = 0;
uint8_t overRMScnt[8];
sysCharge_t sysCharg;
uint16_t Dpart = 0;
float Ipart = 0;
uint8_t timeOfChargeState = 15, SOC = 1;
static uint8_t overCurrentCnt = 0;
void disSOCCalcu(unsigned char *state);
boolean checkPP(boolean checkEnable);
battMin_t battMinCal;
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
	memcpy(currentADCMAX, currentADCMAXCHARGE, sizeof(currentADCMAX));
	memcpy(shortCntMAX, shortCntMAXDIS, sizeof(shortCntMAX));
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
			uartSendData();
			R_WDT_Restart();
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
				//check lineFlag or buttonFlag
				if (sysDis.pushButton) {
					if (Vbat_data.RMS > 1800) {
						SOC = 5;
						setPowerMCU_ON();
						//memcpy(currentMAX, current_ADC_DIS_MAX, 12);
						sysCom.state = SYS_DIS;
					}
				} else if (sysCharg.lineFlag == 1) {
					SOC = 1;
					setPowerMCU_ON();
					//memcpy(currentMAX, current_ADC_CHARGE_MAX, 12);
					sysCom.state = SYS_CHARGE;
				}
				break;
			case SYS_DIS:
				setPowerMCU_ON();
				if (sysCom.cnt <= 60000) {
					if (sysCom.cnt == 0) {
						sysDis.state = DIS_WAIT;
						doneCalcuRMS = 0;
						outputShortFlag = 0;
						memset(overRMScnt, 0, 7);
						//memcpy(currentMAX, current_ADC_DIS_MAX, 12);
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
					outVoltageSet(227);
					memcpy(currentADCMAX, currentADCMAXDIS,
							sizeof(currentADCMAX));
					memcpy(shortCntMAX, shortCntMAXDIS, sizeof(shortCntMAX));
					outputShortFlag = 0;
					memset(overSHORTcnt, 0, sizeof(overSHORTcnt));
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
					for (overCurrentCnt = 0; overCurrentCnt <= 7;
							overCurrentCnt++) {
						if (out_curr_data.RMS
								>= (uint16_t) ((uint32_t) overRMSratio[overCurrentCnt]
										* OUTPUT_DIS_OVERLOAD2 / 100))
							overRMScnt[overCurrentCnt]++;
						else {
							if (overRMScnt[overCurrentCnt] > 0)
								overRMScnt[overCurrentCnt]--;
						}
						if (overRMScnt[overCurrentCnt]
								>= overRMScntMAX[overCurrentCnt]) {
							sysCom.state = SYS_DIS_ERR;
							sysDis.errCode = 0x01;
							sysDis.errCnt++;
						}
					}
					if (out_curr_data.RMS < 320)
						outVoltageSet(227 + out_curr_data.RMS / 40);
					else
						outVoltageSet(232);
					//bao ve AQ yeu
					switch (battMinCal.battMinCalState) {
					case 0: //Dòng lớn
						vbatMIN = 1920;
						if (out_curr_data.RMS < 3) {
							if (battMinCal.noLoadCnt < 720000) //tương đương 4h-720000
								battMinCal.noLoadCnt++;
							else {
								battMinCal.battMinCalState = 1;
								vbatMIN = 2380;
							}
						} else
							battMinCal.noLoadCnt = 0;
						break;
					case 1: //Dòng nhỏ
						if (out_curr_data.RMS >= 3) //tương đương 30s
							battMinCal.battMinCalState = 0;
						battMinCal.noLoadCnt = 0;
						break;
					}
					doneCalcuRMS = 0;
				}
				//bao ve AQ yeu
				if (Vbat_data.RMS < vbatMIN) {
					if (sysDis.underBattVolCnt < 1000) {
						sysDis.underBattVolCnt++;
					} else {
						sysCom.state = SYS_DIS_ERR;
						sysDis.errCode = 0x03;
						sysDis.errCnt += 2;
					}
				} else
					sysDis.underBattVolCnt = 0;
				// output short circuit protect
				if (outputShortFlag) {
					sysCom.state = SYS_DIS_ERR;
					sysDis.errCode = 0x04;
					sysDis.errCnt++;
				}
				//low PP voltage
				if (checkPP(sysDis.checkPPflag) == TRUE) {
					sysCom.state = SYS_DIS_ERR;
					sysDis.errCode = 0x05;
					sysDis.errCnt++;
				}
				//protect over time discharge
				if (timeOut10ms == 0) {
					if (sysDis.disCnt < MAX_TIME_DIS)
						sysDis.disCnt++;
					else {
						sysCom.state = SYS_DIS_ERR;
						sysDis.errCode = 0x06;
						sysDis.errCnt += 4;
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
				sysDis.PPflag = FALSE;
				sysDis.PWMflag = FALSE;
				sysCom.ledState = LED_ERRSTATE;
				sysCom.buzzState = BUZZ_ERR;
				if (sysCom.cnt < 7002) {
					++sysCom.cnt;
				} else {
					if (sysDis.errCnt >= 4)
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
					sysCom.cnt++;
				}
				if (sysCom.cnt < SYS_DELAY_RELAY_CHARGE) {
					if (sysCom.cnt == 1) {
						doneCalcuRMS = 0;
						Dpart = 0;
						Ipart = 0;
						memset(overRMScnt, 0, 7);
						memset(overRMScnt, 0, 7);
						memcpy(currentADCMAX, currentADCMAXCHARGE,
								sizeof(currentADCMAX));
						memcpy(shortCntMAX, shortCntMAXCHARGE,
								sizeof(shortCntMAX));
						outputShortFlag = 0;
						sysCharg.errCode = 0;
						memset(overSHORTcnt, 0, sizeof(overSHORTcnt));
					}
					sysCharg.resetChargCnt = 0;
					outputShortFlag = 0;
				} else {
					sysDis.relayFlag = TRUE;
					//bao ve Ngắn mạch
					if (outputShortFlag) {
						sysCom.state = SYS_CHARGE_ERR;
						sysCharg.errCode = 4;
						outputShortFlag = 0;
					}
					//bao ve qua tai 1, check after 1 cycle of AC output (calculated rms current)
					if (doneCalcuRMS == 1) {
						for (overCurrentCnt = 0; overCurrentCnt <= 7;
								overCurrentCnt++) {
							if (out_curr_data.RMS
									>= (uint16_t) ((uint32_t) overRMSratio[overCurrentCnt]
											* OUTPUT_GRID_OVERLOAD / 100))
								overRMScnt[overCurrentCnt]++;
							else {
								if (overRMScnt[overCurrentCnt] > 0)
									overRMScnt[overCurrentCnt]--;
							}
							if (overRMScnt[overCurrentCnt]
									>= overRMScntMAX[overCurrentCnt]) {
								sysCom.state = SYS_CHARGE_ERR;
								sysCharg.errCode = 1;
							}
						}
						doneCalcuRMS = 0;
					}
					//check if GRID off, change to DISCHARGE STATE
					if ((sysCom.cnt >= SYS_DELAY_RELAY_CHARGE + 400)
							&& (sysCharg.lineFlag == 0))
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
					if (sysCharg.timeStartState1
							< (0xFFFF - timeOfChargeState)) {
						if (sysCom.clock.per20minutes
								>= (sysCharg.timeStartState1 + timeOfChargeState)) {
							sysCharg.chargeState = 2;
						}
					} else {
						if (sysCom.clock.per20minutes
								>= (30 - (0xFFFF - sysCharg.timeStartState1))) {
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
					memset(overRMScnt, 0, 7);
					memcpy(currentADCMAX, currentADCMAXDIS,
							sizeof(currentADCMAX));
					memcpy(shortCntMAX, shortCntMAXDIS, sizeof(shortCntMAX));
					outputShortFlag = 0;
					memset(overSHORTcnt, 0, sizeof(overSHORTcnt));
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
					sysCom.buzzState = BUZZ_DIS;
					sysCom.ledState = LED_AUTO_DIS;
					doneCalcuRMS = 0;
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
							for (overCurrentCnt = 0; overCurrentCnt <= 7;
									overCurrentCnt++) {
								if (out_curr_data.RMS
										>= (uint16_t) ((uint32_t) overRMSratio[overCurrentCnt]
												* OUTPUT_DIS_OVERLOAD2 / 100))
									overRMScnt[overCurrentCnt]++;
								else {
									if (overRMScnt[overCurrentCnt] > 0)
										overRMScnt[overCurrentCnt]--;
								}
								if (overRMScnt[overCurrentCnt]
										>= overRMScntMAX[overCurrentCnt]) {
									backToChargeSystem();
								}
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
					buzzerUpdate(sysCom.buzzState, sysDis.errCode);
					timeOut10ms = 9;
				} else
					--timeOut10ms;
				if ((D_TYPE == 0) || (varCntTest > 350))
					ledUpdate(sysCom.ledState, sysDis.errCode);
				else
					LED_D_UPDATE(SOC, sysCom.ledState);
				if ((sysCom.state == SYS_DIS)
						|| (sysCom.state == SYS_DIS_ERR)) {
					if (sysDis.pushButton == 0)
						sysDis.released = TRUE;
					if ((sysDis.released == TRUE) && (sysDis.pushButton))
						sysCom.state = SYS_STOP;
				}
				pushButtonCheck(&(sysDis.pushButton)); 		//check push button
				lineInCheck(&sysCharg.lineFlag);
				PPcontrol(sysDis.PPflag); 			//control ON/OFF push pull
				RELAYcontrol(sysDis.relayFlag); 		// control ON/OFF relay
				phaseCalcu(HV_data.RMS);			//calcu phase for PWM adjust
				//setDataOut();				//set data to arr to read out by UART
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
	//R_UART0_Receive(&RXdataReceiver, 1);
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
void uartSendData(void) {
	static uint16_t demUart = 0;
	uint16_t firmware = FIRMWARE_VER;
	if (demUart < 500)
		demUart++;
	else {
		demUart = 0;
		dataUartOut[0] = 0xAA;
		dataUartOut[1] = (uint8_t) (firmware);
		dataUartOut[2] = (uint8_t) (firmware >> 8);
		dataUartOut[3] = (uint8_t) (Vbat_data.RMS);
		dataUartOut[4] = (uint8_t) (Vbat_data.RMS >> 8);
		dataUartOut[5] = (uint8_t) (HV_data.RMS);
		dataUartOut[6] = (uint8_t) (HV_data.RMS >> 8);
		dataUartOut[7] = (uint8_t) (out_curr_data.RMS);
		dataUartOut[8] = (uint8_t) (out_curr_data.RMS >> 8);
		dataUartOut[9] = (uint8_t) (sysDis.errCode);
		dataUartOut[10] = (uint8_t) (sysDis.errCode >> 8);
		R_UART0_Send(dataUartOut, 11);
	}
}
/* End user code. Do not edit comment generated here */
