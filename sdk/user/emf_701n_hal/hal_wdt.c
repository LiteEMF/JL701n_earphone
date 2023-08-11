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
#if API_WDT_ENABLE

#include  "api/api_wdt.h"
#include "asm/wdt.h"
/******************************************************************************************************
** Defined
*******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/

/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
//hal

void hal_wdt_feed(void)
{
	wdt_clear();
}

bool hal_wdt_init(uint32_t ms)
{
	if(1000 >= ms){
		wdt_init(WDT_1S);
	}else if(2000 >= ms){
		wdt_init(WDT_2S);
	}else if(4000 >= ms){
		wdt_init(WDT_4S);
	}else if(8000 >= ms){
		wdt_init(WDT_8S);
	}else if(16000 >= ms){
		wdt_init(WDT_16S);
	}else{
		wdt_init(WDT_32S);
	}
	wdt_enable();
	return true;
}
bool hal_wdt_deinit(void)
{
	wdt_close();
	return true;
}


#endif







