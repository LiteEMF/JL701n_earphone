/* 
*   BSD 2-Clause License
*   Copyright (c) 2022, LiteEMF
*   All rights reserved.
*   This software component is licensed by LiteEMF under BSD 2-Clause license,
*   the "License"; You may not use this file except in compliance with the
*   License. You may obtain a copy of the License at:
*   opensource.org/licenses/BSD-2-Clause
*/


#ifndef _hal_typedef_h
#define _hal_typedef_h

#ifdef __cplusplus
extern "C" {
#endif

#include "typedef.h"
#include "generic/list.h"

#define __STDBOOL_H         //杰里不调用stdbool
#define	_STDIO_H_			//杰里不调用stdbio

typedef struct list_head list_head_t;
#define list_for_each_entry_type(pos, head, type, member) 	list_for_each_entry(pos, head, member) 
#define list_for_each_entry_reverse_type(pos, head, type, member)	list_for_each_entry_reverse(pos, head, member)
#define list_prepare_entry_type(pos, head, type, member)	ist_prepare_entry(pos, head, member)
#define list_for_each_entry_continue_type(pos, head, type,member)	list_for_each_entry_continue(pos, head, member)
#define list_for_each_entry_continue_reverse_type(pos, head, type, member) list_for_each_entry_continue_reverse(pos, head,member)
#define list_for_each_entry_from_type(pos, head, type, member) list_for_each_entry_from(pos, head, member)
#define list_for_each_entry_safe_type(pos, n, head, type, member) list_for_each_entry_safe(pos, n, head, member)
#define list_for_each_entry_safe_continue_type(pos, n, head, type, member) list_for_each_entry_safe_continue(pos, n, head, member)
#define list_for_each_entry_safe_from_type(pos, n, head, type, member) list_for_each_entry_safe_from(pos, n, head, member)
#define list_for_each_entry_safe_from_type(pos, n, head, type, member) list_for_each_entry_safe_from(pos, n, head, member)




#ifdef __cplusplus
}
#endif
#endif





