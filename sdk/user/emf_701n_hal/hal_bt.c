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

#include "hw_config.h"
#if API_BT_ENABLE
#include "api/bt/api_bt.h"
#include "api/api_log.h"




#include "btcontroller_config.h"
#include "btctrler/btctrler_task.h"
#include "config/config_transport.h"
#include "btstack/avctp_user.h"
#include "btstack/btstack_task.h"
#include "bt_common.h"
// #include "edr_hid_user.h"
#include "le_common.h"
#include "standard_hid.h"
#include "rcsp_bluetooth.h"
// #include "app_comm_bt.h"

/*******************************************************************************************************************
**	Hardware  Defined
********************************************************************************************************************/

#include "system/includes.h"
///搜索完整结束的事件
#define HCI_EVENT_INQUIRY_COMPLETE                            0x01
///连接完成的事件
#define HCI_EVENT_CONNECTION_COMPLETE                         0x03
///断开的事件,会有一些错误码区分不同情况的断开
#define HCI_EVENT_DISCONNECTION_COMPLETE                      0x05
///连接中请求pin code的事件,给这个事件上来目前是做了一个确认的操作
#define HCI_EVENT_PIN_CODE_REQUEST                            0x16
///连接设备发起了简易配对加密的连接
#define HCI_EVENT_IO_CAPABILITY_REQUEST                       0x31
///这个事件上来目前是做了一个连接确认的操作
#define HCI_EVENT_USER_CONFIRMATION_REQUEST                   0x33
///这个事件是需要输入6个数字的连接消息，一般在键盘应用才会有
#define HCI_EVENT_USER_PASSKEY_REQUEST                        0x34
///<可用于显示输入passkey位置 value 0:start  1:enrer  2:earse   3:clear  4:complete
#define HCI_EVENT_USER_PRESSKEY_NOTIFICATION			      0x3B
///杰理自定义的事件，上电回连的时候读不到VM的地址。
#define HCI_EVENT_VENDOR_NO_RECONN_ADDR                       0xF8
///杰理自定义的事件，有测试盒连接的时候会有这个事件
#define HCI_EVENT_VENDOR_REMOTE_TEST                          0xFE
///杰理自定义的事件，可以认为是断开之后协议栈资源释放完成的事件
#define BTSTACK_EVENT_HCI_CONNECTIONS_DELETE                  0x6D


#define ERROR_CODE_SUCCESS                                    0x00
///<回连超时退出消息
#define ERROR_CODE_PAGE_TIMEOUT                               0x04
///<连接过程中linkkey错误
#define ERROR_CODE_AUTHENTICATION_FAILURE                     0x05
///<连接过程中linkkey丢失，手机删除了linkkey，回连就会出现一次
#define ERROR_CODE_PIN_OR_KEY_MISSING                         0x06
///<连接超时，一般拿远就会断开有这个消息
#define ERROR_CODE_CONNECTION_TIMEOUT                         0x08
///<连接个数超出了资源允许
#define ERROR_CODE_SYNCHRONOUS_CONNECTION_LIMIT_TO_A_DEVICE_EXCEEDED  0x0A
///<回连的时候发现设备还没释放我们这个地址，一般直接断电开机回连会出现
#define ERROR_CODE_ACL_CONNECTION_ALREADY_EXISTS                      0x0B
///<需要回连的设备资源不够。有些手机连了一个设备之后就会用这个拒绝。或者其它的资源原因
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_LIMITED_RESOURCES       0x0D
///<有些可能只允许指定地址连接的，可能会用这个去拒绝连接。或者我们设备的地址全0或者有问题
#define ERROR_CODE_CONNECTION_REJECTED_DUE_TO_UNACCEPTABLE_BD_ADDR    0x0F
///<连接的时间太长了，手机要断开了。这种容易复现可以联系我们分析
#define ERROR_CODE_CONNECTION_ACCEPT_TIMEOUT_EXCEEDED         0x10
///<经常用的连接断开消息。就认为断开就行
#define ERROR_CODE_REMOTE_USER_TERMINATED_CONNECTION          0x13
///<正常的断开消息，本地L2CAP资源释放之后就会发这个值上来了
#define ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST        0x16
///<回连的时候发现设备两次role switch失败了，发断开在重新连接
#define ERROR_CODE_ROLE_SWITCH_FAILED                         0x35
///杰理自定义，在回连的过程中使用page cancel命令主动取消成功会有这个命令
#define CUSTOM_BB_AUTO_CANCEL_PAGE                            0xFD  //// app cancle page
///杰理自定义，库里面有些功能需要停止page的时候会有。比如page的时候来了电话
#define BB_CANCEL_PAGE                                        0xFE  //// bb cancle page

/*******************************************************************************************************************
**	public Parameters
********************************************************************************************************************/

/*******************************************************************************************************************
**	static Parameters
********************************************************************************************************************/

#if TCFG_USER_EDR_ENABLE
//==========================================================
#if SNIFF_MODE_RESET_ANCHOR               //键盘鼠标sniff模式,固定小周期发包,多按键响应快
    #define SNIFF_MODE_TYPE               SNIFF_MODE_ANCHOR
    #define SNIFF_CNT_TIME                1/////<空闲100mS之后进入sniff模式

    #define SNIFF_MAX_INTERVALSLOT        12	//从机anchor point锚点间隔,间隔到了就会唤醒收发数据  time=n*0.625ms
    #define SNIFF_MIN_INTERVALSLOT        12
    #define SNIFF_ATTEMPT_SLOT            2		//锚点时间到唤醒后保持监听M->S slot个数, 必须<=SNIFF_MIN_INTERVALSLOT/2(注意监听要包含S-M slot,所以时间是SLOT*(2+1))
    #define SNIFF_TIMEOUT_SLOT            1		//监听到数据后,延长监听的格式,防止太快进入休眠 
    #define SNIFF_CHECK_TIMER_PERIOD      100
#else                                     //待机固定500ms sniff周期,待机功耗较低,按键唤醒有延时
    #define SNIFF_MODE_TYPE               SNIFF_MODE_DEF
    #define SNIFF_CNT_TIME                5/////<空闲5S之后进入sniff模式

    #define SNIFF_MAX_INTERVALSLOT        800
    #define SNIFF_MIN_INTERVALSLOT        100
    #define SNIFF_ATTEMPT_SLOT            4
    #define SNIFF_TIMEOUT_SLOT            1
    #define SNIFF_CHECK_TIMER_PERIOD      1000
#endif

//默认配置
static const edr_sniff_par_t hid_sniff_param = {
    .sniff_mode = SNIFF_MODE_TYPE,
    .cnt_time = SNIFF_CNT_TIME,
    .max_interval_slots = SNIFF_MAX_INTERVALSLOT,
    .min_interval_slots = SNIFF_MIN_INTERVALSLOT,
    .attempt_slots = SNIFF_ATTEMPT_SLOT,
    .timeout_slots = SNIFF_TIMEOUT_SLOT,
    .check_timer_period = SNIFF_CHECK_TIMER_PERIOD,
};

//----------------------------------
static const edr_init_cfg_t edr_default_config = {
    .page_timeout = 8000,
    .super_timeout = 8000,

#if SUPPORT_USER_PASSKEY            //io_capabilities ; /*0: Display only 1: Display YesNo 2: KeyboardOnly 3: NoInputNoOutput*/
    .io_capabilities = 2,
#else
    .io_capabilities = 3,
#endif

    .authentication_req = 0,        //authentication_requirements: 0:NO Bonding  1 :No bonding io cap to auth,2:Bonding,3:boding io cap auth,4: boding ,5:boding io cap auth
									//cesar fix 2 to 0
    .oob_data = 0,          		//cesar fix 2 to 0
    .sniff_param = &hid_sniff_param,
    .class_type = EDR_ICON,
    .report_map = NULL,         //不在这里设置
    .report_map_size = 0,
};
#endif

/*****************************************************************************************************
**	static Function
******************************************************************************************************/
//选择蓝牙从机模式
void select_btmode(uint16_t trps)
{
    uint8_t i;
    api_bt_ctb_t* bt_ctbp;

    logi("bt trps %x\n", trps);

    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    if(m_trps & BT0_SUPPORT & BIT(TR_BLE_RF)){
        bt_ctbp = api_bt_get_ctb(TR_BLE_RF);
	    
        logi("---------app select 24g--------\n");
        rf_set_24g_hackable_coded(CFG_RF_24G_CODE_ID);
        while (ble_hid_is_connected()) {
            putchar('W');
            os_time_dly(1);
        }
		if(api_bt_is_bonded(BT_ID0, TR_BLE_RF)){
            le_hogp_set_reconnect_adv_cfg(ADV_DIRECT_IND_LOW, 0);
			call_hogp_adv_config_set();
        }
        if(NULL != bt_ctbp){
            hal_bt_enable(BT_ID0,TR_BLE_RF,bt_ctbp->enable);
        }
    }else if(m_trps & BT0_SUPPORT & BIT(TR_BLE)) {
        logi("---------app select ble--------\n");
        bt_ctbp = api_bt_get_ctb(TR_BLE);
        rf_set_24g_hackable_coded(0);
        while (ble_hid_is_connected()) {
            putchar('W');
            os_time_dly(1);
        }
        if(NULL != bt_ctbp){
            hal_bt_enable(BT_ID0,BT_BLE,bt_ctbp->enable);
        }
    }else{
        hal_bt_enable(BT_ID0, BT_BLE, 0);
    }
    #endif


    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    if(m_trps & BT0_SUPPORT & BIT(TR_EDR)) {
        #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
        logi("---------app select edr--------\n");
        hal_bt_enable(BT_ID0,BT_EDR,bt_ctbp->enable);          //打开edr
		#endif
    }else{
        hal_bt_enable(BT_ID0, BT_EDR, 0);
    }
    #endif
}


//BLE + 2.4g 从机接收数据
void ble_hid_transfer_channel_recieve(uint8_t* p_attrib_value,uint16_t length)
{
    bt_evt_rx_t evt;
    evt.bts = BT_HID;
	evt.buf = p_attrib_value;
	evt.len = length;
    if(length) api_bt_event(BT_ID0,BT_BLE,BT_EVT_RX,&evt);    
}

//edr 从机接收数据
void edr_out_callback(u8 *buffer, u16 length, u16 channel)
{
    bt_evt_rx_t evt;
    evt.bts = BT_HID;
	evt.buf = buffer+1;     //注意要去掉0XA2
	evt.len = length-1;

    logd("%s,chl=%d,len=%d", __FUNCTION__, channel, length);
    if(length) api_bt_event(BT_ID0,BT_EDR,BT_EVT_RX,&evt);
}

   
/*****************************************************************************************************
**  hal bt Function
******************************************************************************************************/
bool hal_bt_get_mac(uint8_t id, bt_t bt, uint8_t *buf )
{
    bool ret = false;

    if(BT_ID0 != id) return false;

    memcpy(buf,bt_get_mac_addr(),6);        //获取基础mac地址, EDR地址为基地址

    return ret;
}

bool hal_bt_is_bonded(uint8_t id, bt_t bt)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	    
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_RF)
    case BT_RF:
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_RFC)
    case BT_RFC:
        break;
    #endif
    }   
    
    return ret;
}
bool hal_bt_debond(uint8_t id, bt_t bt)
{
    bool ret = false;
    api_bt_ctb_t* bt_ctbp;

    if(BT_ID0 != id) return false;

	bt_ctbp = api_bt_get_ctb(bt);

    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
         if(bt != BT_EDR){
            clear_app_bond_info(bt);
            ble_module_enable(0);			//断开连接
            sdk_bt_adv_set(bt, BLE_ADV_IND);            //说明必须修改设置后再开启广播
            
            ble_module_enable(1);			//重新开启广播
            bt_ble_adv_enable(bt_ctbp->enable);
            ret = true;
        }
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        ret = !multi_client_clear_pair();
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	 
          ret = !delete_last_device_from_vm();   
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        //unsupport
        break;
    #endif
    }   
    
    return ret;
}

bool hal_bt_disconnect(uint8_t id, bt_t bt)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        ble_app_disconnect();
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	
        user_hid_disconnect();    
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        break;
    #endif
    }   
    
    return ret;
}
bool hal_bt_enable(uint8_t id, bt_t bt,bool en)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        ble_module_enable(en);
        if(en) bt_ble_adv_enable(en);
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        if(en){
            ble_gatt_client_module_enable(1);
            ble_gatt_client_scan_enable(1);
        }else{
            ble_gatt_client_scan_enable(0);
        }
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	
        if (en) {
            user_hid_enable(1);
            btctrler_task_init_bredr();
            if(!bt_connect_phone_back_start()){     //先回连
                bt_wait_phone_connect_control(1);
            }
            sys_auto_sniff_controle(1, NULL);
        } else {
            user_hid_enable(0);
            bt_wait_phone_connect_control(0);
            sys_auto_sniff_controle(0, NULL);
            btctrler_task_close_bredr();
        }    
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        if(en){
            bt_emitter_start_search_device();
        }else{
            bt_emitter_stop_search_device();
        }
        break;
    #endif
    }   
    
    return ret;
}
bool hal_bt_uart_tx(uint8_t id, bt_t bt,uint8_t *buf, uint16_t len)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        return (0 == ble_hid_transfer_channel_send(buf, len));
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        return  multi_client_user_server_write( buf,  len);
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 	
	       //unsupport
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        //unsupport
        break;
    #endif
    }   
    
    return ret;
}



bool hal_bt_hid_tx(uint8_t id, bt_t bt,uint8_t*buf, uint16_t len)
{
    bool ret = false;

    if(BT_ID0 != id) return false;
    switch(bt){
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
    case BT_BLE: 	
    case BT_BLE_RF: 			//BLE模拟2.4G
        ret = (0 == ble_hid_data_send(buf[0], buf+1, len-1));
        break;
    #endif
    #if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
    case BT_BLEC:
    case BT_BLEC_RF:
        //unsupport
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDR)
    case BT_EDR: 
        ret = (0 == edr_hid_data_send(buf[0], buf+1, len-1));	    
        break;
    #endif
    #if BT0_SUPPORT & BIT_ENUM(TR_EDRC)
    case BT_EDRC:
        //unsupport
        break;
    #endif
    }   
    
    return ret;
}



#if BT_HID_SUPPORT
uint8_t *ble_report_mapp = NULL;
uint8_t *edr_report_mapp = NULL;
static void user_hid_set_reportmap (bt_t bt)
{
    api_bt_ctb_t* bt_ctbp;
	bt_ctbp = api_bt_get_ctb(bt);

    if(bt_ctbp->types & BIT(DEV_TYPE_HID)){
        uint8_t i;
        uint8_t *hid_mapp, *report_mapp;
        uint16_t len, map_len = 0;

        //get report map len
        for(i=0; i<16; i++){
            if(bt_ctbp->hid_types & BIT(i)){
                len = get_hid_desc_map((trp_t)bt, i ,NULL);;
                map_len += len;
            }
        }

        if(BT_BLE == bt){
            free(ble_report_mapp);                  //防止多次调用内存溢出
            ble_report_mapp = malloc(map_len);
            report_mapp = ble_report_mapp;
        }else{
            free(edr_report_mapp);                  //防止多次调用内存溢出
            edr_report_mapp = malloc(map_len);
            report_mapp = edr_report_mapp;
        }

        if(NULL == report_mapp) return;
        for(i=0; i<16; i++){
            if(bt_ctbp->hid_types & BIT(i)){
                len = get_hid_desc_map((trp_t)bt, i ,&hid_mapp);
                memcpy(report_mapp, hid_mapp, len);
                report_mapp += len;
            }
        }
        
        if(BT_BLE == bt){
            report_mapp = ble_report_mapp;
            #if TCFG_USER_BLE_ENABLE
            le_hogp_set_ReportMap(ble_report_mapp, map_len);
            #endif
        }else{
            report_mapp = edr_report_mapp;
            #if TCFG_USER_EDR_ENABLE
            user_hid_set_ReportMap(edr_report_mapp, map_len);
            #endif
        }
        logd("bt%d report map %d: ",bt,map_len);dumpd(report_mapp, map_len);
    }
}
#endif

bool hal_bt_init(uint8_t id)
{
    bool ret = false;

    if(BT_ID0 != id) return false;

    #if TCFG_USER_EDR_ENABLE
    btstack_edr_start_before_init(&edr_default_config, 0);
    //change init
    user_hid_init(edr_out_callback);
    #endif

    #if BLE_HID_SUPPORT && TCFG_USER_BLE_ENABLE
    user_hid_set_reportmap (BT_BLE);
    #endif
    #if EDR_HID_SUPPORT && TCFG_USER_EDR_ENABLE
    user_hid_set_reportmap (BT_EDR);
    #endif

    // btstack_init();
    return ret;
}


    

bool hal_bt_deinit(uint8_t id)
{
    bool ret = false;

    if(BT_ID0 != id) return false;

    #if TCFG_USER_BLE_ENABLE
    #endif

    #if TCFG_USER_EDR_ENABLE
    btstack_edr_exit(0);
    #endif

    
    #if BT_HID_SUPPORT
    free(ble_report_mapp);
    free(edr_report_mapp);
    #endif

    return ret;
}
void hal_bt_task(void* pa)
{
    UNUSED_PARAMETER(pa);
}


#endif

