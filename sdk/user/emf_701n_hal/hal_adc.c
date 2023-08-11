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
#ifdef HW_ADC_MAP
#include  "api/api_adc.h"

#include "asm/adc_api.h"
/******************************************************************************************************
** Defined
*******************************************************************************************************/
// #define HW_ADC_MAP {	\
// 			{PA_00,0,VAL2FLD(ADC_CH,0)},								\
// 			{PA_01,0,VAL2FLD(ADC_CH,1) | VAL2FLD(ADC_PULL,1)}			\
// 			}
/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/

/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/

/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
uint16_t hal_adc_to_voltage(uint16_t adc)
{
    u32 adc_vbg = adc_get_value(AD_CH_LDOREF);
    if (adc_vbg == 0) {
        return 0;
    }
    return adc_value_to_voltage(adc_vbg, adc);
}
bool hal_adc_value(uint8_t id, uint16_t* valp)
{
	*valp = adc_get_value(ADC_CH_ATT(id));
	return true;
}
bool hal_adc_start_scan(void)
{
	return true;
}

bool hal_adc_init(void)
{
	uint8_t id;
	
	for(id=0; id<m_adc_num; id++){
		gpio_set_die(m_adc_map[id].pin, 0);
		gpio_set_direction(m_adc_map[id].pin, 1);
		gpio_set_pull_down(m_adc_map[id].pin, 0);
		if(ADC_PULL_ATT(id)){
			gpio_set_pull_up(m_adc_map[id].pin, 0);
		}else{
			gpio_set_pull_up(m_adc_map[id].pin, 1);
		}

		adc_add_sample_ch(ADC_CH_ATT(id));
	}
	return true;
}

bool hal_adc_deinit(void)
{
	extern void adc_close();
	adc_close();
	return true;
}


#endif




