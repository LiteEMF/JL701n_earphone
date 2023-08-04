#ifndef _AUDIO_CVP_DUT_H_
#define _AUDIO_CVP_DUT_H_

#include "generic/typedef.h"

/*******************CVP测试命令集*********************/
#define	CVP_DUT_DMS_MASTER_MIC				0x01		//DMS-测试主MIC		    SMS-关闭CVP算法
#define	CVP_DUT_DMS_SLAVE_MIC   			0x02 		//DMS-测试副MIC
#define CVP_DUT_DMS_OPEN_ALGORITHM    		0x03		//DMS-测试dms算法	    SMS-打开CVP算法

#define CVP_DUT_NS_OPEN						0x04		//打开NS模块
#define CVP_DUT_NS_CLOSE					0x05		//关闭NS模块

//ANC MIC 频响测试指令，且需切换到CVP_DUT_MODE_BYPASS，在通话HFP前设置
#define CVP_DUT_FF_MIC						0x06		//测试TWS FF MIC/头戴式左耳FF_MIC
#define CVP_DUT_FB_MIC						0x07		//测试TWS FB MIC/头戴式左耳FB_MIC
#define CVP_DUT_HEAD_PHONE_R_FF_MIC			0x08		//测试头戴式右耳FF_MIC
#define CVP_DUT_HEAD_PHONE_R_FB_MIC			0x09		//测试头戴式右耳FB_MIC

#define CVP_DUT_OPEN_ALGORITHM				0x0a		//CVP算法开启
#define CVP_DUT_CLOSE_ALGORITHM				0x0b		//CVP算法关闭
#define	CVP_DUT_MODE_ALGORITHM_SET			0x0c		//CVP_DUT 开关算法模式
#define CVP_DUT_POWEROFF					0x0d		//耳机关机指令

//三麦通话测试指令
#define CVP_DUT_3MIC_MASTER_MIC             CVP_DUT_DMS_MASTER_MIC      //测试三麦算法主麦频响
#define CVP_DUT_3MIC_SLAVE_MIC              CVP_DUT_DMS_SLAVE_MIC       //测试三麦算法副麦频响
#define CVP_DUT_3MIC_FB_MIC                 0x0e                        //测试三麦算法FBmic频响
#define CVP_DUT_3MIC_OPEN_ALGORITHM         CVP_DUT_DMS_OPEN_ALGORITHM  //测试三麦算法副麦频响

//回复命令
#define CVP_DUT_ACK_SUCCESS					0x01		//命令接收成功
#define CVP_DUT_ACK_FALI					0x02		//命令接收失败
/*******************CVP测试命令集END*********************/

#define CVP_DUT_SPP_MAGIC					0x55BB
#define CVP_DUT_PACK_NUM					sizeof(cvp_dut_data_t)			//数据包总长度

typedef enum {
    /*
       CVP_DUT 算法模式
       通话中设置命令，可自由开关每个算法，测试性能指标
    */
    CVP_DUT_MODE_ALGORITHM = 1,

    /*
       CVP_DUT BYPASS模式
       通话前设置命令，不经过算法，可自由选择需要测试的MIC
       每次通话结束默认恢复成 CVP_DUT算法模式
    */
    CVP_DUT_MODE_BYPASS    = 2,

} CVP_DUT_mode_t;

typedef enum {
    CVP_3MIC_OUTPUT_SEL_DEFAULT = 0,     /*默认输出*/
    CVP_3MIC_OUTPUT_SEL_MASTER,          /*Talk mic原始数据*/
    CVP_3MIC_OUTPUT_SEL_SLAVE,           /*FF mic原始数据*/
    CVP_3MIC_OUTPUT_SEL_FBMIC,           /*FB mic原始数据*/
} CVP_3MIC_OUTPUT_ENUM;

typedef struct {
    u16 magic;
    u16 crc;
    u8 dat[4];
} cvp_dut_data_t;

typedef struct {
    u8 parse_seq;
    u8 mic_ch_sel;				//bypass模式下，MIC通道号
    u8 mode;					//CVP DUT MODE
    cvp_dut_data_t rx_buf;		//spp接收buf
    cvp_dut_data_t tx_buf;		//spp发送buf
} cvp_dut_t;
//cvp_dut 测试初始化接口
void cvp_dut_init(void);

//cvp_dut 卸载接口
void cvp_dut_unit(void);

//cvp_dut MIC 通道 获取接口
u8 cvp_dut_mic_ch_get(void);

//cvp_dut 模式获取
u8 cvp_dut_mode_get(void);

//cvp_dut 模式设置
void cvp_dut_mode_set(u8 mode);

#endif/*_AUDIO_CVP_DUT_*/
