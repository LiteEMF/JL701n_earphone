/*****************************************************************
>file name : lib/media/cpu/br22/audio_link.c
>author :
>create time : Fri 7 Dec 2018 14:59:12 PM CST
*****************************************************************/
#include "includes.h"
#include "asm/includes.h"
#include "media/includes.h"
#include "system/includes.h"
#include "asm/clock.h"
#include "asm/iis.h"
#include "audio_link.h"

/* #if TCFG_AUDIO_INPUT_IIS || TCFG_AUDIO_OUTPUT_IIS */
/* #define ALINK_TEST_ENABLE */

#define ALINK_DEBUG_INFO
#ifdef ALINK_DEBUG_INFO
#define alink_printf  printf
#else
#define alink_printf(...)
#endif

static void alink_info_dump();
static u32 *ALNK_BUF_ADR[] = {
    (u32 *)(&(JL_ALNK0->ADR0)),
    (u32 *)(&(JL_ALNK0->ADR1)),
    (u32 *)(&(JL_ALNK0->ADR2)),
    (u32 *)(&(JL_ALNK0->ADR3)),
};

static u32 *ALNK_HWPTR[] = {
    (u32 *)(&(JL_ALNK0->HWPTR0)),
    (u32 *)(&(JL_ALNK0->HWPTR1)),
    (u32 *)(&(JL_ALNK0->HWPTR2)),
    (u32 *)(&(JL_ALNK0->HWPTR3)),
};

static u32 *ALNK_SWPTR[] = {
    (u32 *)(&(JL_ALNK0->SWPTR0)),
    (u32 *)(&(JL_ALNK0->SWPTR1)),
    (u32 *)(&(JL_ALNK0->SWPTR2)),
    (u32 *)(&(JL_ALNK0->SWPTR3)),
};

static u32 *ALNK_SHN[] = {
    (u32 *)(&(JL_ALNK0->SHN0)),
    (u32 *)(&(JL_ALNK0->SHN1)),
    (u32 *)(&(JL_ALNK0->SHN2)),
    (u32 *)(&(JL_ALNK0->SHN3)),
};

static u32 PFI_ALNK0_DAT[] = {
    PFI_ALNK0_DAT0,
    PFI_ALNK0_DAT1,
    PFI_ALNK0_DAT2,
    PFI_ALNK0_DAT3,
};

static u32 FO_ALNK0_DAT[] = {
    FO_ALNK0_DAT0,
    FO_ALNK0_DAT1,
    FO_ALNK0_DAT2,
    FO_ALNK0_DAT3,
};


enum {
    MCLK_11M2896K = 0,
    MCLK_12M288K
};

enum {
    MCLK_EXTERNAL	= 0u,
    MCLK_SYS_CLK		,
    MCLK_OSC_CLK 		,
    MCLK_PLL_CLK		,
};

enum {
    MCLK_DIV_1		= 0u,
    MCLK_DIV_2			,
    MCLK_DIV_4			,
    MCLK_DIV_8			,
    MCLK_DIV_16			,
};

enum {
    MCLK_LRDIV_EX	= 0u,
    MCLK_LRDIV_64FS		,
    MCLK_LRDIV_128FS	,
    MCLK_LRDIV_192FS	,
    MCLK_LRDIV_256FS	,
    MCLK_LRDIV_384FS	,
    MCLK_LRDIV_512FS	,
    MCLK_LRDIV_768FS	,
};

ALINK_PARM *p_alink_parm;


//==================================================
static void alink_clk_io_in_init(u8 gpio)
{
    gpio_set_direction(gpio, 1);
    gpio_set_pull_down(gpio, 0);
    gpio_set_pull_up(gpio, 1);
    gpio_set_die(gpio, 1);
}

static void *alink_addr(u8 ch)
{
    u8 *buf_addr = NULL; //can be used
    u32 buf_index = 0;
    buf_addr = (u8 *)(p_alink_parm->ch_cfg[ch].buf);

    u8 index_table[4] = {12, 13, 14, 15};
    u8 bit_index = index_table[ch];
    buf_index = (JL_ALNK0->CON0 & BIT(bit_index)) ? 0 : 1;
    buf_addr = buf_addr + ((p_alink_parm->dma_len / 2) * buf_index);

    return buf_addr;
}

void alink_isr_en(u8 en)
{
    if (en) {
        bit_set_ie(IRQ_ALINK0_IDX);
    } else {
        bit_clr_ie(IRQ_ALINK0_IDX);
    }
}

___interrupt
static void alink_circle_isr(void)
{
    u16 reg;
    s16 *buf_addr = NULL ;
    u8 ch = 0;

    reg = JL_ALNK0->CON2;

    //channel 0
    if (reg & BIT(4)) {
        ch = 0;
        goto __isr_cb;
    }
    //channel 1
    if (reg & BIT(5)) {
        ch = 1;
        goto __isr_cb;
    }
    //channel 2
    if (reg & BIT(6)) {
        ch = 2;
        goto __isr_cb;
    }
    //channel 3
    if (reg & BIT(7)) {
        ch = 3;
        goto __isr_cb;
    }

__isr_cb:
    buf_addr = (s16 *)(*ALNK_BUF_ADR[ch] + *ALNK_SWPTR[ch] * 4);
    if (p_alink_parm->ch_cfg[ch].isr_cb) {
        p_alink_parm->ch_cfg[ch].isr_cb(buf_addr, p_alink_parm->dma_len / 2);
    }
    *ALNK_SHN[ch] = p_alink_parm->dma_len / 8;
    ALINK_CLR_CHx_PND(ch);
    asm("csync");
}

___interrupt
static void alink_dual_isr(void)
{
    u16 reg;
    s16 *buf_addr = NULL ;
    u8 ch = 0;

    reg = JL_ALNK0->CON2;

    //channel 0
    if (reg & BIT(4)) {
        ch = 0;
        goto __isr_cb;
    }
    //channel 1
    if (reg & BIT(5)) {
        ch = 1;
        goto __isr_cb;
    }
    //channel 2
    if (reg & BIT(6)) {
        ch = 2;
        goto __isr_cb;
    }
    //channel 3
    if (reg & BIT(7)) {
        ch = 3;
        goto __isr_cb;
    }

__isr_cb:
    ALINK_CLR_CHx_PND(ch);
    buf_addr = alink_addr(ch);
    if (p_alink_parm->ch_cfg[ch].isr_cb) {
        p_alink_parm->ch_cfg[ch].isr_cb(buf_addr, p_alink_parm->dma_len / 2);
    }
}


static void alink_sr(u32 rate)
{
    alink_printf("ALINK_SR = %d\n", rate);
    u8 pll_target_frequency = clock_get_pll_target_frequency();
    alink_printf("pll_target_frequency = %d\n", pll_target_frequency);
    switch (rate) {
    case ALINK_SR_48000:
        /*12.288Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_44100:
        /*11.289Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_1);
        if (pll_target_frequency == 192) {
            ALINK_LRDIV(MCLK_LRDIV_256FS);
        } else if (pll_target_frequency == 240) {
            ALINK_LRDIV(MCLK_LRDIV_128FS);
        }
        break ;

    case ALINK_SR_32000:
        /*12.288Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_384FS);
        break ;

    case ALINK_SR_24000:
        /*12.288Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_2);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_22050:
        /*11.289Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_1);
        if (pll_target_frequency == 192) {
            ALINK_LRDIV(MCLK_LRDIV_512FS);
        } else if (pll_target_frequency == 240) {
            ALINK_LRDIV(MCLK_LRDIV_256FS);
        }
        break ;

    case ALINK_SR_16000:
        /*12.288/2Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_768FS);
        break ;

    case ALINK_SR_12000:
        /*12.288/2Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_4);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break ;

    case ALINK_SR_11025:
        /*11.289/2Mhz 512fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_2);
        if (pll_target_frequency == 192) {
            ALINK_LRDIV(MCLK_LRDIV_512FS);
        } else if (pll_target_frequency == 240) {
            ALINK_LRDIV(MCLK_LRDIV_256FS);
        }
        break ;

    case ALINK_SR_8000:
        /*12.288/4Mhz 384fs*/
        audio_link_clock_sel(ALINK_CLOCK_12M288K);
        ALINK_MDIV(MCLK_DIV_4);
        ALINK_LRDIV(MCLK_LRDIV_384FS);
        break ;

    default:
        //44100
        /*11.289Mhz 256fs*/
        audio_link_clock_sel(ALINK_CLOCK_11M2896K);
        ALINK_MDIV(MCLK_DIV_1);
        ALINK_LRDIV(MCLK_LRDIV_256FS);
        break;
    }
    if (p_alink_parm->role == ALINK_ROLE_SLAVE) {
        ALINK_LRDIV(MCLK_LRDIV_EX);
    }
}


void alink_channel_init(u8 ch_idx, u8 dir, void (*handle)(s16 *buf, u32 len))
{
    /* u8 i = 0; */
    p_alink_parm->ch_cfg[ch_idx].dir = dir;
    p_alink_parm->ch_cfg[ch_idx].isr_cb = handle;
    p_alink_parm->ch_cfg[ch_idx].buf = zalloc(p_alink_parm->dma_len);

    /* printf(">>> alink_channel_init %x\n", p_alink_parm->ch_cfg[ch_idx].buf); */
    u32 *buf_addr;

    //===================================//
    //           ALNK CH DMA BUF         //
    //===================================//
    buf_addr = ALNK_BUF_ADR[ch_idx];
    *buf_addr = (u32)(p_alink_parm->ch_cfg[ch_idx].buf);

    //===================================//
    //           ALNK工作模式            //
    //===================================//
    if (p_alink_parm->mode > ALINK_MD_IIS_RALIGN) {
        ALINK_CHx_MODE_SEL((p_alink_parm->mode - ALINK_MD_IIS_RALIGN), ch_idx);
    } else {
        ALINK_CHx_MODE_SEL(p_alink_parm->mode, ch_idx);
    }
    //===================================//
    //          ALNK CH DAT IO INIT      //
    //===================================//
    if (p_alink_parm->ch_cfg[ch_idx].dir == ALINK_DIR_RX) {
        gpio_set_direction(p_alink_parm->ch_cfg[ch_idx].data_io, 1);
        gpio_set_pull_down(p_alink_parm->ch_cfg[ch_idx].data_io, 0);
        gpio_set_pull_up(p_alink_parm->ch_cfg[ch_idx].data_io, 1);
        gpio_set_die(p_alink_parm->ch_cfg[ch_idx].data_io, 1);
        gpio_set_fun_input_port(p_alink_parm->ch_cfg[ch_idx].data_io, PFI_ALNK0_DAT[ch_idx], LOW_POWER_FREE);
        ALINK_CHx_DIR_RX_MODE(ch_idx);
    } else {
        gpio_direction_output(p_alink_parm->ch_cfg[ch_idx].data_io, 1);
        gpio_set_fun_output_port(p_alink_parm->ch_cfg[ch_idx].data_io, FO_ALNK0_DAT[ch_idx], 1, 1, LOW_POWER_FREE);
        ALINK_CHx_DIR_TX_MODE(ch_idx);
    }
    ALINK_CHx_IE(ch_idx, 1);
    r_printf("alink_ch %d init\n", ch_idx);
}

static void alink_info_dump()
{
    alink_printf("JL_ALNK0->CON0 = 0x%x", JL_ALNK0->CON0);
    alink_printf("JL_ALNK0->CON1 = 0x%x", JL_ALNK0->CON1);
    alink_printf("JL_ALNK0->CON2 = 0x%x", JL_ALNK0->CON2);
    alink_printf("JL_ALNK0->CON3 = 0x%x", JL_ALNK0->CON3);
    alink_printf("JL_ALNK0->LEN = 0x%x", JL_ALNK0->LEN);
    alink_printf("JL_ALNK0->ADR0 = 0x%x", JL_ALNK0->ADR0);
    alink_printf("JL_ALNK0->ADR1 = 0x%x", JL_ALNK0->ADR1);
    alink_printf("JL_ALNK0->ADR2 = 0x%x", JL_ALNK0->ADR2);
    alink_printf("JL_ALNK0->ADR3 = 0x%x", JL_ALNK0->ADR3);
    alink_printf("JL_ALNK0->PNS = 0x%x", JL_ALNK0->PNS);
    alink_printf("JL_ALNK0->HWPTR0 = 0x%x", JL_ALNK0->HWPTR0);
    alink_printf("JL_ALNK0->HWPTR1 = 0x%x", JL_ALNK0->HWPTR1);
    alink_printf("JL_ALNK0->HWPTR2 = 0x%x", JL_ALNK0->HWPTR2);
    alink_printf("JL_ALNK0->HWPTR3 = 0x%x", JL_ALNK0->HWPTR3);
    alink_printf("JL_ALNK0->SWPTR0 = 0x%x", JL_ALNK0->SWPTR0);
    alink_printf("JL_ALNK0->SWPTR1 = 0x%x", JL_ALNK0->SWPTR1);
    alink_printf("JL_ALNK0->SWPTR2 = 0x%x", JL_ALNK0->SWPTR2);
    alink_printf("JL_ALNK0->SWPTR3 = 0x%x", JL_ALNK0->SWPTR3);
    alink_printf("JL_ALNK0->SHN0 = 0x%x", JL_ALNK0->SHN0);
    alink_printf("JL_ALNK0->SHN1 = 0x%x", JL_ALNK0->SHN1);
    alink_printf("JL_ALNK0->SHN2 = 0x%x", JL_ALNK0->SHN2);
    alink_printf("JL_ALNK0->SHN3 = 0x%x", JL_ALNK0->SHN3);
    /* alink_printf("JL_PLL->PLL_CON3 = 0x%x", JL_PLL->PLL_CON3); */
    //alink_printf("JL_CLOCK->CLK_CON2 = 0x%x", JL_CLOCK->CLK_CON2);
}

int alink_init(ALINK_PARM *parm)
{
    if (parm == NULL) {
        return -1;
    }

    ALNK_CON_RESET();
    ALNK_HWPTR_RESET();
    ALNK_SWPTR_RESET();
    ALNK_SHN_RESET();
    ALNK_ADR_RESET();
    ALNK_PNS_RESET();
    ALINK_CLR_ALL_PND();

    p_alink_parm = parm;

    ALINK_MSRC(MCLK_PLL_CLK);	/*MCLK source*/

    //===================================//
    //        输出时钟配置               //
    //===================================//
    if (parm->role == ALINK_ROLE_MASTER) {
        //主机输出时钟
        if (parm->mclk_io != ALINK_CLK_OUPUT_DISABLE) {
            gpio_direction_output(parm->mclk_io, 1);
            gpio_set_fun_output_port(parm->mclk_io, FO_ALNK0_MCLK, 1, 1, LOW_POWER_FREE);
            ALINK_MOE(1);				/*MCLK output to IO*/
        }
        if ((parm->sclk_io != ALINK_CLK_OUPUT_DISABLE) && (parm->lrclk_io != ALINK_CLK_OUPUT_DISABLE)) {
            gpio_direction_output(parm->lrclk_io, 1);
            gpio_set_fun_output_port(parm->lrclk_io, FO_ALNK0_LRCK, 1, 1, LOW_POWER_FREE);
            gpio_direction_output(parm->sclk_io, 1);
            gpio_set_fun_output_port(parm->sclk_io, FO_ALNK0_SCLK, 1, 1, LOW_POWER_FREE);
            ALINK_SOE(1);				/*SCLK/LRCK output to IO*/
        }
    } else {
        //从机输入时钟
        if (parm->mclk_io != ALINK_CLK_OUPUT_DISABLE) {
            alink_clk_io_in_init(parm->mclk_io);
            gpio_set_fun_input_port(parm->mclk_io, PFI_ALNK0_MCLK, LOW_POWER_FREE);
            ALINK_MOE(0);				/*MCLK input to IO*/
        }
        if ((parm->sclk_io != ALINK_CLK_OUPUT_DISABLE) && (parm->lrclk_io != ALINK_CLK_OUPUT_DISABLE)) {
            alink_clk_io_in_init(parm->lrclk_io);
            gpio_set_fun_input_port(parm->lrclk_io, PFI_ALNK0_LRCK, LOW_POWER_FREE);
            alink_clk_io_in_init(parm->sclk_io);
            gpio_set_fun_input_port(parm->sclk_io, PFI_ALNK0_SCLK, LOW_POWER_FREE);
            ALINK_SOE(0);				/*SCLK/LRCK output to IO*/
        }
    }
    //===================================//
    //        基本模式/扩展模式          //
    //===================================//
    ALINK_DSPE(p_alink_parm->mode >> 2);

    //===================================//
    //         数据位宽16/32bit          //
    //===================================//
    //注意: 配置了24bit, 一定要选ALINK_FRAME_64SCLK, 因为sclk32 x 2才会有24bit;
    if (p_alink_parm->bitwide == ALINK_LEN_24BIT) {
        ASSERT(p_alink_parm->sclk_per_frame == ALINK_FRAME_64SCLK);
        ALINK_24BIT_MODE();
        //一个点需要4bytes, LR = 2, 双buf = 2
    } else {
        ALINK_16BIT_MODE();
        //一个点需要2bytes, LR = 2, 双buf = 2
    }
    //===================================//
    //             时钟边沿选择          //
    //===================================//
    if (p_alink_parm->clk_mode == ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE) {
        SCLKINV(0);
    } else {
        SCLKINV(1);
    }
    //===================================//
    //            每帧数据sclk个数       //
    //===================================//
    if (p_alink_parm->sclk_per_frame == ALINK_FRAME_64SCLK) {
        F32_EN(0);
    } else {
        F32_EN(1);
    }

    //===================================//
    //            设置数据采样率       	 //
    //===================================//
    alink_sr(p_alink_parm->sample_rate);

    //===================================//
    //            设置DMA MODE       	 //
    //===================================//
    ALINK_DMA_MODE_SEL(p_alink_parm->buf_mode);
    if (p_alink_parm->buf_mode == ALINK_BUF_CIRCLE) {
        ALINK_OPNS_SET(p_alink_parm->dma_len / 16);
        ALINK_IPNS_SET(p_alink_parm->dma_len / 16);
        //一个点需要2bytes, LR = 2
        JL_ALNK0->LEN = p_alink_parm->dma_len / 4; //点数
        request_irq(IRQ_ALINK0_IDX, 3, alink_circle_isr, 0);
    } else {
        //一个点需要2bytes, LR = 2, 双buf = 2
        JL_ALNK0->LEN = p_alink_parm->dma_len / 8; //点数
        request_irq(IRQ_ALINK0_IDX, 3, alink_dual_isr, 0);
    }

    return 0;
}

void alink_channel_close(u8 ch_idx)
{
    gpio_set_pull_up(p_alink_parm->ch_cfg[ch_idx].data_io, 0);
    gpio_set_pull_down(p_alink_parm->ch_cfg[ch_idx].data_io, 0);
    gpio_set_direction(p_alink_parm->ch_cfg[ch_idx].data_io, 1);
    gpio_set_die(p_alink_parm->ch_cfg[ch_idx].data_io, 0);
    ALINK_CHx_IE(ch_idx, 0);
    ALINK_CHx_CLOSE(0, ch_idx);
    if (p_alink_parm->ch_cfg[ch_idx].buf) {
        free(p_alink_parm->ch_cfg[ch_idx].buf);
        p_alink_parm->ch_cfg[ch_idx].buf = NULL;
    }
    r_printf("alink_ch %d closed\n", ch_idx);
}


int alink_start(void)
{
    if (p_alink_parm) {
        ALINK_EN(1);
        return 0;
    }
    return -1;
}

void alink_uninit(void)
{
    ALINK_EN(0);
    for (int i = 0; i < 4; i++) {
        if (p_alink_parm->ch_cfg[i].buf) {
            alink_channel_close(i);
        }
    }
    ALNK_CON_RESET();
    p_alink_parm = NULL;
    alink_printf("audio_link_exit OK\n");
}

int alink_sr_set(u16 sr)
{
    if (p_alink_parm) {
        ALINK_EN(0);
        alink_sr(sr);
        ALINK_EN(1);
        return 0;
    } else {
        return -1;
    }
}

//===============================================//
//			     for APP use demo                //
//===============================================//

#ifdef ALINK_TEST_ENABLE

short const tsin_441k[441] = {
    0x0000, 0x122d, 0x23fb, 0x350f, 0x450f, 0x53aa, 0x6092, 0x6b85, 0x744b, 0x7ab5, 0x7ea2, 0x7fff, 0x7ec3, 0x7af6, 0x74ab, 0x6c03,
    0x612a, 0x545a, 0x45d4, 0x35e3, 0x24db, 0x1314, 0x00e9, 0xeeba, 0xdce5, 0xcbc6, 0xbbb6, 0xad08, 0xa008, 0x94fa, 0x8c18, 0x858f,
    0x8181, 0x8003, 0x811d, 0x84ca, 0x8af5, 0x9380, 0x9e3e, 0xaaf7, 0xb969, 0xc94a, 0xda46, 0xec06, 0xfe2d, 0x105e, 0x223a, 0x3365,
    0x4385, 0x5246, 0x5f5d, 0x6a85, 0x7384, 0x7a2d, 0x7e5b, 0x7ffa, 0x7f01, 0x7b75, 0x7568, 0x6cfb, 0x6258, 0x55b7, 0x4759, 0x3789,
    0x2699, 0x14e1, 0x02bc, 0xf089, 0xdea7, 0xcd71, 0xbd42, 0xae6d, 0xa13f, 0x95fd, 0x8ce1, 0x861a, 0x81cb, 0x800b, 0x80e3, 0x844e,
    0x8a3c, 0x928c, 0x9d13, 0xa99c, 0xb7e6, 0xc7a5, 0xd889, 0xea39, 0xfc5a, 0x0e8f, 0x2077, 0x31b8, 0x41f6, 0x50de, 0x5e23, 0x697f,
    0x72b8, 0x799e, 0x7e0d, 0x7fee, 0x7f37, 0x7bed, 0x761f, 0x6ded, 0x6380, 0x570f, 0x48db, 0x392c, 0x2855, 0x16ad, 0x048f, 0xf259,
    0xe06b, 0xcf20, 0xbed2, 0xafd7, 0xa27c, 0x9705, 0x8db0, 0x86ab, 0x821c, 0x801a, 0x80b0, 0x83da, 0x8988, 0x919c, 0x9bee, 0xa846,
    0xb666, 0xc603, 0xd6ce, 0xe86e, 0xfa88, 0x0cbf, 0x1eb3, 0x3008, 0x4064, 0x4f73, 0x5ce4, 0x6874, 0x71e6, 0x790a, 0x7db9, 0x7fdc,
    0x7f68, 0x7c5e, 0x76d0, 0x6ed9, 0x64a3, 0x5863, 0x4a59, 0x3acc, 0x2a0f, 0x1878, 0x0661, 0xf42a, 0xe230, 0xd0d0, 0xc066, 0xb145,
    0xa3bd, 0x9813, 0x8e85, 0x8743, 0x8274, 0x8030, 0x8083, 0x836b, 0x88da, 0x90b3, 0x9acd, 0xa6f5, 0xb4ea, 0xc465, 0xd515, 0xe6a3,
    0xf8b6, 0x0aee, 0x1ced, 0x2e56, 0x3ecf, 0x4e02, 0x5ba1, 0x6764, 0x710e, 0x786f, 0x7d5e, 0x7fc3, 0x7f91, 0x7cc9, 0x777a, 0x6fc0,
    0x65c1, 0x59b3, 0x4bd3, 0x3c6a, 0x2bc7, 0x1a41, 0x0833, 0xf5fb, 0xe3f6, 0xd283, 0xc1fc, 0xb2b7, 0xa503, 0x9926, 0x8f60, 0x87e1,
    0x82d2, 0x804c, 0x805d, 0x8303, 0x8833, 0x8fcf, 0x99b2, 0xa5a8, 0xb372, 0xc2c9, 0xd35e, 0xe4da, 0xf6e4, 0x091c, 0x1b26, 0x2ca2,
    0x3d37, 0x4c8e, 0x5a58, 0x664e, 0x7031, 0x77cd, 0x7cfd, 0x7fa3, 0x7fb4, 0x7d2e, 0x781f, 0x70a0, 0x66da, 0x5afd, 0x4d49, 0x3e04,
    0x2d7d, 0x1c0a, 0x0a05, 0xf7cd, 0xe5bf, 0xd439, 0xc396, 0xb42d, 0xa64d, 0x9a3f, 0x9040, 0x8886, 0x8337, 0x806f, 0x803d, 0x82a2,
    0x8791, 0x8ef2, 0x989c, 0xa45f, 0xb1fe, 0xc131, 0xd1aa, 0xe313, 0xf512, 0x074a, 0x195d, 0x2aeb, 0x3b9b, 0x4b16, 0x590b, 0x6533,
    0x6f4d, 0x7726, 0x7c95, 0x7f7d, 0x7fd0, 0x7d8c, 0x78bd, 0x717b, 0x67ed, 0x5c43, 0x4ebb, 0x3f9a, 0x2f30, 0x1dd0, 0x0bd6, 0xf99f,
    0xe788, 0xd5f1, 0xc534, 0xb5a7, 0xa79d, 0x9b5d, 0x9127, 0x8930, 0x83a2, 0x8098, 0x8024, 0x8247, 0x86f6, 0x8e1a, 0x978c, 0xa31c,
    0xb08d, 0xbf9c, 0xcff8, 0xe14d, 0xf341, 0x0578, 0x1792, 0x2932, 0x39fd, 0x499a, 0x57ba, 0x6412, 0x6e64, 0x7678, 0x7c26, 0x7f50,
    0x7fe6, 0x7de4, 0x7955, 0x7250, 0x68fb, 0x5d84, 0x5029, 0x412e, 0x30e0, 0x1f95, 0x0da7, 0xfb71, 0xe953, 0xd7ab, 0xc6d4, 0xb725,
    0xa8f1, 0x9c80, 0x9213, 0x89e1, 0x8413, 0x80c9, 0x8012, 0x81f3, 0x8662, 0x8d48, 0x9681, 0xa1dd, 0xaf22, 0xbe0a, 0xce48, 0xdf89,
    0xf171, 0x03a6, 0x15c7, 0x2777, 0x385b, 0x481a, 0x5664, 0x62ed, 0x6d74, 0x75c4, 0x7bb2, 0x7f1d, 0x7ff5, 0x7e35, 0x79e6, 0x731f,
    0x6a03, 0x5ec1, 0x5193, 0x42be, 0x328f, 0x2159, 0x0f77, 0xfd44, 0xeb1f, 0xd967, 0xc877, 0xb8a7, 0xaa49, 0x9da8, 0x9305, 0x8a98,
    0x848b, 0x80ff, 0x8006, 0x81a5, 0x85d3, 0x8c7c, 0x957b, 0xa0a3, 0xadba, 0xbc7b, 0xcc9b, 0xddc6, 0xefa2, 0x01d3, 0x13fa, 0x25ba,
    0x36b6, 0x4697, 0x5509, 0x61c2, 0x6c80, 0x750b, 0x7b36, 0x7ee3, 0x7ffd, 0x7e7f, 0x7a71, 0x73e8, 0x6b06, 0x5ff8, 0x52f8, 0x444a,
    0x343a, 0x231b, 0x1146, 0xff17, 0xecec, 0xdb25, 0xca1d, 0xba2c, 0xaba6, 0x9ed6, 0x93fd, 0x8b55, 0x850a, 0x813d, 0x8001, 0x815e,
    0x854b, 0x8bb5, 0x947b, 0x9f6e, 0xac56, 0xbaf1, 0xcaf1, 0xdc05, 0xedd3
};

static short const tsin_16k[16] = {
    0x0000, 0x30fd, 0x5a83, 0x7641, 0x7fff, 0x7642, 0x5a82, 0x30fc, 0x0000, 0xcf04, 0xa57d, 0x89be, 0x8000, 0x89be, 0xa57e, 0xcf05,
};
static u16 tx_s_cnt = 0;
static int get_sine_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 441) {
            *s_cnt = 0;
        }
        *data++ = tsin_441k[*s_cnt];
        if (ch == 2) {
            *data++ = tsin_441k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}

static int get_sine_16k_data(u16 *s_cnt, s16 *data, u16 points, u8 ch)
{
    while (points--) {
        if (*s_cnt >= 16) {
            *s_cnt = 0;
        }
        *data++ = tsin_16k[*s_cnt];
        if (ch == 2) {
            *data++ = tsin_16k[*s_cnt];
        }
        (*s_cnt)++;
    }
    return 0;
}

extern struct audio_decoder_task decode_task;
void handle_tx(void *buf, u16 len)
{
    putchar('o');
#if 0
    /* get_sine_16k_data(&tx_s_cnt, buf, len / 2 / 2, 2); */
    get_sine_data(&tx_s_cnt, buf, len / 2 / 2, 2);
#else
    s16 *data_tx = (s16 *)buf;
    /* s16 *data = (s16 *)data_buf; */
    for (int i = 0; i < len / sizeof(s16); i++) {
        /* data_tx[i] = 0xFFFF; */
        data_tx[i] = 0x5a5a;
        /* printf("%x", data_tx[i]); */
    }
#endif
    /* put_buf(buf, 128); */
}

void handle_rx(void *buf, u16 len)
{
    /* putchar('r'); */
#if 1
    put_buf(buf, 64);
#else
    s16 *data_rx = (s16 *)buf;
    s16 *data_b = (s16 *)data_buf;
    for (int i = 0; i < len / sizeof(s16) ; i++) {
        data_b[i] = data_rx[i];
    }
#endif
}

ALINK_PARM	test_alink = {
    .mclk_io = IO_PORTA_12,
    .sclk_io = IO_PORTA_13,
    .lrclk_io = IO_PORTA_14,
    .ch_cfg[0].data_io = IO_PORTA_15,
    /* .ch_cfg[2].data_io = IO_PORTB_09, */
    .mode = ALINK_MD_IIS_LALIGN,
    /* .mode = ALINK_MD_IIS, */
    /* .role = ALINK_ROLE_SLAVE, */
    .role = ALINK_ROLE_MASTER,
    /* .clk_mode = ALINK_CLK_RAISE_UPDATE_FALL_SAMPLE, */
    .clk_mode = ALINK_CLK_FALL_UPDATE_RAISE_SAMPLE,
    .bitwide = ALINK_LEN_16BIT,
    /* .bitwide = ALINK_LEN_24BIT, */
    .sclk_per_frame = ALINK_FRAME_32SCLK,
    /* .dma_len = ALNK_BUF_POINTS_NUM * 2 * 2 , */
    .dma_len = 4 * 1024,
    .sample_rate = 16000,
    /* .sample_rate = TCFG_IIS_SAMPLE_RATE, */
    /* .buf_mode = ALINK_BUF_CIRCLE, */
    .buf_mode = ALINK_BUF_DUAL,
};



int audio_link_ch_start(u8 ch_idx);
void audio_link_test(void)
{
    alink_init(&test_alink);
    /* alink_channel_init(0, ALINK_DIR_RX, handle_rx); */
    alink_channel_init(0, ALINK_DIR_TX, handle_tx);
    /* alink_isr_en(1); */
    alink_start();

    alink_info_dump();
    while (1) {
        clr_wdt();
    }
}


static u8 alink_init_cnt = 0;
void audio_link_init(void)
{
    if (alink_init_cnt == 0) {
        alink_init(&test_alink);
        alink_start();
    }
    alink_init_cnt++;
}

void audio_link_uninit(void)
{
    alink_init_cnt--;
    if (alink_init_cnt == 0) {
        alink_uninit();
    }
}

#endif

#if 0

#include "asm/audio_sync_resample.h"
struct audio_iis_hdl {
    s16 *output_buf;
    u16 output_buf_len;
    u8 state;
    u8 sync_step;
    u8 channel;
    u32 sample_rate;
    u32 prepare_points;
    u32 start_points;
    u32 delay_points;
    void *resample_ch;
    void *resample_buf;
    int resample_buf_len;
} iis_hdl;

#define PNS_NUM         (441)
#define START_MS        (30)
#define MAX_MS          (50)
#define START_POINTS    (START_MS*44100/1000/2)
#define MAX_POINTS      (MAX_MS*44100/1000/2)

#define IIS_STATE_INIT        0
#define IIS_STATE_OPEN        1
#define IIS_STATE_START       2
#define IIS_STATE_PREPARE     3
#define IIS_STATE_STOP        4
#define IIS_STATE_PAUSE       5
#define IIS_STATE_POWER_OFF   6


static void audio_iis_release_fifo(void *_iis)
{
    struct audio_iis_hdl *iis = (struct audio_iis_hdl *)_iis;

    JL_ALNK0->SHN0 = iis->prepare_points;
}

int audio_link_ch_start(u8 ch_idx)
{
    printf(">>>> IIS_STATE_START\n");

    struct audio_iis_hdl *iis = &iis_hdl;

    if (iis->prepare_points) {
        if (iis->sync_step == 2 && iis->resample_ch) {
            audio_sync_resample_wait_bt(iis->resample_ch, iis, audio_iis_release_fifo);
            if (iis->prepare_points) {
                audio_sync_resample_event_start(iis->resample_ch);
            } else {
                audio_sync_resample_miss_data(iis->resample_ch);
            }
        } else {
            audio_iis_release_fifo(iis);
        }
        __asm_csync();
    }
    if (p_alink_parm->mode > ALINK_MD_IIS_RALIGN) {
        ALINK_CHx_MODE_SEL((p_alink_parm->mode - ALINK_MD_IIS_RALIGN), ch_idx);
    } else {
        ALINK_CHx_MODE_SEL(p_alink_parm->mode, ch_idx);
    }
    iis_hdl.state = IIS_STATE_START;
    return 0;
}

int audio_link_ch_stop(u8 ch_idx)
{
    printf(">>>> IIS_STATE_STOP\n");
    ALINK_CHx_MODE_SEL(ALINK_MD_NONE, ch_idx);
    iis_hdl.state = IIS_STATE_PREPARE;
    return 0;
}

/* int audio_link_ch_write(u8 ch_idx, void *buf, u32 len) */
/* { */
/* s16 *fifo_addr; */
/* u32 free_space; */
/* int wlen = 0; */
/* u32 offset = 0; */
/* u16 swp; */
/* void *wp_addr; */

/* swp = JL_ALNK0->SWPTR0; */
/* wp_addr = (void *)(JL_ALNK0->ADR0 + swp * 2 * 2); */

/* u32 free_points = JL_ALNK0->SHN0; */
/* u32 data_points = JL_ALNK0->LEN - free_points - 1; */
/* if (swp + free_points > JL_ALNK0->LEN) { */
/* free_points = JL_ALNK0->LEN - swp; */
/* } */

/* free_space =  free_points * 2 * 2; */

/* if (free_space == 0) { */
/* return 0; */
/* } */

/* if (free_space > len) { */
/* free_space = len; */
/* } */

/* memcpy(wp_addr, buf, free_space); */

/* JL_ALNK0->SHN0 = free_space / 2 / 2; */

/* if (iis_state == IIS_STATE_PREPARE \ */
/* || iis_state == IIS_STATE_OPEN \ */
/* || iis_state == IIS_STATE_INIT \ */
/* || iis_state == IIS_STATE_STOP \ */
/* ) { */
/* if ((data_points + free_space / 2 / 2) > START_POINTS) { */
/* audio_link_ch_start(0); */
/* } */
/* } */

/* return free_space; */
/* } */


/* #endif */

int audio_iis_set_irq_time(struct audio_iis_hdl *iis, int time_ms)
{
    if (!iis || iis->state != IIS_STATE_START || JL_ALNK0->SHN0 == (JL_ALNK0->LEN - 1)) {
        return -EINVAL;
    }
    int irq_points = iis->sample_rate / (1000 / time_ms);
    u16 pns = (JL_ALNK0->LEN - JL_ALNK0->SHN0) - irq_points;
    if (pns < JL_ALNK0->PNS) {
        /*已开启中断*/
        return 0;
    }
    local_irq_disable();
    JL_ALNK0->PNS = pns;
    alink_isr_en(1);
    local_irq_enable();
    return 0;
}

int audio_iis_irq_enable(struct audio_iis_hdl *iis, int time_ms, void *priv, void (*callback)(void *))
{
    int err = -EINVAL;

    local_irq_disable();
    if (iis->sync_step == 2 && iis->resample_ch) {
        err = audio_sync_resample_wait_irq_callback(iis->resample_ch, priv, callback);
    }
    local_irq_enable();

    if (err) {
        err = audio_iis_set_irq_time(iis, time_ms);
    }

    return err;
}


int audio_iis_get_write_ptr(struct audio_iis_hdl *iis, s16 **ptr)
{
    s16 free_points = 0;
    u16 swp = 0;
    int free_space = 0;

    if (iis->state == IIS_STATE_PREPARE) {
        if (JL_ALNK0->HWPTR0 != JL_ALNK0->SWPTR0) {
            JL_ALNK0->HWPTR0 = JL_ALNK0->SWPTR0;
            __asm_csync();
        }
        free_points = JL_ALNK0->SHN0 - iis->prepare_points;
        swp = JL_ALNK0->SWPTR0 + iis->prepare_points;
        if (swp >= JL_ALNK0->LEN) {
            swp -= JL_ALNK0->LEN;
        }
    } else {
        free_points = JL_ALNK0->SHN0;
        swp = JL_ALNK0->SWPTR0;
    }


    /* u16 min_free_points = JL_ALNK0->LEN - iis->delay_points; */

    /* printf("g:%d %d\n", free_points, iis->delay_points); */
    /* if (free_points <= min_free_points) { */
    /* return 0; */
    /* } */

    /* free_points -= min_free_points; */

    if (swp + free_points > JL_ALNK0->LEN) {
        free_points = JL_ALNK0->LEN - swp;
    }

    u8 bit_mode = 1;
    /* if (iis->bit24_mode_en) { */
    /* bit_mode = 2; */
    /* } */

    /* if (iis->channel_mode == MONO_TO_DUAL) { */
    /* *ptr = (void *)(JL_ALNK0->ADR + swp * 4 * bit_mode); */
    /* free_space = free_points * 2 * iis->channel / 2 * bit_mode; */
    /* } else { */
    *ptr = (void *)(JL_ALNK0->ADR0 + swp * 2 * iis->channel * bit_mode);
    free_space = free_points * 2 * iis->channel * bit_mode;
    /* } */

    return free_space;
}



static int audio_iis_get_cbuf_write_ptr(struct audio_iis_hdl *iis, s16 **ptr)
{
    s16 free_points = 0;
    u16 swp = 0;
    int free_space = 0;

    if (iis->state == IIS_STATE_PREPARE) {
        free_points = JL_ALNK0->SHN0 - iis->prepare_points;
        swp = JL_ALNK0->SWPTR0 + iis->prepare_points;
        if (swp >= JL_ALNK0->LEN) {
            swp -= JL_ALNK0->LEN;
        }
    } else {
        free_points = JL_ALNK0->SHN0;
        swp = JL_ALNK0->SWPTR0;
    }

    /* u16 min_free_points = JL_ALNK0->LEN - iis->delay_points; */
    /* if (free_points <= min_free_points) { */
    /* return 0; */
    /* } */

    /* free_points -= min_free_points; */

    u8 bit_mode = 1;
    /* if (iis->bit24_mode_en) { */
    /* bit_mode = 2; */
    /* } */

    /* if (iis->channel_mode == MONO_TO_DUAL) { */
    /* *ptr = (void *)(JL_ALNK0->ADR + swp * 4 * bit_mode); */
    /* free_space = free_points * 2 * iis->channel / 2 * bit_mode; */
    /* } else { */
    *ptr = (void *)(JL_ALNK0->ADR0 + swp * 2 * iis->channel * bit_mode);
    free_space = free_points * 2 * iis->channel * bit_mode;
    /* } */

    return free_space;
}


int audio_iis_update_write_ptr(struct audio_iis_hdl *iis, int len)
{
    /* printf("u:%d\n", len); */
    u32 wp_addr;
    u16 swp;
    u16 data_size = 0;
    u16 finish_points = 0;

    u8 bit_mode = 1;
    /* if (iis->bit24_mode_en) { */
    /* bit_mode = 2; */
    /* } */

    /* if (iis->channel_mode == MONO_TO_DUAL) { */
    /* finish_points = len / 2 / bit_mode; */
    /* } else { */
    finish_points = len / 2 / iis->channel / bit_mode;
    /* } */

    if (iis->state != IIS_STATE_PREPARE && iis->state != IIS_STATE_START) {
        return 0;
    }

    if (iis->state == IIS_STATE_PREPARE) {
        data_size = iis->prepare_points;
        swp = JL_ALNK0->SWPTR0 + iis->prepare_points;
        if (swp >= JL_ALNK0->LEN) {
            swp -= JL_ALNK0->LEN;
        }
    } else {
        data_size = JL_ALNK0->LEN - JL_ALNK0->SHN0;
        swp = JL_ALNK0->SWPTR0;
        if (data_size <= 1) {
            if (iis->sync_step == 2 && iis->resample_ch) {
                if (audio_sync_resample_miss_data(iis->resample_ch)) {
                    return len;
                }
            }
        }

    }

    /* if (iis->channel_mode != NORMAL_STEREO_CHANNEL) { */
    /* u32 wp_addr; */
    /* switch (iis->channel_mode) { */
    /* case MONO_TO_DUAL: */
    /* case SWITCH_STEREO_LR: */
    /* wp_addr = JL_AUDIO->iis_ADR + swp * 4 * bit_mode; */
    /* break; */
    /* default: */
    /* wp_addr = JL_AUDIO->iis_ADR + swp * 2 * iis->channel * bit_mode; */
    /* break; */
    /* } */

    /* if (swp + finish_points > JL_AUDIO->iis_LEN) { */
    /* __multi_channel_mode_handler((s16 *)wp_addr, (s16 *)wp_addr, JL_AUDIO->iis_LEN - swp, iis->channel_mode, iis->bit24_mode_en); */
    /* __multi_channel_mode_handler((s16 *)JL_AUDIO->iis_ADR, (s16 *)JL_AUDIO->iis_ADR, swp + finish_points - JL_AUDIO->iis_LEN, iis->channel_mode, iis->bit24_mode_en); */
    /* } else { */
    /* __multi_channel_mode_handler((s16 *)wp_addr, (s16 *)wp_addr, finish_points, iis->channel_mode, iis->bit24_mode_en); */
    /* } */
    /* } */

    local_irq_disable();

    if (iis->state == IIS_STATE_START) {
        JL_ALNK0->SHN0 = finish_points;
        __asm_csync(); //fix : 系统频率较高时，SWN写入后未更新到硬件，同步模块立刻锁存会丢失本次信息
    } else {
        iis->prepare_points += finish_points;
    }


    local_irq_enable();

    /* printf("> %d %d %d\n", iis->state, iis->prepare_points, iis->start_points); */
    if (iis->state == IIS_STATE_PREPARE) {
        if (iis->prepare_points >= iis->start_points) {
            /* iis_module_on(iis); */
            audio_link_ch_start(0);
        }
    }

    return len;
}

static int __audio_iis_write(struct audio_iis_hdl *iis, void *buf, int len)
{
    /* printf("w:%d\n", len); */
    int r_len = len;
    s16 *fifo_addr;
    u32 free_space;
    int wlen = 0;
    u32 offset = 0;
    u16 swp;
    void *wp_addr;

    if (iis->state == IIS_STATE_PREPARE) {
        swp = JL_ALNK0->SWPTR0 + iis->prepare_points;
    } else {
        swp = JL_ALNK0->SWPTR0;
    }

    u8 bit_mode = 1;
    /* if (iis->bit24_mode_en) { */
    /* bit_mode = 2; */
    /* } */

    /* if (iis->channel_mode == MONO_TO_DUAL) { */
    /* wp_addr = (void *)(JL_ALNK0->ADR0 + swp * 4 * bit_mode); */
    /* } else { */
    wp_addr = (void *)(JL_ALNK0->ADR0 + swp * 2 * iis->channel * bit_mode);
    /* } */

    /* putchar('1'); */
    if ((u32)buf == (u32)wp_addr) {
        /* putchar('3'); */
        return audio_iis_update_write_ptr(iis, len);
    }

    /* putchar('5'); */
    free_space = audio_iis_get_write_ptr(iis, &fifo_addr);
    /* putchar('7'); */
    /* printf("free_space:%d\n", free_space); */
    if (free_space == 0) {
        /* putchar('a'); */
        return 0;
    }
    if (free_space > len) {
        free_space = len;
    }

    /* putchar('b'); */
    memcpy(fifo_addr, buf, free_space);
    audio_iis_update_write_ptr(iis, free_space);
    wlen += free_space;
    /* putchar('c'); */

    if (free_space < len) {
        len -= free_space;
        /* putchar('d'); */
        free_space = audio_iis_get_write_ptr(iis, &fifo_addr);
        /* putchar('e'); */
        if (free_space == 0) {
            /* putchar('f'); */
            return wlen;
        }
        if (free_space > len) {
            /* putchar('g'); */
            free_space = len;
        }

        /* putchar('h'); */
        memcpy(fifo_addr, (u8 *)buf + wlen, free_space);
        audio_iis_update_write_ptr(iis, free_space);
        wlen += free_space;
        len -= free_space;
    }

    return wlen;
}

void audio_iis_init(void)
{
    struct audio_iis_hdl *iis = &iis_hdl;
    iis->state = IIS_STATE_PREPARE;
    iis->channel = 2;
    iis->sample_rate = 44100;
    iis->start_points = START_POINTS;
    iis->output_buf = (s16 *)JL_ALNK0->ADR0;
    iis->output_buf_len = JL_ALNK0->LEN * 2 * 2;
    alink_isr_en(0);
}

static int audio_iis_resample_output_alloc(void *_iis, s16 **ptr)
{
    /* putchar('R'); */
    struct audio_iis_hdl *iis = (struct audio_iis_hdl *)_iis;

    return audio_iis_get_cbuf_write_ptr(iis, ptr);
}

static void audio_iis_resample_output_free(void *_iis, s16 *addr, int len)
{
    /* putchar('O'); */
    struct audio_iis_hdl *iis = (struct audio_iis_hdl *)_iis;

    audio_iis_update_write_ptr(iis, len);
}

int audio_iis_write(struct audio_iis_hdl *iis, void *buf, int len)
{
    /* printf(">> www\n"); */
    int wlen = 0;

    /* putchar('#'); */
    if (iis->sync_step == 1 && iis->resample_ch) {
        putchar('*');
        iis->sync_step = 2;
        audio_sync_resample_set_extern_output(iis->resample_ch,
                                              iis,
                                              audio_iis_resample_output_alloc,
                                              audio_iis_resample_output_free);
        audio_sync_resample_set_in_buffer(iis->resample_ch, iis->resample_buf, iis->resample_buf_len);
        audio_sync_resample_set_cbuf_addr(iis->resample_ch, iis->output_buf, iis->output_buf_len);
    }

    if (iis->resample_ch && iis->sync_step) {
        /* putchar('$'); */
        return audio_sync_resample_write(iis->resample_ch, buf, len);
    } else {
        /* printf(">> ---\n"); */
        /* putchar('='); */
        return __audio_iis_write(iis, buf, len);
    }
    return 0;
}

int audio_iis_data_time(struct audio_iis_hdl *iis)
{
    if (!iis) {
        return 0;
    }

    int da_buf_time = (((JL_ALNK0->LEN - JL_ALNK0->SHN0 - 1) * 1000000) / iis->sample_rate) / 1000;

    if (iis->sync_step == 2 && iis->resample_ch) {
        int sync_buf_len = 0;
        sync_buf_len = audio_sync_resample_all_samples(iis->resample_ch);
        da_buf_time += ((sync_buf_len * 1000000) / iis->sample_rate) / 1000;
    }

    return da_buf_time;
}

/*
 * DA同步需要的输入缓冲buffer设置
 */
int audio_iis_set_sync_buff(struct audio_iis_hdl *iis, void *buf, int len)
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
    iis->resample_buf = buf;
    iis->resample_buf_len = len;
    return 0;
}

int audio_iis_sync_resample_enable(struct audio_iis_hdl *iis, void *resample)
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
    iis->resample_ch = resample;
    return 0;
}

int audio_iis_sync_resample_disable(struct audio_iis_hdl *iis, void *resample)
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
    if (iis->resample_ch == resample) {
        iis->resample_ch = NULL;
        iis->sync_step = 0;
    }
    return 0;
}

int audio_iis_sync_start(struct audio_iis_hdl *iis)
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
    iis->sync_step = 1;

    return 0;
}

int audio_iis_sync_stop(struct audio_iis_hdl *iis)
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
    iis->sync_step = 0;
    return 0;
}

void *audio_iis_resample_channel(struct audio_iis_hdl *iis)
{
    printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %s %d\n", __func__, __LINE__);
    return iis->resample_ch;
}
#endif
