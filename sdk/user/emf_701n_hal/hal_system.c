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
#include  "api/api_system.h"
#include  "asm/clock.h"
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
bool hal_set_sysclk(emf_clk_t clk, uint32_t freq)
{
	switch(clk){
	case SYSCLK:
		clk_set("sys",freq);
		break;
	default:
		clk_set("lsb",freq);
		break;
	}
	return true;
}
/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
uint32_t hal_get_sysclk(emf_clk_t clk)
{
	uint32_t freq;

	switch(clk){
	case SYSCLK:
		freq = clk_get("sys");
		break;
	default:
		freq = clk_get("lsb");
		break;
	}
	return freq;
}

bool hal_get_uuid(uint8_t *uuid, uint8_t len)
{
	uint8_t buf[16];

	extern __attribute__((weak)) u8 *get_norflash_uuid(void);
    uint8_t *p = get_norflash_uuid();
    logd("flash_uuid:");
    dumpd(p, 16);

	//由于杰理有8和16byte的UUID，为了确保兼容，这里直接把两者混加
	memcpy(buf, p, 16);
    for(int i=0; i<8; i++){
        buf[i] += *(p+8+i);
    }

    memcpy(uuid, buf, MIN(len, 16));

	return true;
}






