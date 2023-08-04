#include "system/includes.h"
#include "app_action.h"
#include "key_event_deal.h"
#include "audio_config.h"
#include "bt_background.h"
#include "default_event_handler.h"
#include "tone_player.h"
#include "user_cfg.h"
#include "app_task.h"
#include "earphone.h"

#define LOG_TAG_CONST       AUX
#define LOG_TAG             "[AUX]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
#include "log.h"

#if TCFG_APP_AUX_EN

#define POWER_OFF_CNT       6

struct app_aux_var {
    u8 inited;
    u8 idle;
};

static u8 goto_poweroff_cnt = 0;
static u8 goto_poweroff_flag = 0;
static struct app_aux_var g_aux;

static const u8 aux_key_table[][KEY_EVENT_MAX] = {
    // SHORT           LONG              HOLD
    // UP              DOUBLE            TRIPLE
    {
        KEY_MODE_SWITCH, KEY_POWEROFF,      KEY_POWEROFF_HOLD,
        KEY_NULL,       KEY_ANC_SWITCH.          KEY_NULL
    },
    {
        KEY_NULL,       KEY_NULL,          KEY_NULL,
        KEY_NULL,       KEY_NULL,          KEY_NULL
    },
    {
        KEY_NULL,       KEY_NULL,          KEY_NULL,
        KEY_NULL,       KEY_NULL,          KEY_NULL
    },
};

static u8 aux_idle_query()
{
    return !g_aux.idle;
}
REGISTER_LP_TARGET(aux_lp_target) = {
    .name = "aux",
    .is_idle = aux_idle_query,
};

static void app_aux_init()
{
    log_info("app_aux_init");
    g_aux.idle = 1;
}

static void app_aux_exit()
{
    log_info("app_aux_exit");
    g_aux.idle = 0;
}

static int app_aux_key_event_handler(struct sys_event *event)
{
    struct key_event *key = &event->u.key;
    int key_event = aux_key_table[key->value][key->event];
    log_info("key_event:%d %d %d\n", key_event, key->value, key->event);
    switch (key_event) {
    case KEY_MODE_SWITCH:
        log_info("KEY_MODE_SWITCH");
        app_task_switch_next();
        break;

    case KEY_POWEROFF:
        goto_poweroff_cnt = 0;
        goto_poweroff_flag = 1;
        break;

    case KEY_POWEROFF_HOLD:
        log_info("poweroff flag:%d cnt:%d\n", goto_poweroff_flag, goto_poweroff_cnt);

        if (goto_poweroff_flag) {
            goto_poweroff_cnt++;
            if (goto_poweroff_cnt >= POWER_OFF_CNT) {
                goto_poweroff_cnt = 0;
                sys_enter_soft_poweroff(NULL);
            }
        }
        break;
    case KEY_ANC_SWITCH:
#if TCFG_AUDIO_ANC_ENABLE
        anc_mode_next();
#endif
        break;
    default:
        break;
    }

    return false;
}

/*
 * 系统事件处理函数
 */
static int event_handler(struct application *app, struct sys_event *event)
{
    switch (event->type) {
    case SYS_KEY_EVENT:
        /*
         * 普通按键消息处理
         */
        return app_aux_key_event_handler(event);
    case SYS_BT_EVENT:
        /*
         * 蓝牙事件处理
         */
        break;
    case SYS_DEVICE_EVENT:
        /*
         * 系统设备事件处理
         */
        if ((u32)event->arg == DEVICE_EVENT_FROM_OTG) {
            if (event->u.dev.event == DEVICE_EVENT_OUT) {
                app_task_switch_next();
                return true;
            }
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_POWER) {
            app_power_event_handler(&event->u.dev);
            return true;
        } else if ((u32)event->arg == DEVICE_EVENT_FROM_CHARGE) {
#if TCFG_CHARGE_ENABLE
            app_charge_event_handler(&event->u.dev);
#endif
            return true;
        }

        break;
    default:
        return false;
    }


    return false;
}

static int state_machine(struct application *app, enum app_state state,
                         struct intent *it)
{
    switch (state) {
    case APP_STA_CREATE:
        log_info("APP_STA_CREATE");
        break;
    case APP_STA_START:
        log_info("APP_STA_START");
        if (!it) {
            break;
        }
        switch (it->action) {
        case ACTION_AUX_MAIN:
            log_info("ACTION_AUX_MAIN");
            ///按键使能
            sys_key_event_enable();
            //关闭自动关机
            sys_auto_shut_down_disable();
            app_aux_init();
            g_aux.idle = 0;
            break;
        }
        break;
    case APP_STA_PAUSE:
        log_info("APP_STA_PAUSE");
        app_aux_exit();
        break;
    case APP_STA_RESUME:
        log_info("APP_STA_RESUME");
        app_aux_init();
        break;
    case APP_STA_STOP:
        log_info("APP_STA_STOP");
        app_aux_exit();
        break;
    case APP_STA_DESTROY:
        log_info("APP_STA_DESTROY");
        break;
    }

    return 0;
}

static const struct application_operation app_aux_ops = {
    .state_machine  = state_machine,
    .event_handler 	= event_handler,
};

/*
 * 注册aux模式
 */
REGISTER_APPLICATION(app_aux) = {
    .name 	= "aux",
    .action	= ACTION_AUX_MAIN,
    .ops 	= &app_aux_ops,
    .state  = APP_STA_DESTROY,
};

#endif

