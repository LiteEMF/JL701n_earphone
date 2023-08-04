#ifndef __IIS_H__
#define __IIS_H__

#include "generic/typedef.h"

#define ALNK_CON_RESET()	do {JL_ALNK0->CON0 = 0; JL_ALNK0->CON1 = 0; JL_ALNK0->CON2 = 0; JL_ALNK0->CON3 = 0;} while(0)
#define ALNK_HWPTR_RESET()	do {JL_ALNK0->HWPTR0 = 0; JL_ALNK0->HWPTR1 = 0; JL_ALNK0->HWPTR2 = 0; JL_ALNK0->HWPTR3 = 0;} while(0)
#define ALNK_SWPTR_RESET()	do {JL_ALNK0->SWPTR0 = 0; JL_ALNK0->SWPTR1 = 0; JL_ALNK0->SWPTR2 = 0; JL_ALNK0->SWPTR3 = 0;} while(0)
#define ALNK_SHN_RESET()	do {JL_ALNK0->SHN0 = 0; JL_ALNK0->SHN1 = 0; JL_ALNK0->SHN2 = 0; JL_ALNK0->SHN3 = 0;} while(0)
#define ALNK_ADR_RESET()	do {JL_ALNK0->ADR0 = 0; JL_ALNK0->ADR1 = 0; JL_ALNK0->ADR2 = 0; JL_ALNK0->ADR3 = 0;} while(0)
#define ALNK_PNS_RESET()	SFR(JL_ALNK0->PNS, 0, 32, 0)

#define	ALINK_DMA_MODE_SEL(x)	SFR(JL_ALNK0->CON0, 2, 1, x)
#define ALINK_DSPE(x)		SFR(JL_ALNK0->CON0, 6, 1, x)
#define ALINK_SOE(x)		SFR(JL_ALNK0->CON0, 7, 1, x)
#define ALINK_MOE(x)		SFR(JL_ALNK0->CON0, 8, 1, x)
#define F32_EN(x)           SFR(JL_ALNK0->CON0, 9, 1, x)
#define SCLKINV(x)         	SFR(JL_ALNK0->CON0,10, 1, x)
#define ALINK_EN(x)         SFR(JL_ALNK0->CON0,11, 1, x)
#define ALINK_24BIT_MODE()	(JL_ALNK0->CON1 |= (BIT(2) | BIT(6) | BIT(10) | BIT(14)))
#define ALINK_16BIT_MODE()	(JL_ALNK0->CON1 &= (~(BIT(2) | BIT(6) | BIT(10) | BIT(14))))

#define ALINK_IIS_MODE()	(JL_ALNK0->CON1 |= (BIT(2) | BIT(6) | BIT(10) | BIT(14)))

#define ALINK_CHx_DIR_TX_MODE(ch)	(JL_ALNK0->CON1 &= (~(1 << (3 + 4 * ch))))
#define ALINK_CHx_DIR_RX_MODE(ch)	(JL_ALNK0->CON1 |= (1 << (3 + 4 * ch)))

#define ALINK_CHx_MODE_SEL(val, ch)		(JL_ALNK0->CON1 |= (val << (0 + 4 * ch)))
#define ALINK_CHx_CLOSE(val, ch)		(JL_ALNK0->CON1 &= (~(3 << (0 + 4 * ch))))

#define ALINK_CLR_CH0_PND()		(JL_ALNK0->CON2 |= BIT(0))
#define ALINK_CLR_CH1_PND()		(JL_ALNK0->CON2 |= BIT(1))
#define ALINK_CLR_CH2_PND()		(JL_ALNK0->CON2 |= BIT(2))
#define ALINK_CLR_CH3_PND()		(JL_ALNK0->CON2 |= BIT(3))
#define ALINK_CLR_ALL_PND()		SFR(JL_ALNK0->CON2, 0, 4, 0xF)

#define ALINK_CLR_CHx_PND(ch)	(JL_ALNK0->CON2 |= BIT(ch))
#define ALINK_CHx_IE(ch, x)		SFR(JL_ALNK0->CON2, ch + 12, 1, x)

#define ALINK_OPNS_SET(x)		SFR(JL_ALNK0->PNS, 16, 16, x)
#define ALINK_IPNS_SET(x)		SFR(JL_ALNK0->PNS, 0, 16, x)

#define ALINK_MSRC(x)		SFR(JL_ALNK0->CON3, 0, 2, x)
#define ALINK_MDIV(x)		SFR(JL_ALNK0->CON3, 2, 3, x)
#define ALINK_LRDIV(x)		SFR(JL_ALNK0->CON3, 5, 3, x)

#define PLL_CLK_F2_OE(x)     SFR(JL_PLL->PLL_CON3, 1, 1,  x)
#define PLL_CLK_F3_OE(x)     SFR(JL_PLL->PLL_CON3, 2, 1,  x)
#endif
