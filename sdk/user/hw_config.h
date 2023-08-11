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


#ifndef _hw_config_h
#define _hw_config_h
#include "utils/emf_defined.h"

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************************************
** Defined
*******************************************************************************************************/
#define HEAP_ID						0					/* used heap_n file*/
#define LOG_ENABLE                  1
#define CRC8_TABLE_EANBLE 			0					/*杰里默认有crc8校验*/

//sdk modules disable
#define TCFG_ADKEY_ENABLE					0
#define TCFG_MATRIX_KEY_ENABLE              0
#define TCFG_CODE_SWITCH_ENABLE             0 //code switch使能
#define TCFG_IOKEY_ENABLE					0


#define PROJECT_KM                  0           //keyboard and mouse project
#define PROJECT_GAMEPAD             1           //keyboard and mouse

#if PROJECT_KM

	#define HELLOW_KEYBOARD						1
	#if HELLOW_KEYBOARD
		#define DEV_TRPS_DEFAULT				(BIT(TR_BLE)|BIT(TR_USBD))		/*产品传输层支持*/
		#define SW_VERSION                     	0x01
        #define DEFAULT_NAME			       	"hellow_keyboard"
        #define DEFAULT_MODEL					"HKB"
	#else

	#endif

#elif PROJECT_GAMEPAD
	#define GAMEPAD1					1
	

	#if GAMEPAD1	
		#define APP_KEY_ENABLE			1
		#define API_WDT_ENABLE			0
		#define TCFG_LOWPOWER_LOWPOWER_SEL			0		/*关闭修改防止定时器不准确*/

		#define API_OTG_BIT_ENABLE			BIT(0)
		#define API_USBD_BIT_ENABLE			BIT(0)
		#define USBD_TYPE_SUPPORT 			(BIT_ENUM(DEV_TYPE_MSD))
		#define USBD_RAM_DISK_ENABLE		1
		// #define USBD_TYPE_SUPPORT		(BIT_ENUM(DEV_TYPE_HID))
		// #define USBD_HID_SUPPORT			(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE) | BIT_ENUM(HID_TYPE_CONSUMER))
		// #define USBD_HID_SUPPORT			(BIT_ENUM(HID_TYPE_KB))

		#define API_USBH_BIT_ENABLE			BIT(0)
		#define USBH_TYPE_SUPPORT			(BIT_ENUM(DEV_TYPE_HID) | BIT_ENUM(DEV_TYPE_HUB))
		#define USBH_HID_SUPPORT			(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE) | BIT_ENUM(HID_TYPE_CONSUMER))


		//bt 
		#define BT0_SUPPORT					(BIT_ENUM(TR_BLE))
		// #define BLE_HID_SUPPORT				(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_SWITCH))
		#define BLE_HID_SUPPORT				(BIT_ENUM(HID_TYPE_KB) | BIT_ENUM(HID_TYPE_MOUSE))
		#define EDR_HID_SUPPORT				BIT_ENUM(HID_TYPE_KB)

		#define DEV_TRPS_DEFAULT				BT0_SUPPORT
		#define SW_VERSION                     	0x01
        #define DEFAULT_NAME			       	"gamepad"
        #define DEFAULT_MODEL					"GP_dev"
	#endif
#endif

#include "hw_board.h"
#include "emf_config.h"

//sdk config
// #define TCFG_UART0_TX_PORT  				IO_PORTA_00		在hw_board中设置
// #define TCFG_DCDC_PORT_SEL				NO_CONFIG_PORT  在hw_board中设置
#ifndef TCFG_CHARGE_ENABLE
#define TCFG_CHARGE_ENABLE					DISABLE_THIS_MOUDLE
#endif
#ifndef TCFG_CHARGE_FULL_MA
#define TCFG_CHARGE_FULL_MA					CHARGE_FULL_mA_10
#endif
#ifndef TCFG_CHARGE_MA
#define TCFG_CHARGE_MA						CHARGE_mA_50
#endif
#ifndef TCFG_LOWPOWER_POWER_SEL
#define TCFG_LOWPOWER_POWER_SEL				PWR_LDO15	/*电源模式设置，可选DCDC和LDO*/
#endif
#ifndef TCFG_LOWPOWER_LOWPOWER_SEL
#define TCFG_LOWPOWER_LOWPOWER_SEL			SLEEP_EN	/*SNIFF状态下芯片是否进入powerdown*/
#endif
#ifndef TCFG_AUDIO_ENABLE
#define TCFG_AUDIO_ENABLE					0
#endif
#ifndef TCFG_AUDIO_DAC_CONNECT_MODE
#define TCFG_AUDIO_DAC_CONNECT_MODE         DAC_OUTPUT_MONO_LR_DIFF
#endif
#ifndef CONFIG_FLASH_SIZE
#define CONFIG_FLASH_SIZE                   FLASH_SIZE_2M    /*配置FLASH大小*/
#endif



#if API_OTG_BIT_ENABLE
	#define TCFG_OTG_USB_DEV_EN     API_OTG_BIT_ENABLE
	#if API_USBD_BIT_ENABLE && API_USBH_BIT_ENABLE
		#define TCFG_OTG_MODE       (OTG_HOST_MODE | OTG_SLAVE_MODE | OTG_CHARGE_MODE)
	#elif API_USBD_BIT_ENABLE
		#define TCFG_OTG_MODE       (OTG_SLAVE_MODE | OTG_CHARGE_MODE)
	#elif API_USBH_BIT_ENABLE
		#define TCFG_OTG_MODE       (OTG_HOST_MODE | OTG_CHARGE_MODE )
	#else
		#define TCFG_OTG_MODE       0
	#endif

#else
	#define TCFG_OTG_USB_DEV_EN     0
#endif


#if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF) | BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
	#define TCFG_USER_BLE_ENABLE                1
#else
	#define TCFG_USER_BLE_ENABLE                0
#endif
#if BT0_SUPPORT & (BIT_ENUM(TR_EDR) | BIT_ENUM(TR_EDRC))
	#define TCFG_USER_EDR_ENABLE                1
#else
	#define TCFG_USER_EDR_ENABLE                0
#endif

#if (BT0_SUPPORT & BIT_ENUM(TR_BLE)) && BLE_HID_SUPPORT
#define CONFIG_BT_SM_SUPPORT_ENABLE        	1
#else
#define CONFIG_BT_SM_SUPPORT_ENABLE        	0
#endif

#if BT0_SUPPORT & (BIT_ENUM(TR_BLE) | BIT_ENUM(TR_BLE_RF))
#define CONFIG_BT_GATT_SERVER_NUM          	1 /*配置从机server个数*/
#else
#define CONFIG_BT_GATT_SERVER_NUM        	0
#endif


#if BT0_SUPPORT & (BIT_ENUM(TR_BLEC) | BIT_ENUM(TR_BLE_RFC))
#define CONFIG_BT_GATT_CLIENT_NUM          	1 /*配置主机client个数*/
#else
#define CONFIG_BT_GATT_CLIENT_NUM        	0
#endif

#if TCFG_USER_BLE_ENABLE
#define BLE_HID_EN                			1
#define TRANS_DATA_EN             			0
#endif


#ifndef USER_SUPPORT_PROFILE_SPP
#define USER_SUPPORT_PROFILE_SPP    0
#endif
#ifndef USER_SUPPORT_PROFILE_HFP
#define USER_SUPPORT_PROFILE_HFP    0
#endif
#ifndef USER_SUPPORT_PROFILE_A2DP
#define USER_SUPPORT_PROFILE_A2DP   0
#endif
#ifndef USER_SUPPORT_PROFILE_AVCTP
#define USER_SUPPORT_PROFILE_AVCTP  0
#endif
#ifndef USER_SUPPORT_PROFILE_HID
#define USER_SUPPORT_PROFILE_HID    1
#endif
#ifndef USER_SUPPORT_PROFILE_PNP
#define USER_SUPPORT_PROFILE_PNP    0
#endif
#ifndef USER_SUPPORT_PROFILE_PBAP
#define USER_SUPPORT_PROFILE_PBAP   0
#endif



#ifdef __cplusplus
}
#endif
#endif





