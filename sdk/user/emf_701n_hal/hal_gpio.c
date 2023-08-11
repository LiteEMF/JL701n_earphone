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
#include  "api/api_gpio.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/

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
void hal_gpio_mode(pin_t pin, uint8_t mode)
{
}
void hal_gpio_dir(pin_t pin, pin_dir_t dir, pin_pull_t pull)
{
    gpio_set_direction(pin,dir);
    gpio_set_die(pin, 1);		//设置普通输入
    gpio_set_dieh(pin, 1);

	if(PIN_OUT == dir){
		gpio_set_hd(pin, 1);//增强输出
		gpio_set_pull_down(pin, 0);
		gpio_set_pull_up(pin, 0);
	}else{
		if(PIN_PULLDOWN == pull){	//端口下拉
			gpio_set_pull_down(pin, 1);
			gpio_set_pull_up(pin, 0);
		}else if(PIN_PULLUP == pull){	//端口上拉
			gpio_set_pull_down(pin, 0);
			gpio_set_pull_up(pin, 1);
		}else{
			gpio_set_pull_down(pin, 0);
			gpio_set_pull_up(pin, 0);
		}
	}
}
uint32_t hal_gpio_in(pin_t pin)
{
	return gpio_read(pin);
}
void hal_gpio_out(pin_t pin, uint8_t value)
{
	gpio_write(pin,value);
}





