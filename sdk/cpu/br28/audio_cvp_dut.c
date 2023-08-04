/*
 ****************************************************************************
 *							Audio CVP DUT
 *
 *
 ****************************************************************************
 */
#include "audio_cvp_dut.h"
#include "online_db_deal.h"
#include "system/includes.h"
#include "app_config.h"
#include "audio_adc.h"
#include "anc.h"
#include "aec_user.h"

#if 0
#define cvp_dut_log	printf
#else
#define cvp_dut_log(...)
#endif

#define CVP_DUT_BIG_ENDDIAN 		1		//大端数据输出
#define CVP_DUT_LITTLE_ENDDIAN		2		//小端数据输出
#define CVP_DUT_OUTPUT_WAY			CVP_DUT_LITTLE_ENDDIAN

#if TCFG_AUDIO_CVP_DUT_ENABLE
static cvp_dut_t *cvp_dut_hdl = NULL;
static int cvp_dut_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size);
int cvp_dut_spp_rx_packet(u8 *dat, u8 len);

#if TCFG_AUDIO_TRIPLE_MIC_ENABLE
extern void audio_aec_output_sel(u8 sel, u8 agc);
#endif/*TCFG_AUDIO_TRIPLE_MIC_ENABLE*/

extern void sys_enter_soft_poweroff(void *priv);
extern int audio_aec_toggle_set(u8 toggle);

/*未经过算法处理，主要用于分析ANC MIC的频响*/
int cvp_dut_bypass_mic_ch_sel(u8 mic_num)
{
    if (cvp_dut_hdl) {
        switch (mic_num) {
        case A_MIC0:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_0;
            break;
        case A_MIC1:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_1;
            break;
        case A_MIC2:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_2;
            break;
        case A_MIC3:
            cvp_dut_hdl->mic_ch_sel = AUDIO_ADC_MIC_3;
            break;
        default:
            break;
        }
    }
    return 0;
}

u8 cvp_dut_mic_ch_get(void)
{
    if (cvp_dut_hdl) {
        return cvp_dut_hdl->mic_ch_sel;
    }
    return 0;
}

u8 cvp_dut_mode_get(void)
{
    if (cvp_dut_hdl) {
        return cvp_dut_hdl->mode;
    }
    return 0;
}

void cvp_dut_mode_set(u8 mode)
{
    if (cvp_dut_hdl) {
        cvp_dut_hdl->mode = mode;
    }
}

int cvp_dut_event_deal(u8 *dat)
{
    if (cvp_dut_hdl) {
        switch (dat[3]) {
#if TCFG_AUDIO_DUAL_MIC_ENABLE
//双麦ENC DUT 事件处理
        case CVP_DUT_DMS_MASTER_MIC:
            cvp_dut_log("CMD:CVP_DUT_DMS_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_MASTER, 0);
            break;
        case CVP_DUT_DMS_SLAVE_MIC:
            cvp_dut_log("CMD:CVP_DUT_DMS_SLAVE_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_SLAVE, 0);
            break;
        case CVP_DUT_DMS_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_DMS_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(DMS_OUTPUT_SEL_DEFAULT, 1);
            break;
#elif TCFG_AUDIO_TRIPLE_MIC_ENABLE
        case CVP_DUT_3MIC_MASTER_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(1, 0);
            break;
        case CVP_DUT_3MIC_SLAVE_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_SLAVE_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(2, 0);
            break;
        case CVP_DUT_3MIC_FB_MIC:
            cvp_dut_log("CMD:CVP_DUT_3MIC_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(3, 0);
            break;
        case CVP_DUT_3MIC_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_3MIC_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_output_sel(0, 0);
            break;
#else
//单麦SMS/DNS DUT 事件处理
        case CVP_DUT_DMS_MASTER_MIC:
            cvp_dut_log("CMD:SMS/CVP_DUT_DMS_MASTER_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(0);
            break;
        case CVP_DUT_DMS_OPEN_ALGORITHM:
            cvp_dut_log("CMD:SMS/CVP_DUT_DMS_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(1);
            break;
#endif/*TCFG_AUDIO_DUAL_MIC_ENABLE*/

#if TCFG_AUDIO_ANC_ENABLE
        case CVP_DUT_FF_MIC:	//TWS_FF_MIC测试 或者 头戴式FF_L_MIC测试
            cvp_dut_log("CMD:CVP_DUT_FF_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH & ANC_L_CH) {
                cvp_dut_bypass_mic_ch_sel(ANCL_FF_MIC);
            } else {
                cvp_dut_bypass_mic_ch_sel(ANCR_FF_MIC);
            }
            break;
        case CVP_DUT_FB_MIC:	//TWS_FB_MIC测试 或者 头戴式FB_L_MIC测试
            cvp_dut_log("CMD:CVP_DUT_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH & ANC_L_CH) {
                cvp_dut_bypass_mic_ch_sel(ANCL_FB_MIC);
            } else {
                cvp_dut_bypass_mic_ch_sel(ANCR_FB_MIC);
            }
            break;
        case CVP_DUT_HEAD_PHONE_R_FF_MIC:	//头戴式FF_R_MIC测试
            cvp_dut_log("CMD:CVP_DUT_HEAD_PHONE_R_FF_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH == (ANC_L_CH | ANC_R_CH)) {
                cvp_dut_bypass_mic_ch_sel(ANCR_FF_MIC);
            }
            break;
        case CVP_DUT_HEAD_PHONE_R_FB_MIC:	//头戴式FB_R_MIC测试
            cvp_dut_log("CMD:CVP_DUT_HEAD_PHONE_R_FB_MIC\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_BYPASS;
            if (ANC_CH == (ANC_L_CH | ANC_R_CH)) {
                cvp_dut_bypass_mic_ch_sel(ANCR_FB_MIC);
            }
            break;
        case CVP_DUT_MODE_ALGORITHM_SET:				//CVP恢复正常模式
            cvp_dut_log("CMD:CVP_DUT_RESUME\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            break;
#endif/*TCFG_AUDIO_ANC_ENABLE*/

//JL自研算法指令API
#if !TCFG_CVP_DEVELOP_ENABLE
        case CVP_DUT_NS_OPEN:
            cvp_dut_log("CMD:CVP_DUT_NS_OPEN\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_cvp_ioctl(CVP_NS_SWITCH, 1, NULL);	//降噪关
            break;
        case CVP_DUT_NS_CLOSE:
            cvp_dut_log("CMD:CVP_DUT_NS_CLOSE\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_cvp_ioctl(CVP_NS_SWITCH, 0, NULL); //降噪开
            break;
#endif/*TCFG_CVP_DEVELOP_ENABLE*/

        case CVP_DUT_OPEN_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_OPEN_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(1);
            break;
        case CVP_DUT_CLOSE_ALGORITHM:
            cvp_dut_log("CMD:CVP_DUT_CLOSE_ALGORITHM\n");
            cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
            audio_aec_toggle_set(0);
            break;
        case CVP_DUT_POWEROFF:
            cvp_dut_log("CMD:CVP_DUT_POWEROFF\n");
            sys_enter_soft_poweroff(NULL);
            break;
        default:
            cvp_dut_log("CMD:UNKNOW CMD!!!\n");
            return CVP_DUT_ACK_FALI;
        }
    }
    return CVP_DUT_ACK_SUCCESS;
}

void cvp_dut_init(void)
{
    cvp_dut_log("tx dat");
    if (cvp_dut_hdl == NULL) {
        cvp_dut_hdl = zalloc(sizeof(cvp_dut_t));
    }
    //开机默认设置成算法模式
    cvp_dut_hdl->mode = CVP_DUT_MODE_ALGORITHM;
    app_online_db_register_handle(DB_PKT_TYPE_DMS, cvp_dut_app_online_parse);
}

void cvp_dut_unit(void)
{
    free(cvp_dut_hdl);
    cvp_dut_hdl = NULL;
}

int cvp_dut_spp_tx_packet(u8 command)
{
    if (cvp_dut_hdl) {
        cvp_dut_hdl->tx_buf.magic = CVP_DUT_SPP_MAGIC;
        cvp_dut_hdl->tx_buf.dat[0] = 0;
        cvp_dut_hdl->tx_buf.dat[1] = 0;
        cvp_dut_hdl->tx_buf.dat[2] = 0;
        cvp_dut_hdl->tx_buf.dat[3] = command;
        cvp_dut_log("tx dat");
#if CVP_DUT_OUTPUT_WAY == CVP_DUT_LITTLE_ENDDIAN	//	小端格式
        cvp_dut_hdl->tx_buf.crc = CRC16((&cvp_dut_hdl->tx_buf.dat), CVP_DUT_PACK_NUM - 4);
        put_buf((u8 *)&cvp_dut_hdl->tx_buf.magic, CVP_DUT_PACK_NUM);
        app_online_db_send(DB_PKT_TYPE_DMS, (u8 *)&cvp_dut_hdl->tx_buf.magic, CVP_DUT_PACK_NUM);
#else
        u16 crc_temp;
        int i;
        u8 dat[CVP_DUT_PACK_NUM];
        u8 dat_temp;
        memcpy(dat, &(cvp_dut_hdl->tx_buf), CVP_DUT_PACK_NUM);
        crc_temp = CRC16(dat + 4, 6);
        printf("crc0x%x,0x%x", crc_temp, cvp_dut_hdl->tx_buf.crc);
        for (i = 0; i < 6; i += 2) {	//小端数据转大端
            dat_temp = dat[i];
            dat[i] = dat[i + 1];
            dat[i + 1] = dat_temp;
        }
        crc_temp = CRC16(dat + 4, CVP_DUT_PACK_NUM - 4);
        dat[2] = crc_temp >> 8;
        dat[3] = crc_temp & 0xff;
        put_buf(dat, CVP_DUT_PACK_NUM);
        app_online_db_send(DB_PKT_TYPE_DMS, dat, CVP_DUT_PACK_NUM);
#endif

    }
    return 0;
}

static int cvp_dut_app_online_parse(u8 *packet, u8 size, u8 *ext_data, u16 ext_size)
{
    if (cvp_dut_hdl) {
        cvp_dut_hdl->parse_seq  = ext_data[1];
        cvp_dut_spp_rx_packet(packet, size);
    }
    return 0;
}


int cvp_dut_spp_rx_packet(u8 *dat, u8 len)
{
    if (cvp_dut_hdl) {
        u8 dat_temp;
        u16 crc = 0;
        int ret = 0;
        u8 dat_packet[CVP_DUT_PACK_NUM];
        if (len > CVP_DUT_PACK_NUM) {
            return 1;
        }
        cvp_dut_log("rx dat,%d\n", CVP_DUT_PACK_NUM);
        put_buf(dat, len);
        memcpy(dat_packet, dat, len);
        crc = CRC16(dat + 4, len - 4);
#if CVP_DUT_OUTPUT_WAY == CVP_DUT_BIG_ENDDIAN	//	大端格式
        for (int i = 0; i < 6; i += 2) {		//	大端数据转小端
            dat_temp = dat_packet[i];
            dat_packet[i] = dat_packet[i + 1];
            dat_packet[i + 1] = dat_temp;
        }
#endif
        cvp_dut_log("rx dat_packet");
        memcpy(&(cvp_dut_hdl->rx_buf), dat_packet, 4);
        if (cvp_dut_hdl->rx_buf.magic == CVP_DUT_SPP_MAGIC) {
            cvp_dut_log("crc %x,cvp_dut_hdl->rx_buf.crc %x\n", crc, cvp_dut_hdl->rx_buf.crc);
            if (cvp_dut_hdl->rx_buf.crc == crc || cvp_dut_hdl->rx_buf.crc == 0x1) {
                memcpy(&(cvp_dut_hdl->rx_buf), dat_packet, len);
                ret = cvp_dut_event_deal(cvp_dut_hdl->rx_buf.dat);
                cvp_dut_spp_tx_packet(ret);		//反馈收到命令
                return 0;
            }
        }
    }
    return 1;
}

#endif /*TCFG_AUDIO_CVP_DUT_ENABLE*/

