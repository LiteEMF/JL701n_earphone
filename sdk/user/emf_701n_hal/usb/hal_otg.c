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
#if API_OTG_BIT_ENABLE
#include "api/usb/usb_typedef.h"
#include "api/usb/api_otg.h"

#include "system/includes.h"
#include "server/server_core.h"
#include "os/os_api.h"


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
bool otg_regist = false;		//多usb只注册一次
/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*******************************************************************
** Parameters:		
** Returns:	
** Description:	usb event handler	
*******************************************************************/
//用于特殊项目中使用USB 识别充电和接电脑
__WEAK bool usbd_event_vendor(struct sys_event *event)
{
    return false;
}
static void usb_event_handler(struct sys_event *event)
{
    const char *usb_msg;
    uint8_t usb_id;

    switch ((u32)event->arg) {
    case DEVICE_EVENT_FROM_OTG:
        usb_msg = (const char *)event->u.dev.value;
        usb_id = usb_msg[2] - '0';

        logd("usb event : %d DEVICE_EVENT_FROM_OTG %s\n",event->u.dev.event, usb_msg);

        if (usb_msg[0] == 'h') {
            if (event->u.dev.event == DEVICE_EVENT_IN) {
				#if API_USBH_BIT_ENABLE
                usbh_init(usb_id<<4);
				usbh_det_event(usb_id<<4, 1);
				#endif

            } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
				#if API_USBH_BIT_ENABLE
				usbh_det_event(usb_id<<4, 0);
                usbh_deinit(usb_id<<4);
				#endif
            }
        } else if (usb_msg[0] == 's') {
            if(usbd_event_vendor(event)){
                break;
            }
			#if API_USBD_BIT_ENABLE
            if (event->u.dev.event == DEVICE_EVENT_IN) {
				usbd_init(usb_id);
            } else {
				usbd_deinit(usb_id);
            }
			#endif
        }
        break;
    case DEVICE_EVENT_FROM_USB_HOST:
        logd("host_event %x\n", event->u.dev.event);
        if ((event->u.dev.event == DEVICE_EVENT_IN) ||
            (event->u.dev.event == DEVICE_EVENT_CHANGE)) {
             logd("device in %x\n", event->u.dev.value);
        } else if (event->u.dev.event == DEVICE_EVENT_OUT) {
            logd("device out %x\n", event->u.dev.value);
        }
        break;
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
error_t hal_otg_init(uint8_t id)
{
	if(!otg_regist){
		otg_regist = true;
        logd("register_sys_event_handler\n");
		register_sys_event_handler(SYS_DEVICE_EVENT, 0, 3, usb_event_handler);
	}
	return ERROR_SUCCESS;
}
error_t hal_otg_deinit(uint8_t id)
{
	if(otg_regist){
		otg_regist = false;
        logd("unregister_sys_event_handler\n");
		unregister_sys_event_handler(usb_event_handler);
	}
	return ERROR_SUCCESS;
}

#endif



