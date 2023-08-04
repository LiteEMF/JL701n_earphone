#include "board_config.h"
#include "icsd_anc_app.h"
#include "tone_player.h"

#if 0
#define icsd_board_log printf
#else
#define icsd_board_log(...)
#endif/*log_en*/

#if TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN
u8 anc_train_result = 0;

float autopick_thd[] = {
#if ADAPTIVE_CLIENT_BOARD == HT03_HYBRID_6G
    ANC_AUTO_PICK_HOLDTIME * 100,
    8.0,   //float mean_vlds_thd_f_anc;
    3.0,   //float mean_vlds_thd_f_bypass;
    9.0,   //float mean_vlds_scratch_thd;//抓痒判断阈值
    0.1,   //float mean_vlds_flt_l_add;//mean_vlds_flt_l 递增值
    90.0,  //float mean_vlds_err_flt_thd;
    3.0,   //float mean_target_err_dither_offset;
    4.0,   //float crosszero_thd_0;
    12.0,  //float crosszero_thd_1;
    1200.0,//float ptr1_max_0;
    3000.0,//float ptr1_max_1;
    130.0,  //float mean_l_thd;//抖动阈值
#elif ADAPTIVE_CLIENT_BOARD == G02_HYBRID_6G
    ANC_AUTO_PICK_HOLDTIME * 100,
    8.0,   //float mean_vlds_thd_f_anc;
    3.0,   //float mean_vlds_thd_f_bypass;
    9.0,   //float mean_vlds_scratch_thd;//抓痒判断阈值
    0.1,   //float mean_vlds_flt_l_add;//mean_vlds_flt_l 递增值
    90.0,  //float mean_vlds_err_flt_thd;
    3.0,   //float mean_target_err_dither_offset;
    4.0,   //float crosszero_thd_0;
    12.0,  //float crosszero_thd_1;
    1200.0,//float ptr1_max_0;
    3000.0,//float ptr1_max_1;
    130.0,  //float mean_l_thd;//抖动阈值
#else
    ANC_AUTO_PICK_HOLDTIME * 100,
    8.0,   //float mean_vlds_thd_f_anc;
    3.0,   //float mean_vlds_thd_f_bypass;
    9.0,   //float mean_vlds_scratch_thd;//抓痒判断阈值
    0.1,   //float mean_vlds_flt_l_add;//mean_vlds_flt_l 递增值
    90.0,  //float mean_vlds_err_flt_thd;
    3.0,   //float mean_target_err_dither_offset;
    4.0,   //float crosszero_thd_0;
    12.0,  //float crosszero_thd_1;
    1200.0,//float ptr1_max_0;
    3000.0,//float ptr1_max_1;
    130.0,  //float mean_l_thd;//抖动阈值
#endif
};

void icsd_anc_board_config()
{
    if (ANC_BOARD_PARAM == NULL) {
        ANC_BOARD_PARAM = zalloc(sizeof(struct icsd_anc_board_param));
    }

    ANC_BOARD_PARAM->ff_yorder  = ANC_ADAPTIVE_FF_ORDER;
    ANC_BOARD_PARAM->fb_yorder  = ANC_ADAPTIVE_FB_ORDER;
    ANC_BOARD_PARAM->cmp_yorder = ANC_ADAPTIVE_CMP_ORDER;
    ANC_BOARD_PARAM->tool_target_sign = 1;

#if ADAPTIVE_CLIENT_BOARD == HT03_HYBRID_6G
#define PROJECT_098                 0
#define PROJECT_HT03                1
#define BOARD_PROJECT         		PROJECT_HT03
#if BOARD_PROJECT == PROJECT_HT03
    icsd_board_log("------------HT03_HYBRID_6G----------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3
    extern const float HT03_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)HT03_DATA;
    ANC_BOARD_PARAM->m_value_l = 120.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 15.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.4;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 11.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 18;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 1;
    ANC_BOARD_PARAM->cmp_abs_en = 0;
    ANC_BOARD_PARAM->idx_begin = 18;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 3;
    ANC_BOARD_PARAM->gain_min_offset = 11;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 2;
    ANC_BOARD_PARAM->tff_sign = 1;
    ANC_BOARD_PARAM->tfb_sign = -1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 0.6; //0.6

    ANC_BOARD_PARAM->tool_target_sign = -1;
#else
    icsd_board_log("------------ 098 -----------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3

#endif

#elif ADAPTIVE_CLIENT_BOARD == G02_HYBRID_6G
    icsd_board_log("------------ G02 -----------------\n");
#define DEFAULT_FF_GAIN				4
#define DEFAULT_FB_GAIN				1
    extern const float G02_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)G02_DATA;
    ANC_BOARD_PARAM->m_value_l = 135.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l = 12.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l = 0.6;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 22.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 10;
    ANC_BOARD_PARAM->gain_a_param = 12.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 1;
    ANC_BOARD_PARAM->idx_begin = 25;
    ANC_BOARD_PARAM->fb_w2r = 1200;
    ANC_BOARD_PARAM->FB_NFIX = 4;
    ANC_BOARD_PARAM->gain_min_offset = 22;
    ANC_BOARD_PARAM->gain_max_offset = 32;
    ANC_BOARD_PARAM->minvld = -30.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = -1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = -1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 0.6;//1.6;

    ANC_BOARD_PARAM->tool_target_sign = 1;

#elif ADAPTIVE_CLIENT_BOARD == ANC05_HYBRID
    icsd_board_log("------------ ANC05_HYBRID -----------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3
    extern const float ANC05_L_DATA[];
    extern const float ANC05_R_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)ANC05_L_DATA;
    ANC_BOARD_PARAM->anc_data_r = (void *)ANC05_R_DATA;
    ANC_BOARD_PARAM->m_value_l  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l      = 12.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->m_value_r  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_r      = 12.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_r     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 13.0 - 18.0;
    ANC_BOARD_PARAM->sen_offset_r = 13.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 18;
    ANC_BOARD_PARAM->ff_target_fix_num_r = 18;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 0;
    ANC_BOARD_PARAM->idx_begin = 18;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 3;
    ANC_BOARD_PARAM->gain_min_offset = 11;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 2.0;

    ANC_BOARD_PARAM->tool_target_sign = -1;
#elif ADAPTIVE_CLIENT_BOARD == H3_HYBRID
    icsd_board_log("------------ H3_HYBRID -----------------\n");
#define DEFAULT_FF_GAIN				3
#define DEFAULT_FB_GAIN				3
    extern const float H3_L_DATA[];
    extern const float H3_R_DATA[];
    ANC_BOARD_PARAM->anc_data_l = (void *)H3_L_DATA;
    ANC_BOARD_PARAM->anc_data_r = (void *)H3_R_DATA;
    ANC_BOARD_PARAM->m_value_l  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_l      = 12.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_l     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->m_value_r  = 220.0;		//80 ~ 200   最深点位置中心值
    ANC_BOARD_PARAM->sen_r      = 12.0;     		//12 ~ 22    最深点深度
    ANC_BOARD_PARAM->in_q_r     = 0.3;  		//0.4 ~ 1.2  最深点降噪宽度
    ANC_BOARD_PARAM->sen_offset_l = 13.0 - 18.0;
    ANC_BOARD_PARAM->sen_offset_r = 13.0 - 18.0;
    ANC_BOARD_PARAM->ff_target_fix_num_l = 18;
    ANC_BOARD_PARAM->ff_target_fix_num_r = 18;
    ANC_BOARD_PARAM->gain_a_param = 25.0;
    ANC_BOARD_PARAM->gain_a_en = 0;
    ANC_BOARD_PARAM->cmp_abs_en = 0;
    ANC_BOARD_PARAM->idx_begin = 18;
    ANC_BOARD_PARAM->fb_w2r = 1000;
    ANC_BOARD_PARAM->FB_NFIX = 3;
    ANC_BOARD_PARAM->gain_min_offset = 11;
    ANC_BOARD_PARAM->gain_max_offset = 18;
    ANC_BOARD_PARAM->minvld = -10.0;

    ANC_BOARD_PARAM->bypass_max_times = BYPASS_MAXTIME;
    ANC_BOARD_PARAM->pz_max_times = PZ_MAXTIME;
    ANC_BOARD_PARAM->jt_en = 1;
    ANC_BOARD_PARAM->tff_sign = -1;
    ANC_BOARD_PARAM->tfb_sign = 1;
    ANC_BOARD_PARAM->bff_sign = 1;
    ANC_BOARD_PARAM->bfb_sign = 1;
    ANC_BOARD_PARAM->bypass_sign = -1;
    ANC_BOARD_PARAM->bypass_volume = 2.0;

    ANC_BOARD_PARAM->tool_target_sign = -1;

#else

#endif

    if (ANC_BOARD_PARAM->pz_max_times < 3) {
        ANC_BOARD_PARAM->pz_max_times = 3;
    }

    if (ANC_BOARD_PARAM->bypass_max_times < 2) {
        ANC_BOARD_PARAM->pz_max_times = 2;
    }

    ANC_BOARD_PARAM->mode = ICSD_ANC_MODE;
    if (ANC_DFF_AFB_EN) {
        icsd_board_log("ANC_DFF_AFB_EN ON\n");
        ANC_BOARD_PARAM->FF_FB_EN |= BIT(7);
    }
    if (ANC_AFF_DFB_EN) {
        icsd_board_log("ANC_AFF_DFB_EN ON\n");
        ANC_BOARD_PARAM->FF_FB_EN |= BIT(6);
    }
    ANC_BOARD_PARAM->default_ff_gain = DEFAULT_FF_GAIN;
    ANC_BOARD_PARAM->default_fb_gain = DEFAULT_FB_GAIN;

    if (TCFG_AUDIO_DAC_MODE == DAC_MODE_H1_DIFF) {
        user_train_state |= ANC_DAC_MODE_H1_DIFF;
    } else {
        user_train_state &= ~ANC_DAC_MODE_H1_DIFF;
    }
}

void icsd_anc_config_inf()
{
    icsd_board_log("\n=============ANC COMFIG INF================\n");

    icsd_anc_version();
    switch (ICSD_ANC_MODE) {
    case TWS_TONE_BYPASS_MODE:
        icsd_board_log("TWS_TONE_BYPASS_MODE\n");
        break;
    case TWS_BYPASS_MODE:
        icsd_board_log("TWS_BYPASS_MODE\n");
        break;
    case HEADSET_TONE_BYPASS_MODE:
        icsd_board_log("HEADSET_TONE_BYPASS_MODE\n");
        break;
    case HEADSET_TONES_MODE:
        icsd_board_log("HEADSET_TONES_MODE\n");
        break;
    case HEADSET_BYPASS_MODE:
        icsd_board_log("HEADSET_BYPASS_MODE\n");
        break;
    default:
        icsd_board_log("NEW MODE\n");
        break;
    }
    icsd_board_log("TEST_ANC_COMBINATION:%d\n", TEST_ANC_COMBINATION);
    icsd_board_log("ANC_ADAPTIVE_CMP_EN:%d\n", ANC_ADAPTIVE_CMP_EN);
    icsd_board_log("ANC_AUTO_PICK_EN:%d\n", ANC_AUTO_PICK_EN);
    icsd_board_log("ANC_EAR_RECORD_EN:%d\n", ANC_EAR_RECORD_EN);
    icsd_board_log("TEST_EAR_RECORD_EN:%d\n", TEST_EAR_RECORD_EN);
    icsd_board_log("BYPASS_TIMES MIN:2 MAX:%d\n", BYPASS_MAXTIME);
    icsd_board_log("PZ_TIMES MIN:3 MAX:%d\n", PZ_MAXTIME);
    icsd_board_log("\n");

#if	TEST_EAR_RECORD_EN
    icsd_board_log("EAR RECORD CURRENT RECORD\n");
    icsd_anc_ear_record_icsd_board_log();
#endif
}

//训练结果获取API，用于通知应用层当前使用自适应参数还是默认参数
u8 icsd_anc_train_result_get(struct icsd_anc_tool_data *TOOL_DATA)
{
    u8 result = 0;
    if (TOOL_DATA) {
        switch (TOOL_DATA->anc_combination) {
        case TFF_TFB://使用自适应FF，自适应FB
            result = ANC_ADAPTIVE_RESULT_LFF | ANC_ADAPTIVE_RESULT_LFB | ANC_ADAPTIVE_RESULT_RFF | ANC_ADAPTIVE_RESULT_RFB;
            break;
        case TFF_DFB://使用自适应FF，默认FB
            result = ANC_ADAPTIVE_RESULT_LFF | ANC_ADAPTIVE_RESULT_RFF;
            break;
        case DFF_TFB://result:1 使用记忆FF，自适应FB   :3 使用默认FF，自适应FB
            result = ANC_ADAPTIVE_RESULT_LFB | ANC_ADAPTIVE_RESULT_RFB;
            break;
        case DFF_DFB://result:1 使用记忆FF，默认FB  :3 使用默认FF，默认FB
        default:
            break;
        }
        if (TOOL_DATA->result == ANC_USE_RECORD) {
            result |= (ANC_ADAPTIVE_RESULT_LFF | ANC_ADAPTIVE_RESULT_RFF);
        }
    }
    return result;
}

extern int tone_play_index(u8 index, u8 preemption);
void icsd_anc_htarget_data_end(u8 result)
{
    user_train_state &= ~ANC_USER_TRAIN_TOOL_DATA;
    ICSD_ANC.adaptive_run_busy = 0;
    /*
       训练结果播放提示音
       result = 1 训练成功
       result = 0 训练失败
    */
    anc_train_result = result;
    icsd_board_log("RESULT:%d %d========================================\n", result, TOOL_DATA->anc_combination);
#if ANC_DEVELOPER_MODE_EN
    switch (TOOL_DATA->anc_combination) {
    case TFF_TFB://使用自适应FF，自适应FB
        break;
    case TFF_DFB://使用自适应FF，默认FB
        tone_play_index(IDEX_TONE_NUM_2, 0);
        break;
    case DFF_TFB://result:1 使用记忆FF，自适应FB   :3 使用默认FF，自适应FB
        tone_play_index(IDEX_TONE_NUM_1, 0);
        break;
    case DFF_DFB://result:1 使用记忆FF，默认FB  :3 使用默认FF，默认FB
        tone_play_index(IDEX_TONE_NUM_0, 0);
        break;
    default:
        break;
    }
#endif/*ANC_DEVELOPER_MODE_EN*/

#if (RCSP_ADV_EN && RCSP_ADV_ANC_VOICE && RCSP_ADV_ADAPTIVE_NOISE_REDUCTION)
    extern void set_adaptive_noise_reduction_reset_callback(u8 result);
    set_adaptive_noise_reduction_reset_callback(result);
#endif

    icsd_board_log("train time tool data end:%d\n", (jiffies - train_time) * 10);
    icsd_anc_htarget_data_send_end();
}

extern void audio_anc_adaptive_data_packet(struct icsd_anc_tool_data *TOOL_DATA);
void icsd_anc_htarget_data_send()
{
    icsd_board_log("train time tool data start:%d\n", (jiffies - train_time) * 10);
    //TOOL_DATA->result  0:失败使用默认参数   1:耳道记忆参数  2:成功
    if (TOOL_DATA->result == 2) {
        //成功时进行耳道记忆
        icsd_anc_save_with_idx(TOOL_DATA->save_idx);
    }
    audio_anc_adaptive_data_packet(TOOL_DATA);
    icsd_anc_htarget_data_end(TOOL_DATA->result);
}

void icsd_anc_end(audio_anc_t *param)
{
    icsd_board_log("train time finish:%dms\n", (jiffies - train_time) * 10);
    user_train_state &= ~ANC_USER_TRAIN_DMA_EN;
    param->fade_lock = 0;
    sys_timeout_del(icsd_time_out_hdl);
    anc_user_train_cb(ANC_ON, anc_train_result);
}

void anc_user_train_tone_play_cb()
{
    icsd_board_log("anc_user_train_tone_play_cb:%d========================================\n", jiffies);
    train_time = jiffies;
#if ANC_USER_TRAIN_EN

#if ANC_USER_TRAIN_TONE_MODE
    icsd_board_log("TONE MODE tone play cb~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    if (user_train_state & ANC_TRAIN_TONE_FIRST) {
        user_train_state |= ANC_TRAIN_TONE_END;
    } else {
        icsd_board_log("second train at cb~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        icsd_anc_train_after_tone();
    }
#else
    extern void audio_anc_post_msg_user_train_init(u8 mode);
    audio_anc_post_msg_user_train_init(ANC_ON);
#endif/*ANC_USER_TRAIN_TONE_MODE*/

#else
    extern void anc_user_train_cb(u8 mode, u8 result);
    anc_user_train_cb(ANC_ON, anc_train_result);
#endif/*ANC_USER_TRAIN_EN*/
}

#endif/*TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN*/
