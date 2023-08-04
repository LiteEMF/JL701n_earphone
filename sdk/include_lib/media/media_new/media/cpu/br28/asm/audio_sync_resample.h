/*****************************************************************
>file name : audio_sync_resample.h
>author : lichao
>create time : Sat 26 Dec 2020 02:27:11 PM CST
*****************************************************************/
#ifndef _AUDIO_SYNC_RESAMPLE_H_
#define _AUDIO_SYNC_RESAMPLE_H_
#include "typedef.h"
#include "system/includes.h"
#include "os/os_api.h"
#include "audio_src.h"


void *audio_sync_resample_open(int input_sample_rate, int sample_rate, u8 data_channels);
int audio_sync_resample_set_output_handler(void *resample,
        void *priv,
        int (*handler)(void *priv, void *data, int len));
void audio_sync_resample_close(void *resample);

int audio_sync_resample_set_slience(void *resample, u8 slience, int fade_time);
int audio_sync_resample_set_in_buffer(void *resample, void *buf, int len);
int audio_sync_resample_config(void *resample, int in_rate, int out_rate);
int audio_sync_resample_write(void *resample, void *data, int len);
int audio_sync_resample_stop(void *resample);
float audio_sync_resample_position(void *resample);
int audio_sync_resample_scale_output(void *resample, int in_sample_rate, int out_sample_rate, int frames);
int audio_sync_resample_bufferd_frames(void *resample);
u32 audio_sync_resample_out_frames(void *resample);
int audio_sync_resample_wait_irq_callback(void *resample, void *priv, void (*callback)(void *));
#endif

