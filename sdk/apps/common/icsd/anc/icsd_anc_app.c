#include "icsd_anc_app.h"
#include "classic/tws_api.h"
#include "board_config.h"
#include "asm/clock.h"
#include "adv_adaptive_noise_reduction.h"
#include "audio_anc.h"
#include "audio_src.h"

#if 0
#define icsd_anc_log	printf
#else
#define icsd_anc_log(...)
#endif/*log_en*/

#if TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN

u32 train_time;
struct icsd_anc_board_param *ANC_BOARD_PARAM = NULL;
volatile struct icsd_anc_tool_data *TOOL_DATA = NULL;
volatile struct icsd_anc_backup *CFG_BACKUP = NULL;
volatile struct icsd_anc ICSD_ANC;
volatile u8 icsd_anc_contral = 0;
volatile u8 icsd_anc_function = 0;
int *user_train_buf = 0;
volatile int user_train_state = NULL;
u16 icsd_time_out_hdl = NULL;
u8 icsd_anc_combination_test;

#define TWS_FUNC_ID_SDANC_M2S    TWS_FUNC_ID('I', 'C', 'M', 'S')
REGISTER_TWS_FUNC_STUB(icsd_anc_m2s) = {
    .func_id = TWS_FUNC_ID_SDANC_M2S,
    .func    = icsd_anc_m2s_cb,
};

#define TWS_FUNC_ID_SDANC_S2M    TWS_FUNC_ID('I', 'C', 'S', 'M')
REGISTER_TWS_FUNC_STUB(icsd_anc_s2m) = {
    .func_id = TWS_FUNC_ID_SDANC_S2M,
    .func    = icsd_anc_s2m_cb,
};

void icsd_anc_tws_m2s(u8 cmd)
{
    s8 data[16];
    icsd_anc_m2s_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 16, TWS_FUNC_ID_SDANC_M2S);
}

void icsd_anc_tws_s2m(u8 cmd)
{
    s8 data[16];
    icsd_anc_s2m_packet(data, cmd);
    int ret = tws_api_send_data_to_sibling(data, 16, TWS_FUNC_ID_SDANC_S2M);
}

void icsd_anc_fgq_printf(float *ptr)
{
    icsd_anc_log("icsd_anc_fgq\n");
    for (int i = 0; i < 25; i++) {
        icsd_anc_log("%d\n", (int)((*ptr++) * 1000));
    }
}

void icsd_anc_aabb_printf(double *iir_ab)
{
    icsd_anc_log("icsd_anc_aabb\n");
    for (int i = 0; i < ANC_BOARD_PARAM->fb_yorder; i++) {
        icsd_anc_log("AB:%d %d %d %d %d\n", (int)(iir_ab[i * 5] * 100000000),
                     (int)(iir_ab[i * 5 + 1] * 100000000),
                     (int)(iir_ab[i * 5 + 2] * 100000000),
                     (int)(iir_ab[i * 5 + 3] * 100000000),
                     (int)(iir_ab[i * 5 + 4] * 100000000));
    }
    put_buf(iir_ab, 8 * 40);
}

void icsd_anc_train_timeout()
{
    icsd_anc_log("icsd_anc_train_timeout\n");
    user_train_state &=	~ANC_USER_TRAIN_RUN;
    user_train_state |=	ANC_TRAIN_TIMEOUT;
    audio_anc_post_msg_user_train_timeout();
}

u32 icsd_anc_get_role()
{
    return tws_api_get_role();
}

u32 icsd_anc_get_tws_state()
{
    return tws_api_get_tws_state();
}


extern int audio_resample_hw_write(void *resample, void *data, int len);
extern void *audio_resample_hw_open(u8 channel, int in_rate, int out_rate, u8 type);
extern int audio_resample_hw_set_output_handler(void *resample, void *priv, int (*handler)(void *, void *, int));
void icsd_anc_src_init(int in_rate, int out_rate)
{
    ICSD_ANC.src_hdl = audio_resample_hw_open(1, in_rate, out_rate, 1);
    audio_resample_hw_set_output_handler(ICSD_ANC.src_hdl, 0, icsd_anc_src_output);
}

void icsd_anc_src_write(void *data, int len)
{
    icsd_anc_log("icsd_anc_src_write:%d\n", len);
    audio_resample_hw_write(ICSD_ANC.src_hdl, data, len);
}

void icsd_audio_adc_mic_close()
{
    audio_adc_mic_demo_close();
}

void icsd_anc_set_alogm(void *_param, int alogm)
{
    audio_anc_t *param = _param;
    param->gains.alogm = alogm;
}

void icsd_anc_set_micgain(void *_param, int lff_gain, int lfb_gain)
{
    //0 ~ 15
    audio_anc_t *param = _param;
    icsd_anc_log("mic gain set:%d %d-->%d %d\n", param->gains.l_ffmic_gain, param->gains.l_fbmic_gain, lff_gain, lfb_gain);
    param->gains.l_ffmic_gain = lff_gain;
    param->gains.l_fbmic_gain = lfb_gain;
    audio_anc_mic_management(param);
    audio_anc_mic_gain(param->mic_param, 0);
}

void icsd_anc_fft(int *in, int *out)
{
    u32 fft_config;
    fft_config = hw_fft_config(1024, 10, 1, 0, 1);
    hw_fft_run(fft_config, in, out);
}

void icsd_anc_fade(void *_param)
{
    audio_anc_t *param = _param;
    param->fade_lock = 0;
    audio_anc_fade(param->anc_fade_gain, param->anc_fade_en, 1);
}

void icsd_anc_long_fade(void *_param)
{
    audio_anc_t *param = _param;
    param->fade_lock = 0;
    audio_anc_fade2(0, param->anc_fade_en, 4, 0);	//增益先淡出到0
    os_time_dly(10);//10
}

void icsd_anc_user_train_dma_on(u8 out_sel, u32 len)
{
    user_train_state |=	ANC_TRAIN_DMA_ON;
    anc_dma_on(out_sel, user_train_buf, len);
}

void icsd_anc_dma_done()
{
    if (ICSD_ANC.adaptive_run_busy) {
        if (user_train_state & ANC_USER_TRAIN_DMA_EN) {
            if (user_train_state & ANC_USER_TRAIN_DMA_READY) {
                user_train_state &= ~ANC_USER_TRAIN_DMA_READY;
                anc_core_dma_stop();
                audio_anc_post_msg_user_train_run();
            } else {
                user_train_state |= ANC_USER_TRAIN_DMA_READY;
            }
        }
    }
}

void icsd_anc_init(audio_anc_t *param, u8 mode)
{
    user_train_state = 0;
    user_train_state |= ANC_TRAIN_TONE_PLAY;
    icsd_anc_function = 0;
    icsd_anc_combination_test = TEST_ANC_COMBINATION;

#if ANC_USER_TRAIN_TONE_MODE
    user_train_state |=	ANC_USER_TRAIN_TONEMODE;
#endif

#if ANC_AUTO_PICK_EN
    icsd_anc_function |= ANC_AUTO_PICK;
#endif

#if ANC_ADAPTIVE_CMP_EN
    icsd_anc_function |= ANC_ADAPTIVE_CMP;
#endif

    ICSD_ANC.param = param;
    ICSD_ANC.adaptive_run_busy = 1;
    ICSD_ANC.anc_fade_gain      = &param->anc_fade_gain;

    ICSD_ANC.gains_l_ffgain     = &param->gains.l_ffgain;
    ICSD_ANC.gains_l_fbgain     = &param->gains.l_fbgain;
    ICSD_ANC.gains_l_cmpgain    = &param->gains.l_cmpgain;
    ICSD_ANC.gains_r_ffgain     = &param->gains.r_ffgain;
    ICSD_ANC.gains_r_fbgain     = &param->gains.r_fbgain;
    ICSD_ANC.gains_r_cmpgain    = &param->gains.r_cmpgain;
    ICSD_ANC.lff_yorder         = &param->lff_yorder;
    ICSD_ANC.lfb_yorder         = &param->lfb_yorder;
    ICSD_ANC.lcmp_yorder        = &param->lcmp_yorder;
    ICSD_ANC.rff_yorder         = &param->rff_yorder;
    ICSD_ANC.rfb_yorder         = &param->rfb_yorder;
    ICSD_ANC.rcmp_yorder        = &param->rcmp_yorder;
    ICSD_ANC.lff_coeff          = &param->lff_coeff;
    ICSD_ANC.lfb_coeff      	= &param->lfb_coeff;
    ICSD_ANC.lcmp_coeff      	= &param->lcmp_coeff;
    ICSD_ANC.rff_coeff          = &param->rff_coeff;
    ICSD_ANC.rfb_coeff      	= &param->rfb_coeff;
    ICSD_ANC.rcmp_coeff      	= &param->rcmp_coeff;
    ICSD_ANC.gains_l_ffmic_gain = &param->gains.l_ffmic_gain;
    ICSD_ANC.gains_l_fbmic_gain = &param->gains.l_fbmic_gain;
    ICSD_ANC.gains_r_ffmic_gain = &param->gains.r_ffmic_gain;
    ICSD_ANC.gains_r_fbmic_gain = &param->gains.r_fbmic_gain;
    ICSD_ANC.gains_alogm        = &param->gains.alogm;
    ICSD_ANC.ff_1st_dcc         = &param->gains.ff_1st_dcc;
    ICSD_ANC.fb_1st_dcc         = &param->gains.fb_1st_dcc;
    ICSD_ANC.ff_2nd_dcc         = &param->gains.ff_2nd_dcc;
    ICSD_ANC.fb_2nd_dcc         = &param->gains.fb_2nd_dcc;
    ICSD_ANC.gains_drc_en       = &param->gains.drc_en;

    ICSD_ANC.mode = ANC_USER_TRAIN_MODE;
    ICSD_ANC.train_index = 0;
    ICSD_ANC.adaptive_run_busy = 1;

    if (mode == 1) {
        user_train_state |= ANC_USER_TRAIN_TOOL_DATA;
        if (TOOL_DATA == NULL) {
            TOOL_DATA = zalloc(sizeof(struct icsd_anc_tool_data));
        }
    }

    icsd_anc_board_config();
    user_train_state |=	ANC_USER_TRAIN_DMA_EN;
    icsd_anc_log("sd anc init:%d %x=========================================\n", param->gains.dac_gain, param->gains.gain_sign);
    TOOL_DATA->save_idx = 0xff;
    ANC_BOARD_PARAM->tool_ffgain_sign = 1;
    ANC_BOARD_PARAM->tool_fbgain_sign = 1;
    if (param->gains.gain_sign & ANCL_FF_SIGN) {
        ANC_BOARD_PARAM->tool_ffgain_sign = -1;
    }
    if (param->gains.gain_sign & ANCL_FB_SIGN) {
        ANC_BOARD_PARAM->tool_fbgain_sign = -1;
    }

    //金机曲线功能==================================
#if 1
    icsd_anc_log("ref en:%d----------------------\n", param->gains.adaptive_ref_en);
    struct icsd_fb_ref *fb_parm = NULL;
    struct icsd_ff_ref *ff_parm = NULL;
    if (param->gains.adaptive_ref_en) {
        icsd_anc_log("get ref data\n");
        anc_adaptive_fb_ref_data_get(&fb_parm);
        anc_adaptive_ff_ref_data_get(&ff_parm);
        ANC_BOARD_PARAM->m_value_l = fb_parm->m_value;
        ANC_BOARD_PARAM->sen_l = fb_parm->sen;
        ANC_BOARD_PARAM->in_q_l = fb_parm->in_q;
        icsd_anc_log("fb_parm:%d %d %d\n", (int)(fb_parm->m_value * 1000), (int)(fb_parm->sen * 1000), (int)(fb_parm->in_q * 1000));
    }
    if (ff_parm) {
        icsd_anc_log("use gold curve==========================================================\n");
        ANC_BOARD_PARAM->gold_curve_en = param->gains.adaptive_ref_en;
        ANC_BOARD_PARAM->mse_tar = ff_parm->db;
    }
#endif
    //==============================================

    icsd_anc_config_inf();
    icsd_anc_lib_init();
    //icsd_anc_set_micgain(param, ANC_USER_TRAIN_FF_MICGAIN, ANC_USER_TRAIN_FB_MICGAIN);
    icsd_anc_set_alogm(param, ANC_USER_TRAIN_SR_SEL);
    icsd_anc_mode_init(TONE_DELAY);

}

void icsd_anc_init_cmd()
{
    icsd_anc_log("icsd_anc_init_cmd\n");
    extern void audio_anc_post_msg_user_train_init(u8 mode);
    audio_anc_post_msg_user_train_init(ANC_ON);
}

void icsd_anctone_dacon(u32 slience_frames, u16 sample_rate)
{
    if (user_train_state & ANC_TRAIN_TONE_PLAY) {
        if (ANC_BOARD_PARAM == NULL) {
            ANC_BOARD_PARAM = zalloc(sizeof(struct icsd_anc_board_param));
        }
        icsd_anc_log("icsd_anctone_dacon 1:%d %d %d\n", jiffies, slience_frames, sample_rate);
        user_train_state |= ANC_TONE_DAC_ON;
        user_train_state &= ~ANC_TRAIN_TONE_PLAY;
        u32 slience_ms = 0;
        if ((slience_frames != 0) && (sample_rate != 0)) {
            slience_ms = (slience_frames * 1000 / sample_rate) + 1;
            icsd_anc_log("icsd_anctone_dacon 2:%d %d %d\n", slience_frames, sample_rate, slience_ms);
        }
        ANC_BOARD_PARAM->tone_jiff = jiffies + (slience_ms / 10) + 1;
        ICSD_ANC.dac_on_jiff = jiffies;
        ICSD_ANC.dac_on_slience = slience_ms;
        if (user_train_state & ANC_TONE_ANC_READY) {

            icsd_anc_log("dac on set tonemode start timeout:%d %d\n", slience_ms, ANC_BOARD_PARAM->tone_jiff);
            switch (ANC_BOARD_PARAM->mode) {
            case HEADSET_TONES_MODE:
                sys_hi_timeout_add(NULL, icsd_anc_tonel_start, slience_ms + TONEL_DELAY);
                sys_hi_timeout_add(NULL, icsd_anc_toner_start, slience_ms + TONER_DELAY);
                sys_hi_timeout_add(NULL, icsd_anc_pzl_start, slience_ms + PZL_DELAY);
                sys_hi_timeout_add(NULL, icsd_anc_pzr_start, slience_ms + PZR_DELAY);
                break;
            default:
                sys_hi_timeout_add(NULL, icsd_anc_tonemode_start, slience_ms + TONE_DELAY);
                break;
            }
        } else {
            icsd_anc_log("dac on wait anc ready\n");
        }

    }
}

void icsd_anc_auto_pick_off()
{
    icsd_anc_log("icsd_anc_auto_pick_off\n");
    icsd_adc_mic_monitor_close();
}

void icsd_anc_tone_play_start()
{
    audio_anc_post_msg_auto_pick_off();
#if ANC_USER_TRAIN_TONE_MODE
    icsd_anc_log("icsd_anc_tone_play_start\n");
    user_train_state |= ANC_TRAIN_TONE_PLAY;
    icsd_anc_init_cmd();
    os_time_dly(20);
#endif
}

u8 icsd_anc_adaptive_busy_flag_get(void)
{
    return ICSD_ANC.adaptive_run_busy;
}

#if ANC_EAR_RECORD_EN
/* float vmdata_FL[EAR_RECORD_MAX][25]; */
/* float vmdata_FR[EAR_RECORD_MAX][25]; */
float (*vmdata_FL)[25] = NULL;
float (*vmdata_FR)[25] = NULL;
int *vmdata_num;

void icsd_anc_vmdata_init(float (*FL)[25], float (*FR)[25], int *num)
{
    //读取耳道记忆 VM数据地址
    vmdata_FL = FL;
    vmdata_FR = FR;
    vmdata_num = num;
    /* icsd_anc_log("vmdata_num %d\n", *vmdata_num); */
    /* put_buf((u8*)vmdata_FL, EAR_RECORD_MAX*25*4); */
    /* put_buf((u8*)vmdata_FR, EAR_RECORD_MAX*25*4); */
}

void icsd_anc_vmdata_num_reset()
{
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    *vmdata_num = 0;
    memset(vmdata_FL, 0, EAR_RECORD_MAX * 4 * 25);
}

u8 icsd_anc_get_vmdata_num()
{
    return *vmdata_num;
}

void icsd_anc_ear_record_printf()
{
    int i;
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    icsd_anc_log("vmdata_num:%d\n", *vmdata_num);
    for (i = 0; i < EAR_RECORD_MAX; i++) {
        icsd_anc_log("vmdata_FL[%d]\n", i);
        icsd_anc_fgq_printf(&vmdata_FL[i][0]);
    }
}

u8 icsd_anc_max_diff_idx()
{
    u8 max_diff_idx = 0;
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return max_diff_idx;
    }
    if (*vmdata_num > 0) {
        float diff;
        float max_diff = icsd_anc_vmdata_match(&vmdata_FL[0][0], vmdata_FL[0][0]);
        max_diff_idx = 0;
        for (int i = 1; i < *vmdata_num; i++) {
            diff = icsd_anc_vmdata_match(&vmdata_FL[i][0], vmdata_FL[i][0]);
            if (diff > max_diff) {
                max_diff = diff;
                max_diff_idx = i;
            }
        }
    }
    icsd_anc_log("max diff idx:%d\n", max_diff_idx);
    return max_diff_idx;
}

u8 icsd_anc_min_diff_idx()
{
    float diff;
    float min_diff;
    u8 min_diff_idx = 0xff;

    if ((!vmdata_FL) || (!vmdata_FR)) {
        return min_diff_idx;
    }
    if (*vmdata_num > 0) {
        min_diff = icsd_anc_vmdata_match(&vmdata_FL[0][0], vmdata_FL[0][0]);
        min_diff_idx = 0;
        icsd_anc_log("DIFF[0]:%d\n", (int)(min_diff * 1000));
        for (int i = 1; i < *vmdata_num; i++) {
            diff = icsd_anc_vmdata_match(&vmdata_FL[i][0], vmdata_FL[i][0]);
            if (diff < min_diff) {
                min_diff = diff;
                min_diff_idx = i;
            }
            icsd_anc_log("DIFF[%d]:%d %d %d\n", i, (int)(diff * 1000), min_diff_idx, (int)(min_diff * 1000));
        }
        diff = icsd_anc_default_match();
        icsd_anc_log("default diff:%d vmmin diff:%d\n", (int)(diff * 100), (int)(min_diff * 100));
        if (diff < min_diff) {
            icsd_anc_log("use default\n");
            min_diff_idx = 0xff;
        } else {
            icsd_anc_log("select vmdata_FL[%d]\n", min_diff_idx);
        }
    }
    return min_diff_idx;
}

float icsd_anc_diff_mean()
{
    float diff;
    float diff_sum = 0;

    if ((!vmdata_FL) || (!vmdata_FR)) {
        return 0;
    }

    if (*vmdata_num > 0) {
        for (int i = 0; i < *vmdata_num; i++) {
            diff = icsd_anc_vmdata_match(&vmdata_FL[i][0], vmdata_FL[i][0]);
            diff_sum += diff;
            icsd_anc_log("vm sum:%d %d %d\n", (int)(diff_sum * 100), (int)(diff * 100), *vmdata_num);
        }
    }
    diff = icsd_anc_default_match();
    diff_sum += diff;
    icsd_anc_log("df sum:%d %d %d\n", (int)(diff_sum * 100), (int)(diff * 100), *vmdata_num);
    float diff_mean = diff_sum / (*vmdata_num + 1);
    icsd_anc_log("diff mean:%d\n", (int)(diff_mean * 100));
    return diff_mean;
}

u8 icsd_anc_get_save_idx()
{
    u8 save_idx = 0;
    if (*vmdata_num < EAR_RECORD_MAX) {
        save_idx = *vmdata_num;
    } else {
        save_idx = icsd_anc_max_diff_idx();
    }
    icsd_anc_log("icsd_anc_get_save_idx:%d\n", save_idx);
    return save_idx;
}

void icsd_anc_save_with_idx(u8 save_idx)
{
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    if (save_idx > *vmdata_num) {
        return;
    }
    if ((save_idx >= 0) && (save_idx < EAR_RECORD_MAX)) {
        if (*vmdata_num < EAR_RECORD_MAX) {
            (*vmdata_num)++;
        }
        icsd_anc_log("icsd_anc_save_with_idx:%d %d=====================================================\n", save_idx, *vmdata_num);
        switch (ANC_BOARD_PARAM->mode) {
        case HEADSET_TONE_BYPASS_MODE:
        case HEADSET_TONES_MODE:
        case HEADSET_BYPASS_MODE:
            memcpy(&vmdata_FL[save_idx][0], TOOL_DATA->data_out5, 4 * 25);
            memcpy(&vmdata_FR[save_idx][0], TOOL_DATA->data_out10, 4 * 25);
            break;
        default:
            memcpy(&vmdata_FL[save_idx][0], TOOL_DATA->data_out5, 4 * 25);
            break;
        }
#if	TEST_EAR_RECORD_EN
        icsd_anc_log("EAR RECORD SAVE:%d\n", save_idx);
        icsd_anc_ear_record_printf();
#endif
    }
}

void icsd_anc_vmdata_by_idx(double *ff_ab, float *ff_gain, u8 idx)
{
    u8 num = icsd_anc_get_vmdata_num();
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return;
    }
    if ((num == 0) || (idx == 0xff) || ((num - 1) < idx)) {
        return;
    }
#if	TEST_EAR_RECORD_EN
    icsd_anc_log("EAR RECORD USED RECORD\n");
    icsd_anc_fgq_printf(&vmdata_FL[idx][0]);
#endif
    ff_fgq_2_aabb(ff_ab, &vmdata_FL[idx][0]);
    *ff_gain = vmdata_FL[idx][0];
}

u8 icsd_anc_tooldata_select_vmdata(float *ff_fgq)
{

    icsd_anc_log("train time vm select start:%d\n", (jiffies - train_time) * 10);
    u8 min_idx = icsd_anc_min_diff_idx();
    extern void icsd_set_min_idx(u8 minidx);
    icsd_set_min_idx(min_idx);
    u8 num = icsd_anc_get_vmdata_num();
    if ((!vmdata_FL) || (!vmdata_FR)) {
        return 0;
    }
    //icsd_anc_log("icsd_anc_tooldata_select_vmdata:%d %d\n", num, min_idx);
    if ((num == 0) || (min_idx == 0xff) || ((num - 1) < min_idx)) {
        icsd_anc_log("use default\n");
        return 0;
    }
    memcpy(ff_fgq, &vmdata_FL[min_idx][0], 4 * 25);

#if	TEST_EAR_RECORD_EN
    icsd_anc_log("EAR RECORD TOOL_DATA RECORD\n");
    icsd_anc_fgq_printf(ff_fgq);
#endif

    icsd_anc_log("train time vm select end:%d\n", (jiffies - train_time) * 10);
    icsd_anc_log("use record:%d\n", min_idx);
    return 1;
}
#else
u8 icsd_anc_min_diff_idx()
{
    return 0xff;
}
u8 icsd_anc_get_save_idx()
{
    return 0xff;
}
u8 icsd_anc_get_vmdata_num()
{
    return 0;
}
void icsd_anc_vmdata_num_reset()
{}
void icsd_anc_save_with_idx(u8 save_idx)
{}
void icsd_anc_vmdata_by_idx(double *ff_ab, float *ff_gain, u8 idx)
{}
void icsd_anc_ear_record_printf()
{}
u8 icsd_anc_tooldata_select_vmdata(float *ff_fgq)
{}
float icsd_anc_diff_mean()
{
    return 0;
}
#endif




#endif/*TCFG_AUDIO_ANC_EAR_ADAPTIVE_EN*/





