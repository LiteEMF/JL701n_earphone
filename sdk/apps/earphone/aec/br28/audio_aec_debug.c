#define ModuleEnable_Max	5
static const char *ModuleEnable_Name[ModuleEnable_Max] = {
    "AEC", "NLP", "ANS", "ENC", "AGC"
};

#if TCFG_AUDIO_DUAL_MIC_ENABLE
void aec_param_dump(struct dms_attr *param)
{
#ifdef CONFIG_DEBUG_ENABLE
    printf("===========dump dms param==================\n");
    printf("EnableBit:%x\n", param->EnableBit);
    for (u8 i = 0; i < ModuleEnable_Max; i++) {
        if (param->EnableBit & BIT(i)) {
            printf("%s ON", ModuleEnable_Name[i]);
        } else {
            printf("%s OFF", ModuleEnable_Name[i]);
        }
    }
    printf("ul_eq_en:%x\n", param->ul_eq_en);
    extern void put_float(double fv);
    puts("AGC_NDT_fade_in_step:");
    put_float(param->AGC_NDT_fade_in_step);
    puts("AGC_NDT_fade_out_step:");
    put_float(param->AGC_NDT_fade_out_step);
    puts("AGC_DT_fade_in_step:");
    put_float(param->AGC_DT_fade_in_step);
    puts("AGC_DT_fade_out_step:");
    put_float(param->AGC_DT_fade_out_step);
    puts("AGC_NDT_max_gain:");
    put_float(param->AGC_NDT_max_gain);
    puts("AGC_NDT_min_gain:");
    put_float(param->AGC_NDT_min_gain);
    puts("AGC_NDT_speech_thr:");
    put_float(param->AGC_NDT_speech_thr);
    puts("AGC_DT_max_gain:");
    put_float(param->AGC_DT_max_gain);
    puts("AGC_DT_min_gain:");
    put_float(param->AGC_DT_min_gain);
    puts("AGC_DT_speech_thr:");
    put_float(param->AGC_DT_speech_thr);
    puts("AGC_echo_present_thr:");
    put_float(param->AGC_echo_present_thr);

    puts("aec_process_maxfrequency:");
    put_float(param->aec_process_maxfrequency);
    puts("aec_process_minfrequency:");
    put_float(param->aec_process_minfrequency);
    puts("aec_af_length:");
    put_float(param->af_length);

    puts("nlp_process_maxfrequency");
    put_float(param->nlp_process_maxfrequency);
    puts("nlp_process_minfrequency:");
    put_float(param->nlp_process_minfrequency);
    puts("nlp_overdrive:");
    put_float(param->overdrive);

    puts("ANS_AggressFactor:");
    put_float(param->aggressfactor);
    puts("ANS_MinSuppress:");
    put_float(param->minsuppress);
    puts("ANC_init_noise_lvl:");
    put_float(param->init_noise_lvl);

    printf("enc_process_maxfreq:%d", param->enc_process_maxfreq);
    printf("enc_process_minfreq:%d", param->enc_process_minfreq);
    printf("sir_maxfreq:%d", param->sir_maxfreq);
    puts("mic_distance:");
    put_float(param->mic_distance);
    puts("target_signal_degradation:");
    put_float(param->target_signal_degradation);
    puts("enc_aggressfactor:");
    put_float(param->enc_aggressfactor);
    puts("enc_minsuppress:");
    put_float(param->enc_minsuppress);

    puts("GloabalMinSuppress:"), put_float(param->global_minsuppress);
    printf("===============End==================\n");
#endif/*CONFIG_DEBUG_ENABLE*/
}
#else
void aec_param_dump(struct aec_s_attr *param)
{
#ifdef CONFIG_DEBUG_ENABLE
    printf("===========dump sms param==================\n");
    printf("toggle:%d\n", param->toggle);
    printf("EnableBit:%x\n", param->EnableBit);
    printf("ul_eq_en:%x\n", param->ul_eq_en);
    extern void put_float(double fv);
    puts("AGC_NDT_fade_in_step:");
    put_float(param->AGC_NDT_fade_in_step);
    puts("AGC_NDT_fade_out_step:");
    put_float(param->AGC_NDT_fade_out_step);
    puts("AGC_DT_fade_in_step:");
    put_float(param->AGC_DT_fade_in_step);
    puts("AGC_DT_fade_out_step:");
    put_float(param->AGC_DT_fade_out_step);
    puts("AGC_NDT_max_gain:");
    put_float(param->AGC_NDT_max_gain);
    puts("AGC_NDT_min_gain:");
    put_float(param->AGC_NDT_min_gain);
    puts("AGC_NDT_speech_thr:");
    put_float(param->AGC_NDT_speech_thr);
    puts("AGC_DT_max_gain:");
    put_float(param->AGC_DT_max_gain);
    puts("AGC_DT_min_gain:");
    put_float(param->AGC_DT_min_gain);
    puts("AGC_DT_speech_thr:");
    put_float(param->AGC_DT_speech_thr);
    puts("AGC_echo_present_thr:");
    put_float(param->AGC_echo_present_thr);
    puts("AEC_DT_AggressiveFactor:");
    put_float(param->AEC_DT_AggressiveFactor);
    puts("AEC_RefEngThr:");
    put_float(param->AEC_RefEngThr);
    puts("ES_AggressFactor:");
    put_float(param->ES_AggressFactor);
    puts("ES_MinSuppress:");
    put_float(param->ES_MinSuppress);
    puts("ANS_AggressFactor:");
    put_float(param->ANS_AggressFactor);
    puts("ANS_MinSuppress:");
    put_float(param->ANS_MinSuppress);
#endif/*CONFIG_DEBUG_ENABLE*/
}
#endif/*CONFIG_DEBUG_ENABLE*/
