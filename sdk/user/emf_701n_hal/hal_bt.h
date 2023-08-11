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

#ifndef	_hal_bt_h
#define	_hal_bt_h
#include "hw_config.h"
#include "system/includes.h"
#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************************************************
**	Hardware  Defined
********************************************************************************************************************/
#define CFG_RF_24G_CODE_ID  	(0x28)
/*******************************************************************************************************************
**	Parameters
********************************************************************************************************************/

/*******************************************************************************************************************
**	event
********************************************************************************************************************/


/*******************************************************************************************************************
**	Functions
********************************************************************************************************************/
extern int sys_bt_event_handler(struct sys_event *event);

#ifdef __cplusplus
}
#endif
#endif

