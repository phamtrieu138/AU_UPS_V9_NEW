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
* File Name    : r_cg_timer.c
* Version      : CodeGenerator for RL78/G14 V2.05.08.02 [03 Jun 2024]
* Device(s)    : R5F104AA
* Tool-Chain   : CCRL
* Description  : This file implements device driver for TAU module.
* Creation Date: 10/9/2025
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
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
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_TAU0_Create
* Description  : This function initializes the TAU0 module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TAU0_Create(void)
{
    TAU0EN = 1U;    /* supplies input clock */
    TPS0 = _0000_TAU_CKM0_FCLK_0 | _0000_TAU_CKM1_FCLK_0 | _0000_TAU_CKM2_FCLK_1 | _0000_TAU_CKM3_FCLK_8;
    /* Stop all channels */
    TT0 = _0001_TAU_CH0_STOP_TRG_ON | _0002_TAU_CH1_STOP_TRG_ON | _0004_TAU_CH2_STOP_TRG_ON |
          _0008_TAU_CH3_STOP_TRG_ON | _0200_TAU_CH1_H8_STOP_TRG_ON | _0800_TAU_CH3_H8_STOP_TRG_ON;
    /* Mask channel 0 interrupt */
    TMMK00 = 1U;    /* disable INTTM00 interrupt */
    TMIF00 = 0U;    /* clear INTTM00 interrupt flag */
    /* Mask channel 1 interrupt */
    TMMK01 = 1U;    /* disable INTTM01 interrupt */
    TMIF01 = 0U;    /* clear INTTM01 interrupt flag */
    /* Mask channel 1 higher 8 bits interrupt */
    TMMK01H = 1U;    /* disable INTTM01H interrupt */
    TMIF01H = 0U;    /* clear INTTM01H interrupt flag */
    /* Mask channel 2 interrupt */
    TMMK02 = 1U;    /* disable INTTM02 interrupt */
    TMIF02 = 0U;    /* clear INTTM02 interrupt flag */
    /* Mask channel 3 interrupt */
    TMMK03 = 1U;    /* disable INTTM03 interrupt */
    TMIF03 = 0U;    /* clear INTTM03 interrupt flag */
    /* Mask channel 3 higher 8 bits interrupt */
    TMMK03H = 1U;    /* disable INTTM03H interrupt */
    TMIF03H = 0U;    /* clear INTTM03H interrupt flag */
    /* Set INTTM00 low priority */
    TMPR100 = 1U;
    TMPR000 = 1U;
    /* Set INTTM01 low priority */
    TMPR101 = 1U;
    TMPR001 = 1U;
    /* Set INTTM02 low priority */
    TMPR102 = 1U;
    TMPR002 = 1U;
    /* Channel 0 is used as master channel for PWM output function */
    TMR00 = _0000_TAU_CLOCK_SELECT_CKM0 | _0000_TAU_CLOCK_MODE_CKS | _0000_TAU_COMBINATION_MASTER |
            _0000_TAU_TRIGGER_SOFTWARE | _0001_TAU_MODE_PWM_MASTER;
    TDR00 = _031F_TAU_TDR00_VALUE;
    TO0 &= ~_0001_TAU_CH0_OUTPUT_VALUE_1;
    TOE0 &= ~_0001_TAU_CH0_OUTPUT_ENABLE;
    /* Channel 3 is used as slave channel for PWM output function */
    TMR03 = _0000_TAU_CLOCK_SELECT_CKM0 | _0000_TAU_CLOCK_MODE_CKS | _0000_TAU_COMBINATION_SLAVE |
            _0400_TAU_TRIGGER_MASTER_INT | _0009_TAU_MODE_PWM_SLAVE;
    TDR03 = _0000_TAU_TDR03_VALUE;
    TOM0 |= _0008_TAU_CH3_OUTPUT_COMBIN;
    TOL0 &= ~_0008_TAU_CH3_OUTPUT_LEVEL_L;
    TO0 &= ~_0008_TAU_CH3_OUTPUT_VALUE_1;
    TOE0 |= _0008_TAU_CH3_OUTPUT_ENABLE;
    /* Channel 1 used as interval timer */
    TMR01 = _0000_TAU_CLOCK_SELECT_CKM0 | _0000_TAU_CLOCK_MODE_CKS | _0000_TAU_16BITS_MODE |
            _0000_TAU_TRIGGER_SOFTWARE | _0000_TAU_MODE_INTERVAL_TIMER | _0000_TAU_START_INT_UNUSED;
    TDR01 = _07CF_TAU_TDR01_VALUE;
    TOM0 &= ~_0002_TAU_CH1_OUTPUT_COMBIN;
    TOL0 &= ~_0002_TAU_CH1_OUTPUT_LEVEL_L;
    TO0 &= ~_0002_TAU_CH1_OUTPUT_VALUE_1;
    TOE0 &= ~_0002_TAU_CH1_OUTPUT_ENABLE;
    /* Channel 2 used as interval timer */
    TMR02 = _0000_TAU_CLOCK_SELECT_CKM0 | _0000_TAU_CLOCK_MODE_CKS | _0000_TAU_COMBINATION_SLAVE |
            _0000_TAU_TRIGGER_SOFTWARE | _0000_TAU_MODE_INTERVAL_TIMER | _0000_TAU_START_INT_UNUSED;
    TDR02 = _031F_TAU_TDR02_VALUE;
    TOM0 &= ~_0004_TAU_CH2_OUTPUT_COMBIN;
    TOL0 &= ~_0004_TAU_CH2_OUTPUT_LEVEL_L;
    TO0 &= ~_0004_TAU_CH2_OUTPUT_VALUE_1;
    TOE0 &= ~_0004_TAU_CH2_OUTPUT_ENABLE;
    /* Set TO03 pin */
    P3 &= 0xFDU;
    PM3 &= 0xFDU;
}

/***********************************************************************************************************************
* Function Name: R_TAU0_Channel0_Start
* Description  : This function starts TAU0 channel 0 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TAU0_Channel0_Start(void)
{
    TMIF00 = 0U;    /* clear INTTM00 interrupt flag */
    TMMK00 = 0U;    /* enable INTTM00 interrupt */
    TOE0 |= _0008_TAU_CH3_OUTPUT_ENABLE;
    TS0 |= _0001_TAU_CH0_START_TRG_ON | _0008_TAU_CH3_START_TRG_ON;
}

/***********************************************************************************************************************
* Function Name: R_TAU0_Channel0_Stop
* Description  : This function stops TAU0 channel 0 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TAU0_Channel0_Stop(void)
{
    TT0 |= _0001_TAU_CH0_STOP_TRG_ON | _0008_TAU_CH3_STOP_TRG_ON;
    TOE0 &= ~_0008_TAU_CH3_OUTPUT_ENABLE;
    /* Mask channel 0 interrupt */
    TMMK00 = 1U;    /* disable INTTM00 interrupt */
    TMIF00 = 0U;    /* clear INTTM00 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: R_TAU0_Channel1_Start
* Description  : This function starts TAU0 channel 1 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TAU0_Channel1_Start(void)
{
    TMIF01 = 0U;    /* clear INTTM01 interrupt flag */
    TMMK01 = 0U;    /* enable INTTM01 interrupt */
    TS0 |= _0002_TAU_CH1_START_TRG_ON;
}

/***********************************************************************************************************************
* Function Name: R_TAU0_Channel1_Stop
* Description  : This function stops TAU0 channel 1 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TAU0_Channel1_Stop(void)
{
    TT0 |= _0002_TAU_CH1_STOP_TRG_ON;
    /* Mask channel 1 interrupt */
    TMMK01 = 1U;    /* disable INTTM01 interrupt */
    TMIF01 = 0U;    /* clear INTTM01 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: R_TAU0_Channel2_Start
* Description  : This function starts TAU0 channel 2 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TAU0_Channel2_Start(void)
{
    TMIF02 = 0U;    /* clear INTTM02 interrupt flag */
    TMMK02 = 0U;    /* enable INTTM02 interrupt */
    TS0 |= _0004_TAU_CH2_START_TRG_ON;
}

/***********************************************************************************************************************
* Function Name: R_TAU0_Channel2_Stop
* Description  : This function stops TAU0 channel 2 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TAU0_Channel2_Stop(void)
{
    TT0 |= _0004_TAU_CH2_STOP_TRG_ON;
    /* Mask channel 2 interrupt */
    TMMK02 = 1U;    /* disable INTTM02 interrupt */
    TMIF02 = 0U;    /* clear INTTM02 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: R_TMR_RD0_Create
* Description  : This function initializes the TMRD0 module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TMR_RD0_Create(void)
{
    TRD0EN = 1U;    /* enable input clock supply */ 
    TRDSTR |= _04_TMRD_TRD0_COUNT_CONTINUES;
    TRDSTR &= (uint8_t)~_01_TMRD_TRD0_COUNT_START;  /* disable TMRD0 operation */
    TRDMK0 = 1U;    /* disable TMRD0 interrupt */
    TRDIF0 = 0U;    /* clear TMRD0 interrupt flag */
    /* Set INTTRD0 high priority */
    TRDPR10 = 0U;
    TRDPR00 = 0U;
    TRDMR |= _00_TMRD_TRDGRC0_GENERAL | _00_TMRD_TRDGRD0_GENERAL;
    TRDOER1 &= _F0_TMRD_CHANNEL0_OUTPUT_DEFAULT;
    TRDOER1 |= _00_TMRD_TRDIOA0_OUTPUT_ENABLE | _00_TMRD_TRDIOB0_OUTPUT_ENABLE;
    TRDOCR |= _00_TMRD_TRDIOA0_INITIAL_OUTPUT_L | _02_TMRD_TRDIOB0_INITIAL_OUTPUT_H;
    TRDCR0 |= _04_TMRD_INETNAL_CLOCK_F32 | _A0_TMRD_COUNTER_CLEAR_TRDGRC;
    TRDIER0 = _00_TMRD_IMIA_DISABLE | _00_TMRD_IMIB_DISABLE | _00_TMRD_IMIC_DISABLE | _08_TMRD_IMID_ENABLE |
              _00_TMRD_OVIE_DISABLE;
    TRDIORA0 = _03_TMRD_TRDGRA_COMPARE_OUTPUT_TOGGLE | _30_TMRD_TRDGRB_COMPARE_OUTPUT_TOGGLE;
    TRDIORC0 = _08_TMRD_TRDGRC_GENERAL_BUFFER_REGISTER | _80_TMRD_TRDGRD_GENERAL_BUFFER_REGISTER;
    TRDGRA0 = _0200_TMRD_TRDGRA0_VALUE;
    TRDGRB0 = _01F3_TMRD_TRDGRB0_VALUE;
    TRDGRC0 = _1387_TMRD_TRDGRC0_VALUE;
    TRDGRD0 = _1386_TMRD_TRDGRD0_VALUE;
    /* Set TRDIOA0 pin */
    POM1 &= 0x7FU;
    P1 &= 0x7FU;
    PM1 &= 0x7FU;
    /* Set TRDIOB0 pin */
    POM1 &= 0xDFU;
    P1 &= 0xDFU;
    PM1 &= 0xDFU;
}

/***********************************************************************************************************************
* Function Name: R_TMR_RD0_Start
* Description  : This function starts TMRD0 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TMR_RD0_Start(void)
{
   volatile uint8_t trdsr_dummy;
    
    TRDIF0 = 0U;    /* clear TMRD0 interrupt flag */
    trdsr_dummy = TRDSR0; /* read TRDSR0 before write 0 */
    TRDSR0 = 0x00U; /* clear TRD0 each interrupt request */
    TRDMK0 = 0U;    /* enable TMRD0 interrupt */
    TRDSTR |= _04_TMRD_TRD0_COUNT_CONTINUES;
    TRDSTR |= _01_TMRD_TRD0_COUNT_START;    /* start TMRD0 counter */
}

/***********************************************************************************************************************
* Function Name: R_TMR_RD0_Stop
* Description  : This function stops TMRD0 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TMR_RD0_Stop(void)
{
    volatile uint8_t trdsr_dummy;
    
    TRDSTR |= _04_TMRD_TRD0_COUNT_CONTINUES;
    TRDSTR &= (uint8_t)~_01_TMRD_TRD0_COUNT_START;  /* stop TMRD0 counter */
    TRDMK0 = 1U;    /* disable TMRD0 interrupt */
    TRDIF0 = 0U;    /* clear TMRD0 interrupt flag */
    trdsr_dummy = TRDSR0; /* read TRDSR0 before write 0 */
    TRDSR0 = 0x00U; /* clear TRD0 each interrupt request */
}

/***********************************************************************************************************************
* Function Name: R_TMR_RD1_Create
* Description  : This function initializes the TMRD1 module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TMR_RD1_Create(void)
{
    TRD0EN = 1U;    /* enable input clock supply */ 
    TRDSTR |= _08_TMRD_TRD1_COUNT_CONTINUES;
    TRDSTR &= (uint8_t)~_02_TMRD_TRD1_COUNT_START;  /* disable TMRD1 operation */
    TRDMK1 = 1U;    /* disable TMRD1 interrupt */
    TRDIF1 = 0U;    /* clear TMRD1 interrupt flag */
    /* Set INTTRD1 high priority */
    TRDPR11 = 0U;
    TRDPR01 = 0U;
    TRDMR |= _00_TMRD_TRDGRC1_GENERAL | _00_TMRD_TRDGRD1_GENERAL;
    TRDOER1 &= _0F_TMRD_CHANNEL1_OUTPUT_DEFAULT;
    TRDOER1 |= _00_TMRD_TRDIOA1_OUTPUT_ENABLE | _00_TMRD_TRDIOB1_OUTPUT_ENABLE;
    TRDOCR |= _00_TMRD_TRDIOA1_INITIAL_OUTPUT_L | _20_TMRD_TRDIOB1_INITIAL_OUTPUT_H;
    TRDCR1 |= _04_TMRD_INETNAL_CLOCK_F32 | _A0_TMRD_COUNTER_CLEAR_TRDGRC;
    TRDIER1 = _00_TMRD_IMIA_DISABLE | _00_TMRD_IMIB_DISABLE | _00_TMRD_IMIC_DISABLE | _08_TMRD_IMID_ENABLE |
              _00_TMRD_OVIE_DISABLE;
    TRDIORA1 = _03_TMRD_TRDGRA_COMPARE_OUTPUT_TOGGLE | _30_TMRD_TRDGRB_COMPARE_OUTPUT_TOGGLE;
    TRDIORC1 = _08_TMRD_TRDGRC_GENERAL_BUFFER_REGISTER | _80_TMRD_TRDGRD_GENERAL_BUFFER_REGISTER;
    TRDGRA1 = _0BC4_TMRD_TRDGRA1_VALUE;
    TRDGRB1 = _0BB7_TMRD_TRDGRB1_VALUE;
    TRDGRC1 = _1387_TMRD_TRDGRC1_VALUE;
    TRDGRD1 = _1386_TMRD_TRDGRD1_VALUE;
    /* Set TRDIOA1 pin */
    POM1 &= 0xF7U;
    P1 &= 0xF7U;
    PM1 &= 0xF7U;
    /* Set TRDIOB1 pin */
    P1 &= 0xFBU;
    PM1 &= 0xFBU;
}

/***********************************************************************************************************************
* Function Name: R_TMR_RD1_Start
* Description  : This function starts TMRD1 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TMR_RD1_Start(void)
{
   volatile uint8_t trdsr_dummy;
    
    TRDIF1 = 0U;    /* clear TMRD1 interrupt flag */
    trdsr_dummy = TRDSR1; /* read TRDSR1 before write 0 */
    TRDSR1 = 0x00U; /* clear TRD1 each interrupt request */
    TRDMK1 = 0U;    /* enable TMRD1 interrupt */
    TRDSTR |= _08_TMRD_TRD1_COUNT_CONTINUES;
    TRDSTR |= _02_TMRD_TRD1_COUNT_START;    /* start TMRD1 counter */
}

/***********************************************************************************************************************
* Function Name: R_TMR_RD1_Stop
* Description  : This function stops TMRD1 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_TMR_RD1_Stop(void)
{
    volatile uint8_t trdsr_dummy;
    
    TRDSTR |= _08_TMRD_TRD1_COUNT_CONTINUES;
    TRDSTR &= (uint8_t)~_02_TMRD_TRD1_COUNT_START;  /* stop TMRD1 counter */
    TRDMK1 = 1U;    /* disable TMRD1 interrupt */
    TRDIF1 = 0U;    /* clear TMRD1 interrupt flag */
    trdsr_dummy = TRDSR1; /* read TRDSR1 before write 0 */
    TRDSR1 = 0x00U; /* clear TRD1 each interrupt request */
}

/* Start user code for adding. Do not edit comment generated here */
void R_TMR_RD0_Create1(void)
{
    TRD0EN = 1U;    /* enable input clock supply */
    TRDSTR |= _04_TMRD_TRD0_COUNT_CONTINUES;
    TRDSTR &= (uint8_t)~_01_TMRD_TRD0_COUNT_START;  /* disable TMRD0 operation */
    TRDMK0 = 1U;    /* disable TMRD0 interrupt */
    TRDIF0 = 0U;    /* clear TMRD0 interrupt flag */
    /* Set INTTRD0 high priority */
    TRDPR10 = 0U;
    TRDPR00 = 0U;
    TRDOER1 &= _F0_TMRD_CHANNEL0_OUTPUT_DEFAULT;
    TRDOER1 |= _00_TMRD_TRDIOA0_OUTPUT_ENABLE | _00_TMRD_TRDIOB0_OUTPUT_ENABLE;
    TRDOCR = 0;
}
void R_TMR_RD1_Create1(void)
{
    TRD0EN = 1U;    /* enable input clock supply */
    TRDSTR |= _08_TMRD_TRD1_COUNT_CONTINUES;
    TRDSTR &= (uint8_t)~_02_TMRD_TRD1_COUNT_START;  /* disable TMRD1 operation */
    TRDMK1 = 1U;    /* disable TMRD1 interrupt */
    TRDIF1 = 0U;    /* clear TMRD1 interrupt flag */
    /* Set INTTRD1 high priority */
    TRDPR11 = 0U;
    TRDPR01 = 0U;
    TRDOER1 &= _0F_TMRD_CHANNEL1_OUTPUT_DEFAULT;
    TRDOER1 |= _00_TMRD_TRDIOA1_OUTPUT_ENABLE | _00_TMRD_TRDIOB1_OUTPUT_ENABLE;
    TRDOCR = 0;
}

/* End user code. Do not edit comment generated here */
