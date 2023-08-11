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
#include "hw_board.h"
#if	defined HW_TIMER_MAP && API_TIMER_BIT_ENABLE
#include  "api/api_timer.h"
#include  "api/api_tick.h"

#include "system/includes.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/
// #define HW_TIMER_MAP	{0, VAL2FLD(TIMER_FREQ,1000)|VAL2FLD(TIMER_PRI,1)}
/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/



/*****************************************************************************************************
**	static Function
******************************************************************************************************/

#define TIMER_SFR(ch) JL_TIMER##ch

/*
 *timer
 * */
#if defined CONFIG_CPU_BR23
#define _timer_init(ch,priority,us)                             \
    request_irq(IRQ_TIME##ch##_IDX, priority, timer##ch##_isr, 0);        \
    TIMER_SFR(ch)->CNT = 0;									    \
    TIMER_SFR(ch)->PWM = 0;                                     \             
	TIMER_SFR(ch)->PRD = us*24UL/4;                             \
    TIMER_SFR(ch)->CON = (0b10 << 2);/*选择晶振时钟源：24MHz*/   \
    TIMER_SFR(ch)->CON |= (0b0001 << 4); /*时钟源再4分频*/        \
	TIMER_SFR(ch)->CON |= BIT(0);

#elif defined CONFIG_CPU_BD19 || defined CONFIG_CPU_BR28
#define _timer_init(ch,priority,us)                             \
    request_irq(IRQ_TIME##ch##_IDX, priority, timer##ch##_isr, 0);        \
    TIMER_SFR(ch)->CNT = 0;									    \
    TIMER_SFR(ch)->PWM = 0;                                     \    
    TIMER_SFR(ch)->CON  = (0b110 << 10);					/*时钟源选择STD_24M时钟源*/\
    TIMER_SFR(ch)->CON |= (0b0001 << 4);					/*时钟源再4分频*/\
	TIMER_SFR(ch)->PRD = us*(24/4); /*TCFG_CLOCK_SYS_SRC选择晶振时钟源：24MHz*/         \
	TIMER_SFR(ch)->CON |= BIT(0);
#endif


uint8_t get_timer_id (uint8_t timer)
{
    uint8_t i;

    for(i=0; i<m_timer_num; i++){
        if(m_timer_map[i].peripheral == (uint32_t)timer){
            return i;
        }
    }
    return ID_NULL;
}

#if API_TIMER_BIT_ENABLE & BIT(0)
___interrupt
static void timer0_isr(void)
{
    uint8_t id = get_timer_id(0);   
    TIMER_SFR(0)->CON |= BIT(14);
    api_timer_hook(id);  
}
#endif

#if API_TIMER_BIT_ENABLE & BIT(1)
___interrupt
static void timer1_isr(void)
{
    uint8_t id = get_timer_id(1);   
    TIMER_SFR(1)->CON |= BIT(14);
    api_timer_hook(id);  
}
#endif

#if API_TIMER_BIT_ENABLE & BIT(2)
___interrupt
static void timer2_isr(void)
{
    uint8_t id = get_timer_id(2);   
    TIMER_SFR(2)->CON |= BIT(14);
    api_timer_hook(id);  
}
#endif
#if API_TIMER_BIT_ENABLE & BIT(3)
___interrupt
static void timer3_isr(void)
{
    uint8_t id = get_timer_id(3);   
    TIMER_SFR(3)->CON |= BIT(14);
    api_timer_hook(id);  
}
#endif
#if API_TIMER_BIT_ENABLE & BIT(4)
___interrupt
static void timer4_isr(void)
{
    uint8_t id = get_timer_id(4);   
    TIMER_SFR(4)->CON |= BIT(14);
    api_timer_hook(id);  
}
#endif
#if API_TIMER_BIT_ENABLE & BIT(5)
___interrupt
static void timer5_isr(void)
{
    uint8_t id = get_timer_id(5);   
    TIMER_SFR(5)->CON |= BIT(14);
    api_timer_hook(id);  
}
#endif

static void timer_init(u8 timer, u32 us)
{
    switch (timer) {
    case 0:
		#if API_TIMER_BIT_ENABLE & BIT(0)
        _timer_init(0, TIMER_PRI_ATT(0), us);
		#endif
        break;
    case 1:
		#if API_TIMER_BIT_ENABLE & BIT(1)
        _timer_init(1, TIMER_PRI_ATT(1), us);
		#endif
        break;
    case 2:
		#if API_TIMER_BIT_ENABLE & BIT(2)
        _timer_init(2, TIMER_PRI_ATT(2), us);
		#endif
        break;
    case 3:
		#if API_TIMER_BIT_ENABLE & BIT(3)
        _timer_init(3, TIMER_PRI_ATT(3), us);
		#endif
        break;
    case 4:
		#if API_TIMER_BIT_ENABLE & BIT(4)
        _timer_init(4, TIMER_PRI_ATT(4), us);
		#endif
        break;
    case 5:
		#if API_TIMER_BIT_ENABLE & BIT(5)
        _timer_init(5, TIMER_PRI_ATT(5), us);
		#endif
        break;
    default:
        break;
    }
}


/*****************************************************************************************************
**  Function
******************************************************************************************************/

/*******************************************************************
** Parameters:		
** Returns:	
** Description:	isr default on
*******************************************************************/
bool hal_timer_init(uint8_t id)
{
	uint32_t timer = m_timer_map[id].peripheral;
	timer_init(timer, TIMER_FREQ_ATT(id));
	return true;
}
bool hal_timer_deinit(uint8_t id)
{
	switch (id) {
    case 0:
		TIMER_SFR(0)->CON = 0;
        break;
    case 1:
		TIMER_SFR(1)->CON = 0;
        break;
    case 2:
		TIMER_SFR(2)->CON = 0;
        break;
    case 3:
		TIMER_SFR(3)->CON = 0;
        break;
    case 4:
        #if API_TIMER_BIT_ENABLE & BIT(4)
		TIMER_SFR(4)->CON = 0;
        #endif
        break;
    case 5:
        #if API_TIMER_BIT_ENABLE & BIT(5)
		TIMER_SFR(5)->CON = 0;
        #endif
        break;
    default:
        break;
    }

	return true;
}

#endif






