#ifndef _ICSD_ANC_APP_H
#define _ICSD_ANC_APP_H

/*===========接口说明=============================================
一. void sd_anc_init(audio_anc_t *param,u8 mode)
  1.调用该函数启动sd anc训练
  2.param为ANC ON模式下的audio_anc_t参数指针
  3.mode：0. 普通模式   1. htarget 数据上传模式

二. void sd_anc_htarget_data_send(float *h_freq,float *hszpz_out,float *hpz_out, float *htarget_out,int len)
  1.htarget数据上传模式下，当数据准备好后会自动调用该函数

三. void sd_anc_htarget_data_end()
  1.htarget数据上传模式下，当数据上传完成后需要调用该函数进入训练后的ANC ON模式

四. void anc_user_train_tone_play_cb()
  1.当前ANC ON提示音结束时回调该函数进入 SD ANC 训练
  2.SD ANC训练结束后会切换到ANC ON模式

 	1:rdout   + lout		2:rdout   + rerrmic
 	3:rdout   + rrefmic		4:ldout   + lerrmic
 	5:ldout   + lrefmic		6:rerrmic + rrefmic
 	7:lerrmic + lrefmic

	anc_sr, 0: 11.7k  1: 23.4k  2: 46.9k  3: 93.8k  4: 187.5k     5: 375k     6: 750k      7: 1.5M

================================================================*/

#include "anc.h"
#include "dac.h"
#include "app_config.h"
#include "icsd_anc.h"

#ifndef CLIENT_BOARD
#define CLIENT_BOARD	0
#endif/*ifndef CLIENT_BOARD*/

#define DEVE_BOARD							0x00	//开发板
#define HT03_HYBRID_6F						0x01 	//CUSTOM1_CFG
#define HT03_HYBRID_6G						0x02 	//CUSTOM2_CFG
#define A896_HYBRID_6G						0x03 	//CUSTOM3_CFG
#define HT05_6G               		        0x04 	//CUSTOM4_CFG
#define ANC05_HYBRID						0x05 	//CUSTOM5_CFG
#define G02_HYBRID_6G						0x06 	//CUSTOM6_CFG
#define AC098_HYBRID_6G						0x07 	//CUSTOM7_CFG
#define H3_HYBRID							0x08	//CUSTOM8_CFG

//自适应板级配置，适配不同样机
#define ADAPTIVE_CLIENT_BOARD			HT03_HYBRID_6G

#if ADAPTIVE_CLIENT_BOARD == HT03_HYBRID_6G
#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				6				/*ANC自适应CMP滤波器阶数，原厂指定*/

#elif ADAPTIVE_CLIENT_BOARD == G02_HYBRID_6G
#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				6				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				4				/*ANC自适应CMP滤波器阶数，原厂指定*/

#elif ADAPTIVE_CLIENT_BOARD == ANC05_HYBRID
#define ICSD_ANC_MODE     			HEADSET_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				4				/*ANC自适应CMP滤波器阶数，原厂指定*/

#elif ADAPTIVE_CLIENT_BOARD == H3_HYBRID
#define ICSD_ANC_MODE     			HEADSET_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				4				/*ANC自适应CMP滤波器阶数，原厂指定*/

#else
#define ICSD_ANC_MODE     			TWS_TONE_BYPASS_MODE
#define ANC_ADAPTIVE_FF_ORDER				8				/*ANC自适应FF滤波器阶数, 原厂指定*/
#define ANC_ADAPTIVE_FB_ORDER				5				/*ANC自适应FB滤波器阶数，原厂指定*/
#define ANC_ADAPTIVE_CMP_ORDER				4				/*ANC自适应CMP滤波器阶数，原厂指定*/

#endif/*ADAPTIVE_CLIENT_BOARD*/



#define ANC_USER_TRAIN_EN			1

//TWS_TONE_BYPASS_MODE
//TWS_BYPASS_MODE
//HEADSET_TONE_BYPASS_MODE
//HEADSET_BYPASS_MODE
//TWS_PN_MODE
//TWS_TONE_PN_MODE

#if ICSD_ANC_MODE == HEADSET_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    0
#define TONE_DELAY  700
#define BYPASS_MAXTIME              4
#define PZ_MAXTIME                  4

#elif ICSD_ANC_MODE == HEADSET_TONES_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define TONE_DELAY  50
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  3

#elif ICSD_ANC_MODE == TWS_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    0
#define TONE_DELAY  700
#define BYPASS_MAXTIME              4
#define PZ_MAXTIME                  4

#elif ICSD_ANC_MODE == TWS_TONE_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define TONE_DELAY  700
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  3

#elif ICSD_ANC_MODE == HEADSET_TONE_BYPASS_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define TONE_DELAY  700
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  3

#elif ICSD_ANC_MODE == TWS_TONE_PN_MODE
#define ANC_USER_TRAIN_TONE_MODE    1
#define TONE_DELAY  700
#define BYPASS_MAXTIME              2
#define PZ_MAXTIME                  6

#else
#define ANC_USER_TRAIN_TONE_MODE    0
#define TONE_DELAY  700
#define BYPASS_MAXTIME              4
#define PZ_MAXTIME                  4
#endif

#define ANC_USER_TRAIN_MODE         2//0:FF 1:FB 2:HY
#define ANC_USER_TRAIN_SR_SEL       2//46.9k
#define ANC_ADAPTIVE_CMP_EN         0 //自适应CMP补偿
#define ANC_AUTO_PICK_EN            0 //智能免摘
#define ANC_AUTO_PICK_HOLDTIME      10//多长秒不说话返回降噪模式
#define ANC_EAR_RECORD_EN           1 //耳道记忆
#define EAR_RECORD_MAX  		    5
#define ANC_DFF_AFB_EN              1//默认FF + 自适应FB  1:判断为自适应成功  0:判断为自适应失败
#define ANC_AFF_DFB_EN              1//默认FB + 自适应FF  1:判断为自适应成功  0:判断为自适应失败

extern float (*vmdata_FL)[25];//ff fgq
extern float (*vmdata_FR)[25];//ff fgq
extern int *vmdata_num;

#define ANC_USE_RECORD     1
#define ANC_SUCCESS        2
#define ANC_USE_DEFAULT    3

//自适应训练结果
#define ANC_ADAPTIVE_RESULT_LFF		BIT(0)	//使用LFF自适应or记忆参数
#define ANC_ADAPTIVE_RESULT_LFB    	BIT(1)	//使用LFB自适应参数
#define ANC_ADAPTIVE_RESULT_RFF     BIT(2)	//使用RFF自适应or记忆参数
#define ANC_ADAPTIVE_RESULT_RFB     BIT(3)	//使用RFB自适应参数

struct icsd_fb_ref {
    float m_value;
    float sen;
    float in_q;
};

struct icsd_ff_ref {
    float fre[75];
    float db[75];
};

#define TEST_EAR_RECORD_EN          0
//TEST_ANC_COMBINATION_OFF
//TEST_TFF_TFB_TONE			          // bypassl强制失败
//TEST_TFF_TFB_BYPASS			      // tonel强制失败
//TEST_TFF_DFB				          // FB训练强制失败
//TEST_DFF_TFB				          // PZ强制失败
//TEST_DFF_DFB				          // tonel,bypassl,PZ强制失败
//TEST_DFF_DFB_2                      // tonel,PZ强制失败
#define TEST_ANC_COMBINATION	TEST_ANC_COMBINATION_OFF

#endif/*_ICSD_ANC_APP_H*/
