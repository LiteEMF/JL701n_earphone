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
#if API_AUDIO_ENABLE
#include "api/audio/api_audio.h"


#include "api/api_log.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/


/******************************************************************************************************
**	static Parameters
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
void hal_audio_spk_set_vol(uint8_t id,uint16_t l_vol, uint16_t r_vol)
{
}
void hal_audio_mic_set_vol(uint8_t id,uint16_t mic_vol)
{
}
void hal_audio_open_spk(uint8_t id,api_audio_t *paudio)
{
}
void hal_audio_close_spk(uint8_t id)
{
}
void hal_audio_open_mic(uint8_t id,api_audio_t *paudio)
{
}
void hal_audio_close_mic(uint8_t id)
{
}
bool hal_audio_init(uint8_t id,api_audio_t *paudio)
{
	return true;
}

#endif






