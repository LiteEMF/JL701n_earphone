/*****************************************************************
>file name : sound_device.c
>create time : Fri 25 Feb 2022 05:44:41 PM CST
*****************************************************************/
#include "app_config.h"
#include "media/includes.h"
#include "system/includes.h"
#include "sound_device.h"
#include "audio_config.h"
#include "audio_syncts.h"
#include "audio_dvol.h"
#if TCFG_AUDIO_DAC_ENABLE
#define SOUND_PCM_DAC_ENABLE    1
#else
#define SOUND_PCM_DAC_ENABLE    0
#endif

#define SOUND_PCM_DEV_NUM       (SOUND_PCM_DAC_ENABLE)

#if SOUND_PCM_DAC_ENABLE
#if (TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_LR)||(TCFG_AUDIO_DAC_CONNECT_MODE == DAC_OUTPUT_MONO_R)
s16 dac_buff[4 * 1024 + 512];
#else
s16 dac_buff[2 * 1024 + 512];
#endif
#else
s16 dac_buff[0];
#endif

struct audio_dac_hdl dac_hdl;

extern const int config_audio_dac_mix_enable;
extern struct dac_platform_data dac_data;
void audio_fade_in_fade_out(u8 left_vol, u8 right_vol);

int sound_pcm_dev_is_running(void)
{
    if (SOUND_PCM_DAC_ENABLE) {
        return audio_dac_is_working(&dac_hdl);
    }
    return 1;
}

void sound_pcm_dev_channel_mute(u8 ch, u8 mute)
{
    if (SOUND_PCM_DAC_ENABLE) {
        audio_dac_ch_mute(&dac_hdl, ch, mute);
    }
}


int sound_pcm_trigger_resume(int time_ms, void *priv, void (*callback)(void *priv))
{
    int err = 0;

    if (SOUND_PCM_DAC_ENABLE) {
        err = audio_dac_irq_enable(&dac_hdl, time_ms, priv, callback);
    }

    return err;
}

int sound_pcm_dev_set_delay_time(int prepared_time, int delay_time)
{
    if (SOUND_PCM_DAC_ENABLE) {
        audio_dac_set_delay_time(&dac_hdl, prepared_time, delay_time);
    }

    return 0;
}

int sound_pcm_dev_start(void *private_data, int sample_rate, s16 volume)
{
    if (SOUND_PCM_DAC_ENABLE) {
        if (!audio_dac_is_working(&dac_hdl)) {
            audio_dac_set_sample_rate(&dac_hdl, sample_rate);
            audio_dac_set_volume(&dac_hdl, volume);
            audio_dac_start(&dac_hdl);
        }
        audio_dac_channel_start(private_data);
    }

    return 0;
}

int sound_pcm_dev_stop(void *private_data)
{
    if (SOUND_PCM_DAC_ENABLE) {
        audio_dac_set_protect_time(&dac_hdl, 0, NULL, NULL);
        audio_dac_channel_close(private_data);
        audio_dac_stop(&dac_hdl);
#if (SYS_VOL_TYPE == VOL_TYPE_DIGITAL)
        u8 dvol_idx = MUSIC_DVOL;
        audio_digital_vol_set_no_fade(dvol_idx, 0); //更新当前数字音量为0
#endif/*SYS_VOL_TYPE == VOL_TYPE_DIGITAL*/
    }

    return 0;
}

int sound_pcm_match_sample_rate(int sample_rate)
{
    return sample_rate;
}

int sound_pcm_dev_write(void *private_data, void *data, int len)
{
    int wlen = len;
    if (SOUND_PCM_DEV_NUM == 1) {
        if (SOUND_PCM_DAC_ENABLE) {
            if (config_audio_dac_mix_enable) {
                return audio_dac_channel_write(private_data, &dac_hdl, data, len);
            } else {
                return audio_dac_write(&dac_hdl, data, len);
            }
        }

        return 0;
    }

    int wlen1 = 0;
    if (SOUND_PCM_DAC_ENABLE) {
        if (config_audio_dac_mix_enable) {
            wlen = audio_dac_channel_write(private_data, &dac_hdl, data, len);
        } else {
            wlen = audio_dac_write(&dac_hdl, data, len);
        }
    }

    if (wlen != wlen1) {
        /*
         * TODO :
         * 这个时候说明DAC与IIS的数据消耗不同步，需要启动同步处理机制
         * */
    }

    return wlen;
}

void sound_pcm_dev_add_syncts(void *priv)
{
    /*蓝牙音频同步仅能挂载到一个设备上，无法挂载多个*/

    if (SOUND_PCM_DAC_ENABLE) {
        audio_dac_add_syncts_handle(&dac_hdl, priv);
        return;
    }
}

void sound_pcm_dev_remove_syncts(void *handle)
{
    if (SOUND_PCM_DAC_ENABLE) {
        audio_dac_remove_syncts_handle(&dac_hdl, handle);
    }
}

int sound_pcm_dev_channel_mapping(int ch_num)
{
    u8 ch_map = SOUND_CHMAP_MONO;
    if (SOUND_PCM_DAC_ENABLE) {
        ch_map = audio_dac_get_channel(&dac_hdl);
        if (ch_map == DAC_OUTPUT_LR) {
            return 2;
        }
        return 1;
    }

    return 0;
}

int sound_pcm_dev_buffered_frames(void)
{
    if (SOUND_PCM_DAC_ENABLE) {
        return audio_dac_buffered_frames(&dac_hdl);
    }

    return 0;
}

int sound_pcm_dev_buffered_time(void)
{
    if (SOUND_PCM_DAC_ENABLE) {
        return audio_dac_data_time(&dac_hdl);
    }

    return 0;
}

void sound_pcm_set_underrun_params(int time_ms, void *priv, void (*callback)(void *))
{
    if (SOUND_PCM_DAC_ENABLE) {
        audio_dac_set_protect_time(&dac_hdl, time_ms, priv, callback);
    }
}

void sound_pcm_dev_try_power_on(void)
{
    if (SOUND_PCM_DAC_ENABLE) {
        if (!sound_pcm_dev_is_running()) {
            audio_dac_try_power_on(&dac_hdl);
        }
    }
}

void sound_pcm_dev_set_analog_gain(s16 gain)
{
    if (SOUND_PCM_DAC_ENABLE) {
        audio_dac_set_analog_vol(&dac_hdl, gain);
    }
}

static void sound_pcm_dac_driver_init(void)
{
    if (!SOUND_PCM_DAC_ENABLE) {
        return;
    }

    audio_dac_init(&dac_hdl, &dac_data);

#if defined(TCFG_AUDIO_DAC_24BIT_MODE) && TCFG_AUDIO_DAC_24BIT_MODE
    audio_dac_set_bit_mode(&dac_hdl, 1);
#endif/*TCFG_AUDIO_DAC_24BIT_MODE*/

    //u32 dacr32 = read_capless_DTB();
    //audio_dac_set_capless_DTB(&dac_hdl, dacr32);
    audio_dac_set_buff(&dac_hdl, dac_buff, sizeof(dac_buff));
    audio_dac_set_delay_time(&dac_hdl, 20, 30);
    audio_dac_set_analog_vol(&dac_hdl, 0);

    struct audio_dac_trim dac_trim = {0};
    int len = syscfg_read(CFG_DAC_TRIM_INFO, (void *)&dac_trim, sizeof(dac_trim));
    if (len != sizeof(dac_trim) || audio_dac_trim_value_check(&dac_trim)) {
        audio_dac_do_trim(&dac_hdl, &dac_trim, 0);
        syscfg_write(CFG_DAC_TRIM_INFO, (void *)&dac_trim, sizeof(dac_trim));
    }

    //mic_trim_run();
    audio_dac_set_trim_value(&dac_hdl, &dac_trim);
    audio_dac_set_fade_handler(&dac_hdl, NULL, audio_fade_in_fade_out);
}

int sound_pcm_driver_init(void)
{
    if (SOUND_PCM_DAC_ENABLE) {
        sound_pcm_dac_driver_init();
    }
    return 0;
}


int sound_pcm_sync_device_select(void)
{
    if (SOUND_PCM_DAC_ENABLE) {
        return PCM_INSIDE_DAC;
    }
}

