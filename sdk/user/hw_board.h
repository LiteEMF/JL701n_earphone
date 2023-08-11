/* 
*   BSD 2-Clause License
*   Copyright (c) 2022, LiteEMF
*   All rights reserved.
*   This software component is licensed by LiteEMF under BSD 2-Clause license,
*   the "License"; You may not use this file except in compliance with the
*   License. You may obtain a copy of the License at:
*       opensource.org/licenses/BSD-2-Clause
* 
*/


#ifndef _hw_board_h
#define _hw_board_h
#include "utils/emf_defined.h"

#ifdef __cplusplus
extern "C" {
#endif



/******************************************************************************************************
** Defined
*******************************************************************************************************/
#if PROJECT_KM

#elif PROJECT_GAMEPAD
	#if GAMEPAD1
		// timer
		#define API_TIMER_BIT_ENABLE 	BIT(2)
		#define HW_TIMER_MAP {\
			{2, VAL2FLD(TIMER_FREQ,1000)|VAL2FLD(TIMER_PRI,1)},	}
			
		// uart
		#define TCFG_UART0_TX_PORT  				IO_PORTA_05
		#define HW_UART_MAP {\
			{PA_00, PIN_NULL, 0, 0, 0, VAL2FLD(UART_BAUD,1000000)}}

		//adc
		// #define HW_ADC_MAP {	\
			{PA_00,0,VAL2FLD(ADC_CH,0)},								\
			{PA_01,0,VAL2FLD(ADC_CH,1) | VAL2FLD(ADC_PULL,1)}			\
			}

		// iic
		// #define HW_IIC_MAP {	\
			{PA_08,PA_09,PB_02,0,VAL2FLD(IIC_BADU,400000)},	\
			}
		// spi
		// #define HW_SPI_HOST_MAP {\
			{PB_07,PB_05,PB_06,PB_04,SPI1,VAL2FLD(SPI_BADU,1000)},	\
			}

		// pwm			
		// #define HW_PWM_MAP {	\
			{PA_01,   pwm_timer0,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch0|PWM_CH_H)|VAL2FLD(PWM_ACTIVE,1)},\
			{PB_00,   pwm_timer1,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1),VAL2FLD(PWM_ACTIVE,1)},\
			{PB_02,   pwm_timer1,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1|PWM_CH_H)},\
			{PB_06,   pwm_timer2,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch2),VAL2FLD(PWM_ACTIVE,1)},\
			}
	#endif
#endif




#ifdef __cplusplus
}
#endif
#endif





