#ifndef _SD_ANC_LIB_H
#define _SD_ANC_LIB_H

#include "anc.h"
#include "timer.h"
#define ANC_TRAIN_TIMEOUT_S         5//10//秒
#define TWS_STA_SIBLING_CONNECTED   0x00000002//tws已连接

#define  TONEL_DELAY     50
#define  TONER_DELAY     2400
#define  PZL_DELAY       900
#define  PZR_DELAY       3300

#define TEST_ANC_COMBINATION_OFF    0x00
#define TEST_TFF_TFB_TONE			0x01          // bypassl强制失败
#define TEST_TFF_TFB_BYPASS			0x02          // tonel强制失败
#define TEST_TFF_DFB				0x03          // FB训练强制失败
#define TEST_DFF_TFB				0x04          // PZ强制失败
#define TEST_DFF_DFB				0x05          // tonel,bypassl,PZ强制失败
#define TEST_DFF_DFB_2              0x06          // tonel,PZ强制失败

#define TWS_TONE_BYPASS_MODE  		1
#define TWS_BYPASS_MODE      		2
#define HEADSET_TONE_BYPASS_MODE 	3
#define HEADSET_BYPASS_MODE  		4
#define TWS_PN_MODE			 		5
#define TWS_TONE_PN_MODE     		6
#define HEADSET_TONES_MODE          7

//user_train_state------------------------
#define ANC_USER_TRAIN_STEP         BIT(0)
#define ANC_USER_TRAIN_DMA_READY	BIT(1)
#define ANC_USER_TRAIN_TOOL_DATA 	BIT(2)
#define ANC_PREDATA_DMA             BIT(3)
#define ANC_DAC_MODE_H1_DIFF        BIT(4)
#define ANC_USER_TRAIN_DMA_EN       BIT(5)
#define ANC_USER_TRAIN_INFSYNC      BIT(6)
#define ANC_USER_TRAIN_REVERSE      BIT(7)
#define ANC_USER_TRAIN_TONEMODE     BIT(8)
#define ANC_TRAIN_TONE_PLAY			BIT(9)
#define ANC_TRAIN_TONE_END			BIT(10)
#define ANC_TRAIN_TONE_FIRST 		BIT(11)
#define ANC_TRAIN_IIR_READY 		BIT(12)
#define ANC_MASTER_RX_PZSYNC 		BIT(13)
#define ANC_FB_IIR_AB_DONE 			BIT(14)
#define ANC_FF_IIR_AB_DONE 			BIT(15)
#define ANC_PZOUT_INITED 			BIT(16)
#define ANC_SZOUT_INITED 			BIT(17)
#define ANC_USER_TRAIN_RUN 			BIT(18)
#define ANC_TRAIN_TIMEOUT 			BIT(19)
#define ANC_TRAIN_DMA_ON			BIT(20)
#define ANC_TONE_DAC_ON				BIT(21)
#define ANC_TONE_ANC_READY			BIT(22)

//icsd_anc_contral------------------------
#define ICSD_MIC_OPEND         		BIT(0)
#define ICSD_MIC_MONITOR_ON         BIT(1)
#define ICSD_SPEAK_MODE_ON          BIT(2)

//SZPZ->anc_err---------------------------
#define ANC_ERR_LR_BALANCE          BIT(0)//左右耳不平衡
#define ANC_ERR_WIND_NOISE          BIT(1)//风噪
#define ANC_ERR_SHAKE               BIT(2)//震动
#define ANC_ERR_SOUND_PRESSURE      BIT(3)//声压太低

//icsd_anc_function-----------------------
#define ANC_ADAPTIVE_CMP            BIT(0)
#define ANC_AUTO_PICK               BIT(1)

#define TARLEN    	120//120
#define TARLEN_L	40//8+32
#define DRPPNT   	10// 0,46,92,138,184

struct icsd_anc_board_param {
    void  *anc_data_l;
    void  *anc_data_r;
    u8 default_ff_gain;
    u8 default_fb_gain;
    s8 tool_ffgain_sign;
    s8 tool_fbgain_sign;
    s8 tool_target_sign;
    s8 tfb_sign;
    s8 tff_sign;
    s8 bfb_sign;
    s8 bff_sign;
    s8 bypass_sign;
    float m_value_l;			//80 ~ 200   //135  最深点位置中心值
    float sen_l;     			//12 ~ 22    //18   最深点深度
    float in_q_l;  				//0.4 ~ 1.2  //0.6	最深点降噪宽度
    float m_value_r;			//80 ~ 200   //135  最深点位置中心值
    float sen_r;     			//12 ~ 22    //18   最深点深度
    float in_q_r;  				//0.4 ~ 1.2  //0.6	最深点降噪宽度
    float *mse_tar;
    int   gold_curve_en;
    float gain_a_param;
    u8    gain_a_en;
    u8    cmp_abs_en;
    u8    jt_en;
    int   idx_begin;
    u8  FB_NFIX;
    u16 fb_w2r;
    float sen_offset_l;
    float sen_offset_r;
    u8  gain_min_offset;
    u8  gain_max_offset;
    u8  ff_target_fix_num_l;
    u8  ff_target_fix_num_r;
    u8	pz_max_times;
    u8	bypass_max_times;
    u8	ff_yorder;
    u8	fb_yorder;
    u8	cmp_yorder;
    float bypass_volume;
    float minvld;
    //ctl
    u8 mode;
    u8 FF_FB_EN;
    u8 dma_belong_to;
    u32 tone_jiff;
};
extern struct icsd_anc_board_param *ANC_BOARD_PARAM;

struct icsd_anc_backup {
    int gains_alogm;
    u8 gains_l_ffmic_gain;
    u8 gains_l_fbmic_gain;
    u8 gains_r_ffmic_gain;
    u8 gains_r_fbmic_gain;
    float gains_l_ffgain;
    float gains_l_fbgain;
    float gains_r_ffgain;
    float gains_r_fbgain;
    double *lff_coeff;
    double *lfb_coeff;
    double *rff_coeff;
    double *rfb_coeff;
    u8 lff_yorder;
    u8 lfb_yorder;
    u8 rff_yorder;
    u8 rfb_yorder;
    u8 ff_1st_dcc;
    u8 fb_1st_dcc;
    u8 ff_2nd_dcc;
    u8 fb_2nd_dcc;
    u8 gains_drc_en;
};
extern volatile struct icsd_anc_backup *CFG_BACKUP;

struct icsd_anc {
    audio_anc_t *param;
    u16	*anc_fade_gain;
    u8 mode;
    u8 train_index;
    u8 adaptive_run_busy;			//自适应训练中
    int	*gains_alogm;
    u8 *gains_l_ffmic_gain;
    u8 *gains_l_fbmic_gain;
    u8 *gains_r_ffmic_gain;
    u8 *gains_r_fbmic_gain;
    float *gains_l_ffgain;
    float *gains_l_fbgain;
    float *gains_l_cmpgain;
    float *gains_r_ffgain;
    float *gains_r_fbgain;
    float *gains_r_cmpgain;
    double **lff_coeff;
    double **lfb_coeff;
    double **lcmp_coeff;
    double **rff_coeff;
    double **rfb_coeff;
    double **rcmp_coeff;
    u8 *lff_yorder;
    u8 *lfb_yorder;
    u8 *lcmp_yorder;
    u8 *rff_yorder;
    u8 *rfb_yorder;
    u8 *rcmp_yorder;
    u8 *ff_1st_dcc;
    u8 *fb_1st_dcc;
    u8 *ff_2nd_dcc;
    u8 *fb_2nd_dcc;
    u8 *gains_drc_en;
    void *src_hdl;
    u32 dac_on_jiff;
    u32 dac_on_slience;
};
extern volatile struct icsd_anc ICSD_ANC;

struct icsd_anc_tool_data {
    int h_len;
    int yorderb;//int fb_yorder;
    int yorderf;//int ff_yorder;
    int yorderc;//int cmp_yorder;
    float *h_freq;
    float *data_out1;//float *hszpz_out_l;
    float *data_out2;//float *hpz_out_l;
    float *data_out3;//float *htarget_out_l;
    float *data_out4;//float *fb_fgq_l;
    float *data_out5;//float *ff_fgq_l;
    float *data_out6;//float *hszpz_out_r;
    float *data_out7;//float *hpz_out_r;
    float *data_out8;//float *htarget_out_r;
    float *data_out9;//float *fb_fgq_r;,
    float *data_out10;//float *ff_fgq_r;
    float *data_out11;//float *cmp_fgq_l;
    float *data_out12;//float *cmp_fgq_r;
    float *wz_temp;
    u8 result;
    u8 anc_err;
    u8 save_idx;
    u8 use_idx;
    u8 anc_combination;
};
extern volatile struct icsd_anc_tool_data *TOOL_DATA;

struct _icsd_autopick {
    float hold_time;
    float mean_vlds_thd_f_anc;
    float mean_vlds_thd_f_bypass;
    float mean_vlds_scratch_thd;//抓痒判断阈值
    float mean_vlds_flt_l_add;//mean_vlds_flt_l 递增值
    float mean_vlds_err_flt_thd;
    float mean_target_err_dither_offset;
    float crosszero_thd_0;
    float crosszero_thd_1;
    float ptr1_max_0;
    float ptr1_max_1;
    float mean_l_thd;//抖动阈值
};

enum {
    TFF_TFB = 0,
    TFF_DFB,
    DFF_TFB,
    DFF_DFB,
};

extern volatile u8 icsd_anc_function;
extern volatile u8 icsd_anc_contral;
extern volatile int user_train_state;
extern u32 train_time;
extern u16 icsd_time_out_hdl;
extern u8 icsd_anc_combination_test;
extern float autopick_thd[];


extern void icsd_anc_save_with_idx(u8 save_idx);
extern void audio_adc_mic_demo_close(void);
extern void icsd_anc_board_config();
//SDK调用的SDANC APP函数
extern void icsd_anc_vmdata_init(float (*FL)[25], float (*FR)[25], int *num);
extern void icsd_anc_dma_done();
extern void icsd_anc_end(audio_anc_t *param);
extern void icsd_anc_run();
extern void	icsd_anc_setparam();
extern void icsd_anc_timeout_handler();
extern void icsd_anc_init(audio_anc_t *param, u8 mode);
extern void icsd_anc_speak_mode_enter();
extern void icsd_anc_speak_mode_exit();
extern void icsd_anc_auto_pick_off();
extern u16  sys_timeout_add(void *priv, void (*func)(void *priv), u32 msec);

//SDANC APP调用的库函数
extern void icsd_anc_cmd_packet(s8 *data, u8 cmd);
extern void icsd_anc_tws_sync_cmd(void *_data, u16 len, bool rx);
extern void icsd_anc_version();
extern void icsd_anc_lib_init();
extern void icsd_anc_htarget_data_send_end();
extern void icsd_anc_mode_init(int tone_delay);
extern void icsd_anc_train_after_tone();
extern void icsd_anc_m2s_packet(s8 *data, u8 cmd);
extern void icsd_anc_s2m_packet(s8 *data, u8 cmd);
extern void icsd_anc_m2s_cb(void *_data, u16 len, bool rx);
extern void icsd_anc_s2m_cb(void *_data, u16 len, bool rx);
extern int  icsd_anc_src_output(void *hdl, void *data, int len);
extern void cal_wz(double *ab, float gain, int tap, float *freq, float fs, float *wz, int len);
extern void ff_fgq_2_aabb(double *iir_ab, float *ff_fgq);
extern void fb_fgq_2_aabb_vm(double *iir_ab, float *fb_fgq);
extern float icsd_anc_vmdata_match(float *_vmdata, float gain);
extern void icsd_anc_tonemode_start();
extern float icsd_anc_default_match();
extern void icsd_adc_mic_monitor_close();
//SDANC APP调用的SDK函数
extern void biquad2ab_double(float gain, float f, float q, double *a0, double *a1, double *a2, double *b0, double *b1, double *b2, int type);
extern int  tws_api_get_role(void);
extern int  tws_api_get_tws_state();
extern void anc_dma_on(u8 out_sel, int *buf, int len);
extern void audio_anc_fade2(int gain, u8 en, u8 step, u8 slow);
extern void anc_user_train_process(audio_anc_t *param);
extern void audio_anc_post_msg_user_train_run(void);
extern void audio_anc_post_msg_user_train_setparam(void);
extern void audio_anc_post_msg_user_train_timeout(void);
extern void audio_anc_post_msg_user_speak_mode_enter(void);
extern void audio_anc_post_msg_user_speak_mode_exit(void);
extern void audio_anc_post_msg_auto_pick_off(void);
extern unsigned int hw_fft_config(int N, int log2N, int is_same_addr, int is_ifft, int is_real);
extern void hw_fft_run(unsigned int fft_config, const int *in, int *out);
extern void anc_user_train_cb(u8 mode, u8 result);
//库调用的SDANC APP函数
extern void icsd_anc_set_alogm(void *_param, int alogm);
extern void icsd_anc_set_micgain(void *_param, int lff_gain, int lfb_gain);
extern void icsd_anc_fade(void *_param);
extern void icsd_anc_long_fade(void *_param);
extern void icsd_anc_user_train_dma_on(u8 out_sel, u32 len);
extern void icsd_anc_htarget_data_send();
extern void icsd_anc_fft(int *in, int *out);
extern u32  icsd_anc_get_role();
extern u32  icsd_anc_get_tws_state();
extern void icsd_anc_tws_m2s(u8 cmd);
extern void icsd_anc_tws_s2m(u8 cmd);
extern void icsd_anc_train_timeout();
extern void icsd_anc_src_init(int in_rate, int out_rate);
extern void icsd_anc_src_write(void *data, int len);
extern void icsd_audio_adc_mic_close();
extern u8   icsd_anc_get_save_idx();
extern void icsd_anc_vmdata_num_reset();
extern u8   icsd_anc_get_vmdata_num();
extern u8 	icsd_anc_min_diff_idx();
extern float icsd_anc_diff_mean();
extern void icsd_anc_fgq_printf(float *ptr);
extern void icsd_anc_aabb_printf(double *iir_ab);
extern void icsd_anc_vmdata_by_idx(double *ff_ab, float *ff_gain, u8 idx);
extern void icsd_anc_config_inf();
extern void icsd_anc_ear_record_printf();
extern u8   icsd_anc_tooldata_select_vmdata(float *ff_fgq);
extern void icsd_audio_adc_mic_open(u8 mic_idx, u8 gain, u16 sr, u8 mic_2_dac);


extern void icsd_anc_tonel_start();
extern void icsd_anc_toner_start();
extern void icsd_anc_pzl_start();
extern void icsd_anc_pzr_start();
extern void anc_dmadata_debug();
#endif
