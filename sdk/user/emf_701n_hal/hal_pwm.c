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

/************************************************************************************************************
**	Description:
************************************************************************************************************/
#include "hw_config.h"
#ifdef HW_PWM_MAP

#include  "api/api_system.h"
#include  "api/api_pwm.h"
#include  "api/api_gpio.h"

#include "asm/mcpwm.h"
#include "asm/clock.h"
#include "system/includes.h"

#include  "api/api_log.h"


/******************************************************************************************************
** Defined
*******************************************************************************************************/
/*
AC695 芯片定义:
如果非硬件引脚pwm_ch必须小于3
如果是硬件引脚pwm_ch必须如下对应,可以通过PWM_CH_H选项选择不同通道
     mcpwm硬件引脚，上下为一对：---CH0---     ---CH1---    ---CH2---    ---CH3---    ---CH4---    ---CH5--- 
// static u8 pwm_hw_h_pin[6] = {IO_PORTA_00, IO_PORTB_00, IO_PORTB_04, IO_PORTB_09, IO_PORTA_09, IO_PORTC_04};
// static u8 pwm_hw_l_pin[6] = {IO_PORTA_01, IO_PORTB_02, IO_PORTB_06, IO_PORTB_10, IO_PORTA_10, IO_PORTC_05};
#define HW_PWM_MAP {	\
			{PA_01,   pwm_timer0,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch0|PWM_CH_H)|VAL2FLD(PWM_ACTIVE,1)},\
			{PB_00,   pwm_timer1,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1),VAL2FLD(PWM_ACTIVE,1)},\
			{PB_02,   pwm_timer1,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1|PWM_CH_H)},\
			{PB_06,   pwm_timer2,   (PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch2),VAL2FLD(PWM_ACTIVE,1)},\
			}

sdk AC6321/701n 芯片定义:
#define HW_PWM_MAP {	\
			{Pn_XX,   NULL, VAL2FLD(PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch0|PWM_CH_H)},\
			{Pn_XX,   NULL, VAL2FLD(PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1)},\
			{Pn_XX,   NULL, VAL2FLD(PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch1|PWM_CH_H)|VAL2FLD(PWM_ACTIVE,1)},\
			{Pn_XX,   NULL, VAL2FLD(PWM_FREQ,10000)|VAL2FLD(PWM_CH,pwm_ch2)|VAL2FLD(PWM_ACTIVE,1)},\
			}   
*/






/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/

/*****************************************************************************************************
**	static Function
******************************************************************************************************/
/**
 * @brief 单独设置PWM H或L pin的duty
 * 
 * @param pwm_ch 
 * @param timer_ch : BD19/BR28 使用 pwm_ch
 * @param is_h 1:H, 0:L pin
 * @param duty 
 */
void mcpwm_set_hl_duty(pwm_ch_num_type pwm_ch, uint8_t timer_ch, uint8_t is_h, u16 duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(timer_ch);     //bd19 使用 pwm_ch
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);

    if (pwm_reg && timer_reg) {
        if(is_h){
            pwm_reg->ch_cmph = timer_reg->tmr_pr * duty / 10000;
        }else{
            pwm_reg->ch_cmpl = timer_reg->tmr_pr * duty / 10000;
        }
        timer_reg->tmr_cnt = 0;
        timer_reg->tmr_con |= 0b01;
    }
}
void mcpwm_set_duty(pwm_ch_num_type pwm_ch, u16 duty)
{
    PWM_TIMER_REG *timer_reg = get_pwm_timer_reg(pwm_ch);
    PWM_CH_REG *pwm_reg = get_pwm_ch_reg(pwm_ch);

    if (pwm_reg && timer_reg) {
        pwm_reg->ch_cmpl = timer_reg->tmr_pr * duty / 10000;
        pwm_reg->ch_cmph = pwm_reg->ch_cmpl;
        timer_reg->tmr_cnt = 0;
        timer_reg->tmr_con |= 0b01;

        if (duty == 10000) {
            timer_reg->tmr_cnt = 0;
            timer_reg->tmr_con &= ~(0b11);
        } else if (duty == 0) {
            timer_reg->tmr_cnt = pwm_reg->ch_cmpl;
            timer_reg->tmr_con &= ~(0b11);
        }
    }
}


/*****************************************************************************************************
**  Function
******************************************************************************************************/


/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
bool hal_pwm_set_duty(uint16_t id, uint8_t duty)
{
    #if defined CONFIG_CPU_BD19 || defined CONFIG_CPU_BR28
    uint8_t timer_ch = PWM_CH_ATT(id);
    #elif defined CONFIG_CPU_BR23
    uint8_t timer_ch = m_pwm_map[id].peripheral;
    #endif

    if(PWM_CH_ATT(id) & PWM_CH_H){
        mcpwm_set_hl_duty(PWM_CH_ATT(id), timer_ch, 1, duty*10000UL/255UL);
    }else{ 
        mcpwm_set_hl_duty(PWM_CH_ATT(id), timer_ch, 0, duty*10000UL/255UL);
    }
	return true;
}

bool hal_pwm_init(uint16_t id, uint8_t duty)
{
    struct pwm_platform_data pwm_p_data = {0};

    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         // 边沿对齐
    pwm_p_data.frequency = PWM_FREQ_ATT(id);                // 频率
    pwm_p_data.duty = duty*10000UL/255UL;                   // 占空比
    pwm_p_data.pwm_ch_num = PWM_CH_ATT(id);                 // 外设通道, pwm_ch0, pwm_ch1...
    #if defined CONFIG_CPU_BR23
    pwm_p_data.pwm_timer_num = m_pwm_map[id].peripheral;    // 时基选择,  timer1, timer2...
    #endif

    if(PWM_CH_ATT(id) & PWM_CH_H){
        #if defined CONFIG_CPU_BR23
        pwm_p_data.h_pin_output_ch_num = 2;
        #endif
        pwm_p_data.h_pin = m_pwm_map[id].pin;                   // PWM 引脚
        pwm_p_data.l_pin = -1;                                  // PWM 引脚
    }else{
        #if defined CONFIG_CPU_BR23
        pwm_p_data.l_pin_output_ch_num = 2;
        #endif
        pwm_p_data.l_pin = m_pwm_map[id].pin;
        pwm_p_data.h_pin = -1;    
    }
    pwm_p_data.complementary_en = 0;                        // 两个引脚的波形, 1: 互补, 0: 同步;

    mcpwm_init(&pwm_p_data);
	return true;
}
bool hal_pwm_deinit(uint16_t id)
{
    // 传通道
    #if defined CONFIG_CPU_BD19 || defined CONFIG_CPU_BR28
        mcpwm_close(PWM_CH_ATT(id));
    #elif defined CONFIG_CPU_BR23
        mcpwm_close(PWM_CH_ATT(id), m_pwm_map[id].peripheral);
    #endif
    return true;
}



#endif





