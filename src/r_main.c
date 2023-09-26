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
* File Name    : r_main.c
* Version      : CodeGenerator for RL78/G14 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements main function.
* Creation Date: 10/7/2022
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
//extern
extern uint8_t dataReadOut[10][4];
extern uint8_t data1024Arr[1024];
extern uint8_t uartBlockReadFlag;
extern uint8_t gFdlStatus;
extern uint8_t dataFlashState;
//
uint16_t dataFlashTimer = 2000;
uint8_t dataSaveArr[4];
uint8_t timeOut10ms = 9;
sysCommon_t sysCom;
sysDis_t sysDis;
sysCharge_t sysCharg;
uint16_t Dpart = 0;
float Ipart = 0;
uint16_t varCntTest = 0;
uint8_t timeOfChargeState = 15, SOC = 1;
uint8_t outputShortFlag = 0;
extern analogData_t HV_data, Vbat_data, line_vol_data, out_curr_data;
void disSOCCalcu(unsigned char *state);
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
	//delay 400mS to wait stable VCC for MCU
	BUZZER = 0;
	RELAY = 0;
	ON_24V_MCU = 0;
	SD_PP = 0;
	LED_GREEN = 1;
	LED_YELLOW = 1;
	LED_RED = 0;
	setupTestCheck();
	while (sysCom.cnt < 100) {
		if (checkTAU0_2flag() == 0) {
			sysCom.cnt++;
			setTAU0_2flag();
		}
	}
	sysCom.cnt = 0;
	while (sysCom.cnt < 400) {
		if (checkTAU0_2flag() == 0) {
			sysCom.cnt++;
			if (P1_bit.no0 == 0)
				varCntTest++;
			setTAU0_2flag();
		}
	}
	if (varCntTest > 350)
		timeOfChargeState = 0;
	else
		timeOfChargeState = 15;
	clearTestCheck();
	sysCom.cnt = 0;
	uartSendSysCallBack(UART_ADDRESS_SYS, SYS_INIT, 0);
	while (1U) {
		if (checkTAU0_2flag() == 0) {
			switch (sysCom.state) {
			case SYS_INIT:
				if (sysCom.cnt < 20000)
					sysCom.cnt++;
				else
					sysCom.cnt = 0;
				//answer read block command from uart
				if (uartBlockReadFlag < 4) {
					if (dataFlashTimer == 2000) {
						readDataFlashBlock(uartBlockReadFlag);
						uartBlockReadFlag = 5;
						dataFlashTimer = 0;
					}
				} else if (uartBlockReadFlag == 5) {
					if (dataFlashState == DF_IDLE) {
						R_UART0_Send(data1024Arr, 1024);
						uartBlockReadFlag = 4;
					}
				}
				//check lineFlag or buttonFlag
				if (sysDis.pushButton) {
					if (Vbat_data.RMS > 1800) {
						SOC = 5;
						setPowerMCU_ON();
						sysCom.state = SYS_DIS;
						dataSaveArr[0] = sysCom.state << 4;	//4 bits high byte 0 = sysState;
						dataSaveArr[1] = 0xFE;
						dataSaveArr[2] = 0xFE;
						dataSaveArr[3] = 0xFE;
					}
				} else if (sysCharg.lineFlag == 1) {
					SOC = 1;
					setPowerMCU_ON();
					sysCom.state = SYS_CHARGE;
					dataSaveArr[0] = sysCom.state << 4;	//4 bits high byte 0 = sysState;
					dataSaveArr[1] = 0xFE;
					dataSaveArr[2] = 0xFE;
					dataSaveArr[3] = 0xFE;
				}
				break;
			case SYS_DIS:
				setPowerMCU_ON();
				if (sysCom.cnt <= 60000) {
					if (sysCom.cnt == 0) {
						sysDis.state = DIS_WAIT;
						uartSendSysCallBack(UART_ADDRESS_SYS, SYS_DIS, DIS_WAIT);
					}
					sysCom.cnt++;
				}
				switch (sysDis.state) {
				case DIS_WAIT:
					sysDis.PWMflag = FALSE;
					sysDis.checkPPflag = FALSE;
					sysDis.PPflag = FALSE;
					sysDis.underBattVolCnt = 0;
					sysDis.timeCheckCnt = 0;
					sysDis.disCnt = 0;
					sysDis.noLoadCnt = 0;
					sysCom.buzzState = BUZZ_DIS;
					sysCom.ledState = LED_DIS;
					setDutyDEC(0);
					outVoltageSet(230);
					if (sysCom.cnt >= SYS_DELAY_RELAY_DIS) {
						sysDis.state = DIS_RELAY_ON;
						uartSendSysCallBack(UART_ADDRESS_SYS, SYS_DIS, DIS_RELAY_ON);
					}
					break;
				case DIS_RELAY_ON:
					sysDis.relayFlag = FALSE;
					if (sysCom.cnt > SYS_DELAY_PP) {
						sysDis.state = DIS_PP_ON;
						uartSendSysCallBack(UART_ADDRESS_SYS, SYS_DIS, DIS_PP_ON);
					}
					break;
				case DIS_PP_ON:
					sysDis.PPflag = TRUE;
					if (sysCom.cnt > SYS_DELAY_CHECK_PP) {
						sysDis.state = DIS_CHECK_PP_ON;
						uartSendSysCallBack(UART_ADDRESS_SYS, SYS_DIS, DIS_CHECK_PP_ON);
					}
					break;
				case DIS_CHECK_PP_ON:
					sysDis.checkPPflag = TRUE;
					if (sysCom.cnt > SYS_DELAY_PWM) {
						sysDis.state = DIS_PWM_ON;
						uartSendSysCallBack(UART_ADDRESS_SYS, SYS_DIS, DIS_PWM_ON);
					}
					break;
				case DIS_PWM_ON:
					sysDis.PWMflag = TRUE;
					if (sysDis.timeCheckCnt <= 480000)	//480s 1 vong
						sysDis.timeCheckCnt++;
					else
						sysDis.timeCheckCnt = 0;
					//bao ve qua tai 1
					if ((sysDis.timeCheckCnt % 1600) == 0) {
						if (outCurrCheck(OUTPUT_DIS_OVERLOAD2, 1, 20) > 0) { //385
							sysCom.state = SYS_DIS_ERR;
							sysDis.errCode = 0x01;
							sysDis.errCnt++;
						} else if (outCurrCheck(OUTPUT_DIS_OVERLOAD1, 1, 20) > 0) {
							outVoltageSet(readOutVoltage() - 3);
							if (readOutVoltage() <= 200) {
								sysCom.state = SYS_DIS_ERR;
								sysDis.errCode = 0x02;
								sysDis.errCnt++;
							}
						} else if (outCurrCheck(OUTPUT_DIS_NORMAL, 0, 20) > 0) {
							outVoltageSet(230);
						}
					}
					//reset bien chargeState
					if (out_curr_data.RMS > 100) {
						sysDis.noLoadCnt = 0;
						if (Vbat_data.RMS < 2400)
							sysCharg.chargeState = 0;
					} else {
						if (sysDis.noLoadCnt < 300000)
							++sysDis.noLoadCnt;
						else {
							if (Vbat_data.RMS < 2550)
								sysCharg.chargeState = 0;
						}
					}
					//bao ve AQ yeu
					if (Vbat_data.RMS < 1900) {
						if (sysDis.underBattVolCnt < 2000) {
							sysDis.underBattVolCnt++;
						} else {
							sysCom.state = SYS_DIS_ERR;
							sysDis.errCode = 0x03;
							sysDis.errCnt += 9;
						}
					} else
						sysDis.underBattVolCnt = 0;
					// output short circuit protect
					if (outputShortFlag) {
						outputShortFlag = 0;
						sysCom.state = SYS_DIS_ERR;
						sysDis.errCode = 0x04;
						sysDis.errCnt += 1;
					}
					break;
				}
				disSOCCalcu(&SOC);
				//save data to per 30 mins
				if ((sysCom.cnt >= 20000) && (sysCom.cnt <= 22002)) { //1. wait 20s to save change state data
					if (dataFlashTimer == 2000) {
						sysCom.cnt = 22003;
						sendToDataFlashBuffer(dataSaveArr);
						dataSaveArr[0] = (sysCom.state << 4);
						dataFlashTimer = 0;
					}
				} else if (sysCom.cnt > 22002) { //2. start saving data when have load
					//send data to data Flash;
					if (out_curr_data.RMS >= 40) {	//0.2A
						//gia tri time truoc khi ngat tai
						dataSaveArr[0] += (uint8_t) ((sysDis.disCnt / 30000) >> 8);	//4 bits high of time = 4 bit low byte 0
						dataSaveArr[1] = (uint8_t) (sysDis.disCnt / 30000);	//8bit low of time (1 index = 5 minutes)
						//gia tri Vbat truoc khi ngat tai
						dataSaveArr[2] = (uint8_t) (Vbat_data.RMS / 10);
						//lay gia tri dong rms max (ex:4.5A = 45)
						dataSaveArr[3] = (uint8_t) (out_curr_data.RMS / 10);
						//					if ((out_curr_data.RMS / 10) > dataSaveArr[3])
						//						dataSaveArr[3] = (uint8_t) (out_curr_data.RMS / 10);
					}
					//3. save data to dataFlash per 4h
					if (((sysDis.disCnt % 1440000) == 0) && (dataFlashTimer == 2000)) {
						if (dataSaveArr[3] == 0)
							dataSaveArr[2] = (uint8_t) (Vbat_data.RMS / 10);
						sendToDataFlashBuffer(dataSaveArr);
						dataSaveArr[0] = (sysCom.state << 4);
						dataFlashTimer = 0;
					}
				}
				//protect over time discharge and save data to data flash per 5 mins
				if (timeOut10ms == 0) {
					if (sysDis.disCnt < MAX_TIME_DIS)
						sysDis.disCnt++;
					else {
						sysDis.errCode = 0x05;
						sysDis.errCnt = 20;
						sysCom.state = SYS_DIS_ERR;
					}
				}
				//reset timming counter online if batt not full
				if (Vbat_data.RMS < 2550) {
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
				}
				if (sysDis.pushButton == 0)
					sysDis.released = TRUE;
				if ((sysDis.released == TRUE) && (sysDis.pushButton))
					sysCom.state = SYS_STOP;
				if (sysCharg.lineFlag == 1)
					sysCom.state = SYS_CHARGE;
				break;
			case SYS_AUTO_DIS:
				setPowerMCU_ON();
				switch (sysCom.autoDisState) {
				case 0: //delay 50ms
					sysDis.PPflag = FALSE;
					sysDis.PWMflag = FALSE;
					sysDis.underBattVolCnt = 0;
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
					sysCharg.floatVoltageSet = 2300;
					sysCom.buzzState = BUZZ_DIS;
					sysCom.ledState = LED_AUTO_DIS;
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
					} else
						sysDis.checkPPflag = FALSE;
					if (sysCom.cnt < SYS_DELAY_PWM - SYS_DELAY_RELAY_DIS) {
						++sysCom.cnt;
						if (sysCom.cnt < SYS_DELAY_PP - SYS_DELAY_RELAY_DIS)
							sysDis.PPflag = FALSE;
						else
							sysDis.PPflag = TRUE;
					} else {
						if (sysCom.cnt < 60000)
							sysCom.cnt++;
						if ((sysCom.cnt >= 20000) && (sysCom.cnt <= 22002)) { //1. wait 20s to save change state data
							if (dataFlashTimer == 2000) {
								sendToDataFlashBuffer(dataSaveArr);
								dataSaveArr[0] = (sysCom.state << 4);
								dataFlashTimer = 0;
								sysCom.cnt = 22003;
							}
						}
						sysDis.PWMflag = TRUE;
						if (sysCharg.timeOnline.day >= 1) {
							backToChargeSystem();
							break;
						} else                 // QUA 1 NGAY
						{
							timeOnlineContinuos();
							// kiem tra AQ yeu
							if (Vbat_data.RMS < 2250) {
								if (sysDis.underBattVolCnt < 2000) {
									sysDis.underBattVolCnt++;
								} else {
									backToChargeSystem();
								}
							} else
								sysDis.underBattVolCnt = 0;
							// kiem tra qua tai
							if (outCurrCheck(OUTPUT_DIS_OVERLOAD1, 1, 20) > 0)
								backToChargeSystem();
							// output short circuit protect
							if (outputShortFlag) {
								outputShortFlag = 0;
								backToChargeSystem();
							}
						}
					}
					break;
				}
				disSOCCalcu(&SOC);
				chargeVolADJ(sysCharg.floatVoltageSet);
				if (sysCharg.lineFlag == 0) {
					sysCom.state = SYS_DIS;
					sysCharg.timeOnline.day = 0;
					sysCharg.timeOnline.minute = 0;
				}
				break;
			case SYS_DIS_ERR:
				setPowerMCU_ON();
				if (sysCom.cnt == 0) {
					uartSendSysCallBack(UART_ADDRESS_SYS, SYS_DIS_ERR, sysDis.errCode);
					dataSaveArr[0] = sysCom.state << 4;	//4 bits high byte 0 = sysState;
					dataSaveArr[0] |= sysDis.errCode;
					dataSaveArr[1] = sysDis.errCnt;
				}
				sysDis.PPflag = FALSE;
				sysDis.PWMflag = FALSE;
				sysCom.ledState = LED_ERRSTATE;
				sysCom.buzzState = BUZZ_ERR;
				//save data to dataFlash
				if (sysCom.cnt < 7002) {
					if ((sysCom.cnt == 5000) && (dataFlashTimer == 2000)) {
						sendToDataFlashBuffer(dataSaveArr);
						dataFlashTimer = 0;
					}
					++sysCom.cnt;
				} else {
					if (sysDis.errCnt >= 20)
						sysCom.state = SYS_STOP;
					else
						sysCom.state = SYS_DIS;
				}
				if ((sysDis.released == TRUE) && (sysDis.pushButton))
					sysCom.state = SYS_STOP;
				if (sysCharg.lineFlag == 1)
					sysCom.state = SYS_CHARGE;
				break;
			case SYS_STOP:
				if (sysCom.cnt == 0)
					uartSendSysCallBack(UART_ADDRESS_SYS, SYS_STOP, 0);
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysDis.relayFlag = FALSE;
				sysCom.ledState = LED_OFF;
				setPowerMCU_ON();
				if (sysCom.cnt < 2100) {
					if ((sysCom.cnt < 100) && (dataFlashTimer == 2000)) {
						sendToDataFlashBuffer(dataSaveArr);
						sysCom.cnt = 100;
						dataFlashTimer = 0;
					}
					sysCom.cnt++;
				} else
					sysCom.state = SYS_OFF;
				sysCom.buzzState = BUZZ_STOP;
				if (sysCharg.lineFlag == 1) {
					sysCom.state = SYS_CHARGE;
				}
				break;
			case SYS_OFF:
				if (sysCom.cnt == 0) {
					uartSendSysCallBack(UART_ADDRESS_SYS, SYS_OFF, 0);
					sysCom.cnt = 1;
				}
				clearPowerMCU_ON();
				if (sysCharg.lineFlag == 1) {
					sysCom.state = SYS_CHARGE;
				}
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysCom.buzzState = BUZZ_OFF;
				sysCom.ledState = LED_OFF;
				break;
			case SYS_CHARGE:
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysDis.errCnt = 0;
				sysCom.buzzState = BUZZ_CHARGE;
				setPowerMCU_ON();
				if (sysCom.cnt < 65000)
					sysCom.cnt++;
				if (sysCom.cnt < SYS_DELAY_RELAY_CHARGE) {
					if (sysCom.cnt == 0) {
						uartSendSysCallBack(UART_ADDRESS_SYS, SYS_CHARGE, sysCharg.chargeState);
						sysCharg.chargCnt = 0;
						Dpart = 0;
						Ipart = 0;
					}
					sysCharg.resetChargCnt = 0;
					dataSaveArr[3] = 0;
				} else {
					sysDis.relayFlag = TRUE;
					//protect over power
					if (outCurrCheck(OUTPUT_GRID_OVERLOAD, 1, 20) > 0) { //450
						sysCom.state = SYS_CHARGE_ERR;
					}
					if (outputShortFlag) {
						outputShortFlag = 0;
						sysCom.state = SYS_CHARGE_ERR;
					}
				}
				switch (sysCharg.chargeState) {
				case 0:			//charge const power state
					sysCom.ledState = LED_CHARGE;
					sysCharg.floatVoltageSet = 3300;
					if (Vbat_data.RMS < 2484)
						SOC = 1;
					else if (Vbat_data.RMS < 2600)
						SOC = 2;
					else if (Vbat_data.RMS < 2900)
						SOC = 3;
					else {
						sysCharg.timeStartState1 = sysCom.clock.per20minutes;
						sysCharg.chargeState = 1;
						uartSendSysCallBack(UART_ADDRESS_SYS, SYS_CHARGE, 1);
					}
					break;
				case 1:		//charge high const voltage state
					SOC = 4;
					sysCom.ledState = LED_CHARGE;
					sysCharg.floatVoltageSet = 3000;
					if (sysCharg.timeStartState1 < (0xFFFF - timeOfChargeState)) {
						if (sysCom.clock.per20minutes >= (sysCharg.timeStartState1 + timeOfChargeState)) {
							sysCharg.chargeState = 2;
							uartSendSysCallBack(UART_ADDRESS_SYS, SYS_CHARGE, 2);
						}
					} else {
						if (sysCom.clock.per20minutes >= (30 - (0xFFFF - sysCharg.timeStartState1))) {
							sysCharg.chargeState = 2;
							uartSendSysCallBack(UART_ADDRESS_SYS, SYS_CHARGE, 2);
						}
					}
					break;
				case 2:	//charge floating voltage state
					SOC = 5;
					sysCom.ledState = LED_FULLCHARGE;
					sysCharg.floatVoltageSet = 2730;
					break;
				}
				if (sysCharg.resetChargCnt < 60000) {
					if (Vbat_data.RMS < 2650)
						sysCharg.resetChargCnt++;
					else
						sysCharg.resetChargCnt = 0;
				} else {
					sysCharg.resetChargCnt = 0;
					sysCharg.chargeState = 0;
					uartSendSysCallBack(UART_ADDRESS_SYS, SYS_CHARGE, 0);
				}
				//send data to data Flash;
				if ((sysCom.cnt >= 20000) && (sysCom.cnt <= 22002)) { //1. wait 20s to save change state data
					if (dataFlashTimer == 2000) {
						sendToDataFlashBuffer(dataSaveArr);
						dataSaveArr[0] = (sysCom.state << 4);
						dataFlashTimer = 0;
						sysCom.cnt = 22003;
					}
				} else if (sysCom.cnt > 22002) { //2. start saving data when have load
					//send data to data Flash;
					if ((out_curr_data.RMS / 10) >= dataSaveArr[3]) {	//0.2A
						//gia tri time truoc khi ngat tai
						dataSaveArr[0] = (uint8_t) ((sysCharg.chargCnt / 30000) >> 8) + dataSaveArr[0] & 0xF0;			//4 bits high of time = 4 bit low byte 0
						dataSaveArr[1] = (uint8_t) (sysCharg.chargCnt / 30000);						//8bit low of time (1 index = 2 minutes)
						//lay gia tri dong rms max (ex:4.5A = 45)
						dataSaveArr[3] = (uint8_t) (out_curr_data.RMS / 10);
						//					if ((out_curr_data.RMS / 10) > dataSaveArr[3])
						//						dataSaveArr[3] = (uint8_t) (out_curr_data.RMS / 10);
					}
					//VBatt rms before save to dataFlash
					dataSaveArr[2] = (uint8_t) (Vbat_data.RMS / 10);
					//3. save data to dataFlash per 2 day
					if ((sysCharg.chargCnt >= 17280000) && (dataFlashTimer == 2000)) {
						if (dataSaveArr[3] == 0)
							dataSaveArr[1] = sysCharg.chargeState;
						sendToDataFlashBuffer(dataSaveArr);
						dataSaveArr[0] = (sysCom.state << 4);
						dataFlashTimer = 0;
						sysCharg.chargCnt = 0;
					}
				}
				if (timeOut10ms == 0)
					sysCharg.chargCnt++;
				//PI to conftrol float voltage
				chargeVolADJ(sysCharg.floatVoltageSet);
				//check if GRID off, change to DISCHARGE STATE
				if (sysCharg.lineFlag == 0) {
					sysCom.state = SYS_DIS;
					sysDis.state = DIS_WAIT;
				}
				if (sysCharg.timeOnline.day < 30)
					timeOnlineContinuos();
				else {
					sysCom.state = SYS_AUTO_DIS;
					sysCom.autoDisState = 0;
				}
				break;
			case SYS_CHARGE_ERR:
				setPowerMCU_ON();
				if (sysCom.cnt == 0) {
					uartSendSysCallBack(UART_ADDRESS_SYS, SYS_CHARGE_ERR, 0);
				}
				sysDis.PWMflag = FALSE;
				sysDis.PPflag = FALSE;
				sysDis.relayFlag = FALSE;
				sysCom.ledState = LED_CHARGE_ERR;
				if (sysCharg.lineFlag == 0) {
					sysCom.state = SYS_DIS;
					sysDis.state = DIS_WAIT;
				} else {
					if (sysCom.cnt < 7002) {
						if ((sysCom.cnt == 5000) && (dataFlashTimer == 2000)) {
							sendToDataFlashBuffer(dataSaveArr);
							dataFlashTimer = 0;
						}
						++sysCom.cnt;
					} else
						sysCom.state = SYS_CHARGE;
				}
				break;
			}
			if (timeOut10ms == 0) {
				buzzerUpdate(sysCom.buzzState);
				timeOut10ms = 9;
			} else
				--timeOut10ms;
			if (D_TYPE == 0)
				ledUpdate(sysCom.ledState, sysDis.errCode);
			else
				LED_D_UPDATE(SOC, sysCom.ledState);
			// writing data to data flash only start after before command executed 2s.
			if (dataFlashTimer < 2000)
				dataFlashTimer++;
			chainManager(); 				//execute all thing about data Flash
			pushButtonCheck(&(sysDis.pushButton)); 			//check push button
			lineVolCheck(line_vol_data.RMS, &(sysCharg.lineFlag)); //check line state
			PPcontrol(sysDis.PPflag); 				//controll ON/OFF push pull
			RELAYcontrol(sysDis.relayFlag); 			// controll ON/OFF relay
			phaseCalcu(HV_data.RMS);				//calcu phase for PWM adjust
			PWMcontrol(sysDis.PWMflag);			//control ON/OFF-adjust duty PWM
			setDataOut();				//set data to arr to read out by UART
			if (sysCom.state != sysCom.stateOld) {
				if (sysCom.stateOld != SYS_INIT) {
					//set data [0] to save in data Flash
					dataSaveArr[0] = sysCom.state << 4;	//4 bits high byte 0 = sysState;
					dataSaveArr[1] = 0;
					//save batt voltage RMS before change sysState
					dataSaveArr[2] = (uint8_t) (Vbat_data.RMS / 10);
					//save output current before change sysState
					dataSaveArr[3] = (uint8_t) (out_curr_data.RMS / 10);
				}
				sysCom.stateOld = sysCom.state;
				sysCom.cnt = 0;
			}
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
void setDataOut(void) {
	dataReadOut[0][0] = 0;
	dataReadOut[1][0] = 6;
	dataReadOut[2][0] = 5;
	dataReadOut[3][0] = 5;
	dataReadOut[4][0] = 5;
	dataReadOut[5][0] = 4;
	dataReadOut[6][0] = 4;
	dataReadOut[7][0] = 5;
	dataReadOut[1][1] = (uint8_t) (FIRMWARE_VER / 100);
	dataReadOut[1][2] = (uint8_t) ((FIRMWARE_VER % 100) / 10);
	dataReadOut[1][3] = (uint8_t) (FIRMWARE_VER % 10);
	dataReadOut[2][1] = (uint8_t) (Vbat_data.RMS >> 8);
	dataReadOut[2][2] = (uint8_t) (Vbat_data.RMS & 0xFF);
	dataReadOut[3][1] = (uint8_t) (out_curr_data.RMS >> 8);
	dataReadOut[3][2] = (uint8_t) (out_curr_data.RMS & 0xFF);
	dataReadOut[4][1] = (uint8_t) (HV_data.RMS >> 8);
	dataReadOut[4][2] = (uint8_t) (HV_data.RMS & 0xFF);
	dataReadOut[5][1] = sysDis.pushButton;
	dataReadOut[6][1] = sysCharg.lineFlag;
	dataReadOut[7][1] = sysCom.state;
	if (sysCom.state == SYS_DIS)
		dataReadOut[7][2] = sysDis.state;
	else if (sysCom.state == SYS_CHARGE)
		dataReadOut[7][2] = sysCharg.chargeState;
	else
		dataReadOut[7][2] = 0;
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
