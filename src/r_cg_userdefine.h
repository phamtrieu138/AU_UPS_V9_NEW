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
* File Name    : r_cg_userdefine.h
* Version      : CodeGenerator for RL78/G14 V2.05.06.02 [08 Nov 2021]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file includes user definition.
* Creation Date: 10/5/2022
***********************************************************************************************************************/

#ifndef _USER_DEF_H
#define _USER_DEF_H

/***********************************************************************************************************************
User definitions
***********************************************************************************************************************/

/* Start user code for function. Do not edit comment generated here */
#define	FIRMWARE_VER			903
#define ENABLE_LINE_OVERCURRENT	0
#define ENABLE_DECREASE_VOLTAGE	0
#define C750 1
#define D_TYPE 1
//define type of UPS
#if C750
	//C750
	#define OUTPUT_DIS_OVERLOAD2 	330		//385
	#define	MAX_TIME_DIS			20000000   //56h
#else
//C1200
	#define OUTPUT_DIS_OVERLOAD2 	440
	#define	MAX_TIME_DIS			28000000	//78h
#endif
//over current when grid on
#if ENABLE_LINE_OVERCURRENT
#if C750
#define OUTPUT_GRID_OVERLOAD 	450  //340
#else
		#define OUTPUT_GRID_OVERLOAD 	700  //340
	#endif
#else
	#define OUTPUT_GRID_OVERLOAD 	1000  //340
#endif
#if ENABLE_DECREASE_VOLTAGE
#if C750
	#define OUTPUT_DIS_OVERLOAD1 	300  //340
#else
		#define OUTPUT_DIS_OVERLOAD1 	350  //340
	#endif
#else
	#define OUTPUT_DIS_OVERLOAD1 	1000  //340
#endif
#define	TIME_A_DAY				8640000
#define OUTPUT_DIS_NORMAL 		200
//readwrite type define
#define WRITEDATA				1
#define	READBLOCK				2
#define	READBYTES				3
//DF_sate define
#define DF_READINGBLOCK0		0
#define DF_STARTWRITEBLOCK0		1
#define DF_WAITTOREADBLOCKX		2
#define DF_READINGBLOCKX		3
#define DF_WAITINGTOEND			4
#define DF_IDLE					5
#define DF_ONLYREADBLOCK0		6
#define DF_ONLYREADBLOCK1		7
#define DF_ONLYREADBLOCK2		8
#define DF_ONLYREADBLOCK3		9
#define SYS_DELAY_INIT 			1000
#define SYS_DELAY_CHECK_PP 		2350
#define SYS_DELAY_PWM	 		2800
#define SYS_DELAY_PP	 		2200
#define SYS_DELAY_RELAY_DIS		800
#define SYS_DELAY_RELAY_CHARGE 	2500
typedef enum {
	SYS_INIT = 0, SYS_DIS = 1, SYS_DIS_ERR = 2, SYS_STOP = 3, SYS_OFF = 4, SYS_CHARGE = 5, SYS_CHARGE_ERR = 6, SYS_AUTO_DIS = 7
} sysState_t;
typedef enum {
	DIS_WAIT = 0, DIS_RELAY_ON = 1, DIS_PP_ON = 2, DIS_CHECK_PP_ON = 3, DIS_PWM_ON = 4
} disState_t;
typedef enum {
	BUZZ_OFF = 0, BUZZ_DIS = 1, BUZZ_CHARGE = 2, BUZZ_ERR = 3, BUZZ_STOP = 4
} buzzer_t;
typedef enum {
	LED_OFF = 0, LED_DIS = 1, LED_AUTO_DIS = 2, LED_ERRSTATE = 3, LED_CHARGE = 4, LED_FULLCHARGE = 5, LED_CHARGE_ERR = 6, LED_IDLE = 7
} led_t;
typedef enum {
	FALSE = 0, TRUE = 1
} boolean;
typedef struct {
	uint16_t per20minutes;
	uint16_t sec;
	uint16_t tic;
} clock_t;
typedef struct {
	uint16_t day;
	uint16_t minute;
} timeOnline_t;
typedef struct {
	sysState_t state, stateOld;
	uint16_t cnt;
	clock_t clock;
	buzzer_t buzzState;
	led_t ledState;
	uint8_t autoDisState;
} sysCommon_t;
typedef struct {
	uint32_t disCnt;
	uint32_t timeCheckCnt;
	uint16_t underBattVolCnt;
	uint32_t noLoadCnt;
	disState_t state;
	uint8_t errCnt;
	uint8_t errCode;
	boolean PWMflag;
	boolean PPflag;
	boolean checkPPflag;
	boolean relayFlag;
	uint8_t pushButton;
	boolean released;
} sysDis_t;
typedef struct {
	uint32_t chargCnt;
	uint16_t resetChargCnt;
	uint16_t timeStartState1;
	uint16_t floatVoltageSet;
	uint8_t chargeState;
	uint8_t lineFlag;
	timeOnline_t timeOnline;
} sysCharge_t;
typedef struct {
	uint8_t length;
	uint8_t command;
	uint8_t address;
	uint8_t *data;
} uartData_t;
typedef struct {
	uint8_t comType;
	uint8_t *buff;
	uint16_t start;
	uint8_t length;
} comDataFlash_t;

void buzzerUpdate(buzzer_t state);
void ledUpdate(led_t state, uint8_t errCode);
void LED_D_UPDATE(unsigned char stateOfCharge, led_t ledState1);
void PPcontrol(boolean flag);
void RELAYcontrol(boolean flag);
uint8_t chargeVolADJ(uint16_t volSet);
void setDataOut(void);
void sendToDataFlashBuffer(uint8_t *data);
void readDataFlashBlock(uint8_t blockno);
uint8_t chainManager(void);
void backToChargeSystem(void);
void timeOnlineContinuos(void);
/* End user code. Do not edit comment generated here */
#endif
