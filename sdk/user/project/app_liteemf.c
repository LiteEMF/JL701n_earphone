/*********************************************************************************************
    *   Filename        : app_keyboard.c

    *   Description     :

    *   Author          :

    *   Email           :

    *   Last modifiled  : 2019-07-05 10:09

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "system/includes.h"
#include "server/server_core.h"
#include "app_action.h"
#include "app_config.h"

#if(CONFIG_APP_LITEEMF || 1)
#include "system/app_core.h"
#include "os/os_api.h"
#include "rcsp_bluetooth.h"
#include "update_loader_download.h"
#include "app/emf.h"
#include "api/api_log.h"


/******************************************************************************************************
** Defined
*******************************************************************************************************/
// #define    LITEEMF_APP           /*使用LITEEMFP APP*/


#define     TASK_NAME               "emf_task"
#define     HEARTBEAT_EVENT         0x01000000  
#define     UART_RX_EVENT           0x02000000       //低24bit 作为参数
#define     BT_REC_DATA_EVENT       0x03000000       //bt_t len

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/
static volatile u8 is_app_liteemf_active = 0;//1-临界点,系统不允许进入低功耗，0-系统可以进入低功耗

uint8_t heartbeat_msg_cnt = 1;
/*****************************************************************************************************
**  Function
******************************************************************************************************/
void api_timer_hook(uint8_t id)
{
    if(0 == id){
        m_systick++;

        if(0 == heartbeat_msg_cnt){
            int err = os_taskq_post_msg(TASK_NAME, 1, HEARTBEAT_EVENT);
            if(err){
                logd("heatbeat post err!!!\n");
            }else{
                heartbeat_msg_cnt = 1;
            }
        }
        
    }
}





extern u32 get_jl_rcsp_update_status();
static u32 check_ota_mode()
{
    if (UPDATE_MODULE_IS_SUPPORT(UPDATE_APP_EN)) {
        #if RCSP_UPDATE_EN
        if (get_jl_rcsp_update_status()) {
            r_printf("OTA ing");
            set_run_mode(OTA_MODE);
            usb_sie_close_all();//关闭usb
            JL_UART1->CON0 = BIT(13) | BIT(12) | BIT(10);//关闭串口
            return 1;
        }
        #endif
    }
    return 0;
}
/*******************************************************************
** Parameters:		
** Returns:	
** Description:	task	
*******************************************************************/
static void emf_task_handle(void *arg)
{
    uint8_t *p;
    int ret = 0;
    int msg[16];

    while (1) {
        ret = os_taskq_pend("taskq", msg, ARRAY_SIZE(msg));
        if (ret != OS_TASKQ) {
            continue;
        }
        if (msg[0] != Q_MSG) {
            continue;
        }

        switch (msg[1] & 0XFF000000) {
            case HEARTBEAT_EVENT:
                heartbeat_msg_cnt = 0;
                m_task_tick10us +=100;      //同步任务tick时钟
                emf_handler(0);

                #if API_USBD_BIT_ENABLE     //无线升级时候关闭usb
                if (check_ota_mode()) {
                    usb_stop(0);
                }
                #endif
                break;
            case UART_RX_EVENT:
                break;
            case BT_REC_DATA_EVENT:
                break;
            default:
                break;
        }
    }
}




/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
extern void bt_pll_para(u32 osc, u32 sys, u8 low_power, u8 xosc);
void liteemf_app_start(void)
{
    logi("=======================================");
    logi("-------------LITEMEF DEMO-----------------");
    logi("=======================================");
    logi("app_file: %s", __FILE__);

    // clk_set("sys", 96 * 1000000L);		//fix clk
    // clk_set("sys", BT_NORMAL_HZ);

    emf_api_init();
    emf_init();

    logd("start end\n");
    os_task_create(emf_task_handle,NULL,2,2048,512,"emf_task");
    heartbeat_msg_cnt = 0;

    #if BT_ENABLE
    api_bt_init();
    #endif
}

#ifdef LITEEMF_APP

/*******************************************************************
** Parameters:		
** Returns:	
** Description:	app  状态处理	
*******************************************************************/
static int liteemf_state_machine(struct application *app, enum app_state state, struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        break;
    case APP_STA_START:
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_LITEEMF_MAIN:
            liteemf_app_start();
            break;
        }
        break;
    case APP_STA_PAUSE:
        break;
    case APP_STA_RESUME:
        break;
    case APP_STA_STOP:
        break;
    case APP_STA_DESTROY:
        logi("APP_STA_DESTROY\n");
        break;
    }

    return 0;
}
/*******************************************************************
** Parameters:		
** Returns:	
** Description:	app 线程事件处理
*******************************************************************/
static int liteemf_event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_BT_EVENT:
        #if API_BT_ENABLE
        sys_bt_event_handler(event);
        #endif
        return 0;

    case SYS_DEVICE_EVENT:
        if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            return app_power_event_handler(&event->u.dev, NULL);    //set_soft_poweroff_call 可以传入关机函数使用系统电池检测
        }
        #if TCFG_CHARGE_ENABLE
        else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
            app_charge_event_handler(&event->u.dev);
        }
        #endif
        return 0;

    default:
        return 0;
    }

    return 0;
}
/*******************************************************************
** Parameters:		
** Returns:	
** Description:	注册控制是否进入sleep
*******************************************************************/
//system check go sleep is ok
static u8 liteemf_app_idle_query(void)
{
    return !is_app_liteemf_active;
}
REGISTER_LP_TARGET(app_liteemf_lp_target) = {
    .name = "app_liteemf_deal",
    .is_idle = liteemf_app_idle_query,
};


static const struct application_operation app_liteemf_ops = {
    .state_machine  = liteemf_state_machine,
    .event_handler 	= liteemf_event_handler,
};

/*
 * 注册模式
 */
REGISTER_APPLICATION(app_liteemf) = {
    .name 	= "app_liteemf",
    .action	= ACTION_LITEEMF_MAIN,
    .ops 	= &app_liteemf_ops,
    .state  = APP_STA_DESTROY,
};
#endif

#endif

