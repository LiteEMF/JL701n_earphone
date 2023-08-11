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


#ifndef _hal_usbd_h
#define _hal_usbd_h
#include "emf_typedef.h" 

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************************************
** Defined
*******************************************************************************************************/

#if defined CONFIG_CPU_BD19
#ifndef USBD_NUM
#define USBD_NUM				2
#endif		

#else

#ifndef USBD_NUM
#define USBD_NUM				1
#endif		
#endif


/******************************************************************************************************
**	Parameters
*******************************************************************************************************/



/*****************************************************************************************************
**  Function
******************************************************************************************************/

#ifdef __cplusplus
}
#endif
#endif





