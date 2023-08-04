/*********************************************************************************************
    *   Filename        : p11_app.h

    *   Description     : 本文件基于p11.h文件封装应用的接口

    *   Author          : MoZhiYe

    *   Email           : mozhiye@zh-jieli.com

    *   Last modifiled  : 2021-04-19 09:00

    *   Copyright:(c)JIELI  2021-2029  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef __P11_APP_H__
#define __P11_APP_H__

#include "p11.h"

/*

 _______________ <-----P11 Message Acess End
| poweroff boot |
|_______________|
| m2p msg(0x40) |
|_______________|
| p2m msg(0x40) |
|_______________|<-----P11 Message Acess Begin
|               |
|    p11 use    |
|_______________|__

*/
#define P11_RAM_BASE 				0xF20000
#define P11_RAM_SIZE 				(0x8000)
#define P11_RAM_END 				(P11_RAM_BASE + P11_RAM_SIZE)

#define P11_POWEROFF_RAM_SIZE (0x14 + 0xc)
#define P11_POWEROFF_RAM_BEGIN  (P11_RAM_END - P11_POWEROFF_RAM_SIZE)

#define P2M_MESSAGE_SIZE 			0x40
#define M2P_MESSAGE_SIZE 			0x60

#define M2P_MESSAGE_RAM_BEGIN 		(P11_POWEROFF_RAM_BEGIN - M2P_MESSAGE_SIZE)
#define P2M_MESSAGE_RAM_BEGIN 		(M2P_MESSAGE_RAM_BEGIN - P2M_MESSAGE_SIZE)

#define P11_MESSAGE_RAM_BEGIN 		(P2M_MESSAGE_RAM_BEGIN)

#define P11_RAM_ACCESS(x)   		(*(volatile u8 *)(x))

#define P2M_MESSAGE_ACCESS(x)      	P11_RAM_ACCESS(P2M_MESSAGE_RAM_BEGIN + x)
#define M2P_MESSAGE_ACCESS(x)      	P11_RAM_ACCESS(M2P_MESSAGE_RAM_BEGIN + x)

//==========================================================//
//                       P11_VAD_RAM                        //
//==========================================================//
//-------------------------- VAD CBUF-----------------------//
#define VAD_POINT_PER_FRAME 				(160)
#define VAD_FRAME_SIZE 						(160 * 2)
#define VAD_CBUF_FRAME_CNT 					(6)
#define VAD_CBUF_TAG_SIZE 					(0)
#define VAD_CBUF_FRAME_SIZE 				(VAD_FRAME_SIZE +  VAD_CBUF_TAG_SIZE)
#define CONFIG_P11_CBUF_SIZE 				(VAD_CBUF_FRAME_SIZE * VAD_CBUF_FRAME_CNT)
#define VAD_CBUF_END 						(P11_MESSAGE_RAM_BEGIN - 0x20)
#define VAD_CBUF_BEGIN 						(VAD_CBUF_END - CONFIG_P11_CBUF_SIZE)

//------------------------ VAD CONFIG-----------------------//
#define CONFIG_P2M_AVAD_CONFIG_SIZE 		(20 * 4) //sizeof(int)
#define CONFIG_P2M_DVAD_CONFIG_SIZE 		(20 * 4) //sizeof(int)
#define CONFIG_VAD_CONFIG_SIZE 				(CONFIG_P2M_AVAD_CONFIG_SIZE + CONFIG_P2M_DVAD_CONFIG_SIZE)
#define VAD_AVAD_CONFIG_BEGIN 				(VAD_CBUF_BEGIN - CONFIG_P2M_AVAD_CONFIG_SIZE)
#define VAD_DVAD_CONFIG_BEGIN 				(VAD_AVAD_CONFIG_BEGIN - CONFIG_P2M_DVAD_CONFIG_SIZE)

#define P11_HEAP_BEGIN 						(P11_RAM_BASE + ((P2M_P11_HEAP_BEGIN_ADDR_H << 8) | P2M_P11_HEAP_BEGIN_ADDR_L))
#define P11_HEAP_SIZE 						((P2M_P11_HEAP_SIZE_H << 8) | P2M_P11_HEAP_SIZE_L)

#define P11_RAM_PROTECT_END 				(P11_HEAP_BEGIN)


#define P11_PWR_CON				   	P11_CLOCK->PWR_CON

/*
 *------------------- P11_CLOCK->CLK_CON
 */

#define P11_CLK_CON0 				P11_CLOCK->CLK_CON0
enum P11_SYS_CLK_TABLE {
    P11_SYS_CLK_RC250K = 0,
    P11_SYS_CLK_RC16M,
    P11_SYS_CLK_LRC_OSC,
    P11_SYS_CLK_BTOSC_24M,
    P11_SYS_CLK_BTOSC_48M,
    P11_SYS_CLK_PLL_SYS_CLK,
    P11_SYS_CLK_CLK_X2,
};
//#define P11_SYS_CLK_SEL(x) 			SFR(P11_CLOCK->CLK_CON0, 0, 3, x)
#define P11_SYS_CLK_SEL(x) 			(P11_CLOCK->CLK_CON0 = x)


//p11 btosc use d2sh
#define CLOCK_KEEP(en)							  \
	if(en){										  \
		P11_CLOCK->CLK_CON1 &= ~(3<<15);		  \
		P11_CLOCK->CLK_CON1 |= BIT(14);			  \
		P11_CLOCK->CLK_CON1 |= (2<<15);			  \
	}else{  									  \
		P11_CLOCK->CLK_CON1 &= ~(3<<15);		  \
		P11_CLOCK->CLK_CON1 &= ~BIT(14);		  \
	}

#define P11_P2M_CLK_CON0 			P11_SYSTEM->P2M_CLK_CON0
#define P11_SYSTEM_CON0 		   	P11_SYSTEM->P11_SYS_CON0
#define P11_SYSTEM_CON1 		   	P11_SYSTEM->P11_SYS_CON1
#define P11_P11_SYS_CON0  			P11_SYSTEM_CON0
#define P11_P11_SYS_CON1  			P11_SYSTEM_CON1
#define P11_RST_SRC					P11_CLOCK->RST_SRC

#define LED_CLK_SEL(x)             P11_SYSTEM->P2M_CLK_CON0 = ((P11_SYSTEM->P2M_CLK_CON0 & ~0xe0) | (x) << 5)
#define GET_LED_CLK_SEL(x)         (P11_SYSTEM->P2M_CLK_CON0 & 0xe0)

#define P11_WDT_CON 				P11_WDT->CON

#define P11_P2M_INT_IE 			   	P11_SYSTEM->P2M_INT_IE
#define P11_M2P_INT_IE 			   	P11_SYSTEM->M2P_INT_IE
#define P11_M2P_INT_SET 			P11_SYSTEM->M2P_INT_SET
#define P11_P2M_INT_SET 			P11_SYSTEM->P2M_INT_SET
#define P11_P2M_INT_CLR 			P11_SYSTEM->P2M_INT_CLR
#define P11_P2M_INT_PND 			P11_SYSTEM->P2M_INT_PND //?
#define P11_M2P_INT_PND 			P11_SYSTEM->M2P_INT_PND //?

#define P11_TMR0_CON0 			   P11_LPTMR0->CON0
#define P11_TMR0_CON1 			   P11_LPTMR0->CON1
#define P11_TMR0_CON2 			   P11_LPTMR0->CON2
#define P11_TMR0_CNT 			   P11_LPTMR0->CNT
#define P11_TMR0_PRD 			   P11_LPTMR0->PRD
#define P11_TMR0_RSC 			   P11_LPTMR0->RSC

#define P11_TMR1_CON0 			   P11_LPTMR1->CON0
#define P11_TMR1_CON1 			   P11_LPTMR1->CON1
#define P11_TMR1_CON2 			   P11_LPTMR1->CON2
#define P11_TMR1_CNT 			   P11_LPTMR1->CNT
#define P11_TMR1_PRD 			   P11_LPTMR1->PRD
#define P11_TMR1_RSC 			   P11_LPTMR1->RSC

#define GET_P11_SYS_RST_SRC()	   P11_RST_SRC

#define LP_PWR_IDLE(x)             SFR(P11_PWR_CON, 0, 1, x)
#define LP_PWR_STANDBY(x)          SFR(P11_PWR_CON, 1, 1, x)
#define LP_PWR_SLEEP(x)            SFR(P11_PWR_CON, 2, 1, x)
#define LP_PWR_SSMODE(x)           SFR(P11_PWR_CON, 3, 1, x)
#define LP_PWR_SOFT_RESET(x)       SFR(P11_PWR_CON, 4, 1, x)
#define LP_PWR_INIT_FLAG()         (P11_PWR_CON & BIT(5))
#define LP_PWR_RST_FLAG_CLR(x)     SFR(P11_PWR_CON, 6, 1, x)
#define LP_PWR_RST_FLAG()          (P11_PWR_CON & BIT(7))

#define P33_TEST_ENABLE()           P11_P11_SYS_CON0 |= BIT(5)
#define P33_TEST_DISABLE()          P11_P11_SYS_CON0 &= ~BIT(5)

#define P11_TX_DISABLE(x)           P11_SYSTEM->P11_SYS_CON1 |= BIT(2)
#define P11_TX_ENABLE(x)            P11_SYSTEM->P11_SYS_CON1 &= ~BIT(2)

#define MSYS_IO_LATCH_ENABLE()      P11_SYSTEM->P11_SYS_CON1 |= BIT(7)
#define MSYS_IO_LATCH_DISABLE()     P11_SYSTEM->P11_SYS_CON1 &= ~BIT(7)

#define LP_TMR0_EN(x)               SFR(P11_TMR0_CON0, 0, 1, x)
#define LP_TMR0_CTU(x)              SFR(P11_TMR0_CON0, 1, 1, x)
#define LP_TMR0_P11_WKUP_IE(x)      SFR(P11_TMR0_CON0, 2, 1, x)
#define LP_TMR0_P11_TO_IE(x)        SFR(P11_TMR0_CON0, 3, 1, x)
#define LP_TMR0_CLR_P11_WKUP(x)     SFR(P11_TMR0_CON0, 4, 1, x)
#define LP_TMR0_P11_WKUP(x)        (P11_TMR0_CON0 & BIT(5))
#define LP_TMR0_CLR_P11_TO(x)       SFR(P11_TMR0_CON0, 6, 1, x)
#define LP_TMR0_P11_TO(x)          (P11_TMR0_CON0 & BIT(7))

#define LP_TMR0_SW_KICK_START_EN(x)   SFR(P11_TMR0_CON1, 0, 1, x)
#define LP_TMR0_HW_KICK_START_EN(x)   SFR(P11_TMR0_CON1, 1, 1, x)
#define LP_TMR0_WKUP_IE(x)         SFR(P11_TMR0_CON1, 2, 1, x)
#define LP_TMR0_TO_IE(x)           SFR(P11_TMR0_CON1, 3, 1, x)
#define LP_TMR0_CLR_MSYS_WKUP(x)   SFR(P11_TMR0_CON1, 4, 1, x)
#define LP_TMR0_MSYS_WKUP(x)       (P11_TMR0_CON1 & BIT(5))
#define LP_TMR0_CLR_MSYS_TO(x)     SFR(P11_TMR0_CON1, 6, 1, x)
#define LP_TMR0_MSYS_TO(x)         (P11_TMR0_CON1 & BIT(7))

#define LP_TMR0_CLK_SEL(x)         SFR(P11_TMR0_CON2, 0, 4, x)
#define LP_TMR0_CLK_DIV(x)         SFR(P11_TMR0_CON2, 4, 4, x)
#define LP_TMR0_KST(x)             SFR(P11_TMR0_CON2, 8, 1, x)
#define LP_TMR0_RUN()              (P11_TMR0_CON2 & BIT(9))

#define P11_M2P_RESET_MASK(x)      SFR(P11_P11_SYS_CON1 , 4, 1, x)

//MEM_PWR_CON
#define	MEM_PWR_CPU_CON	BIT(0)
#define MEM_PWR_RAM0_RAM3_CON BIT(1)
#define MEM_PWR_RAM4_RAM5_CON BIT(2)
#define MEM_PWR_RAM6_RAM7_CON BIT(3)
#define MEM_PWR_RAM8_RAM9_CON BIT(4)
#define MEM_PWR_PERIPH_CON  BIT(5)

#define MEM_PWR_RAM_SET(a)   (((1 << a) - 1) - 1)



//Mapping to P11
//===========================================================================//
//                              P2M MESSAGE TABLE                            //
//===========================================================================//

#define P2M_WKUP_SRC                                    P2M_MESSAGE_ACCESS(0x000)
#define P2M_WKUP_PND0                                   P2M_MESSAGE_ACCESS(0x001)
#define P2M_WKUP_PND1                                   P2M_MESSAGE_ACCESS(0x002)
#define P2M_REPLY_COMMON_CMD                            P2M_MESSAGE_ACCESS(0x003)
#define P2M_MESSAGE_VAD_CMD                             P2M_MESSAGE_ACCESS(0x004)
#define P2M_MESSAGE_VAD_CBUF_WPTR                       P2M_MESSAGE_ACCESS(0x005)
#define P2M_MESSAGE_BANK_ADR_L                          P2M_MESSAGE_ACCESS(0x006)
#define P2M_MESSAGE_BANK_ADR_H                          P2M_MESSAGE_ACCESS(0x007)
#define P2M_MESSAGE_BANK_INDEX                          P2M_MESSAGE_ACCESS(0x008)
#define P2M_MESSAGE_BANK_ACK                            P2M_MESSAGE_ACCESS(0x009)
#define P2M_P11_HEAP_BEGIN_ADDR_L    					P2M_MESSAGE_ACCESS(0x00A)
#define P2M_P11_HEAP_BEGIN_ADDR_H    					P2M_MESSAGE_ACCESS(0x00B)
#define P2M_P11_HEAP_SIZE_L    							P2M_MESSAGE_ACCESS(0x00C)
#define P2M_P11_HEAP_SIZE_H    							P2M_MESSAGE_ACCESS(0x00D)

#define P2M_CTMU_KEY_EVENT                              P2M_MESSAGE_ACCESS(0x010)
#define P2M_CTMU_KEY_CNT                                P2M_MESSAGE_ACCESS(0x011)
#define P2M_CTMU_WKUP_MSG                               P2M_MESSAGE_ACCESS(0x012)
#define P2M_CTMU_EARTCH_EVENT                           P2M_MESSAGE_ACCESS(0x013)

#define P2M_MASSAGE_CTMU_CH0_L_RES                                         0x014
#define P2M_MASSAGE_CTMU_CH0_H_RES                                         0x015
#define P2M_CTMU_CH0_L_RES                              P2M_MESSAGE_ACCESS(0x014)
#define P2M_CTMU_CH0_H_RES                              P2M_MESSAGE_ACCESS(0x015)
#define P2M_CTMU_CH1_L_RES                              P2M_MESSAGE_ACCESS(0x016)
#define P2M_CTMU_CH1_H_RES                              P2M_MESSAGE_ACCESS(0x017)
#define P2M_CTMU_CH2_L_RES                              P2M_MESSAGE_ACCESS(0x018)
#define P2M_CTMU_CH2_H_RES                              P2M_MESSAGE_ACCESS(0x019)
#define P2M_CTMU_CH3_L_RES                              P2M_MESSAGE_ACCESS(0x01a)
#define P2M_CTMU_CH3_H_RES                              P2M_MESSAGE_ACCESS(0x01b)
#define P2M_CTMU_CH4_L_RES                              P2M_MESSAGE_ACCESS(0x01c)
#define P2M_CTMU_CH4_H_RES                              P2M_MESSAGE_ACCESS(0x01d)

#define P2M_CTMU_EARTCH_L_IIR_VALUE                     P2M_MESSAGE_ACCESS(0x01e)
#define P2M_CTMU_EARTCH_H_IIR_VALUE                     P2M_MESSAGE_ACCESS(0x01f)
#define P2M_CTMU_EARTCH_L_TRIM_VALUE                    P2M_MESSAGE_ACCESS(0x020)
#define P2M_CTMU_EARTCH_H_TRIM_VALUE                    P2M_MESSAGE_ACCESS(0x021)
#define P2M_CTMU_EARTCH_L_DIFF_VALUE                    P2M_MESSAGE_ACCESS(0x022)
#define P2M_CTMU_EARTCH_H_DIFF_VALUE                    P2M_MESSAGE_ACCESS(0x023)

#define P2M_AWKUP_P_PND                                 P2M_MESSAGE_ACCESS(0x024)
#define P2M_AWKUP_N_PND                                 P2M_MESSAGE_ACCESS(0x025)
#define P2M_WKUP_RTC                                    P2M_MESSAGE_ACCESS(0x026)

#define P2M_USER_PEND                                  (0x038)//传感器使用或者开放客户使用
#define P2M_USER_MSG_TYPE                              (0x039)
#define P2M_USER_MSG_LEN0                              (0x03a)
#define P2M_USER_MSG_LEN1                              (0x03b)
#define P2M_USER_ADDR0                                 (0x03c)
#define P2M_USER_ADDR1                                 (0x03d)
#define P2M_USER_ADDR2                                 (0x03e)
#define P2M_USER_ADDR3                                 (0x040)

//===========================================================================//
//                              M2P MESSAGE TABLE                            //
//===========================================================================//
#define M2P_LRC_PRD                                     M2P_MESSAGE_ACCESS(0x000)
#define M2P_MESSAGE_WVDD                                M2P_MESSAGE_ACCESS(0x001)
#define M2P_LRC_TMR_50us                                M2P_MESSAGE_ACCESS(0x002)
#define M2P_LRC_TMR_200us                               M2P_MESSAGE_ACCESS(0x003)
#define M2P_LRC_TMR_600us                               M2P_MESSAGE_ACCESS(0x004)
#define M2P_VDDIO_KEEP                                  M2P_MESSAGE_ACCESS(0x005)
#define M2P_LRC_KEEP                                    M2P_MESSAGE_ACCESS(0x006)
#define M2P_COMMON_CMD                                  M2P_MESSAGE_ACCESS(0x007)
#define M2P_MESSAGE_VAD_CMD                             M2P_MESSAGE_ACCESS(0x008)
#define M2P_MESSAGE_VAD_CBUF_RPTR                       M2P_MESSAGE_ACCESS(0x009)
#define M2P_RCH_FEQ_L                                   M2P_MESSAGE_ACCESS(0x00a)
#define M2P_RCH_FEQ_H                                   M2P_MESSAGE_ACCESS(0x00b)
#define M2P_MEM_CONTROL                                 M2P_MESSAGE_ACCESS(0x00c)
#define M2P_BTOSC_KEEP                                  M2P_MESSAGE_ACCESS(0x00d)
#define M2P_CTMU_KEEP									M2P_MESSAGE_ACCESS(0x00e)
#define M2P_RTC_KEEP                                    M2P_MESSAGE_ACCESS(0x00f)
#define M2P_WDT_SYNC                                   	M2P_MESSAGE_ACCESS(0x010)
#define M2P_LIGHT_PDOWN_DVDD_VOL                        M2P_MESSAGE_ACCESS(0x011)

/*触摸所有通道配置*/
#define M2P_CTMU_CMD  									M2P_MESSAGE_ACCESS(0x18)
#define M2P_CTMU_MSG                                    M2P_MESSAGE_ACCESS(0x19)
#define M2P_CTMU_PRD0               		            M2P_MESSAGE_ACCESS(0x1a)
#define M2P_CTMU_PRD1                                   M2P_MESSAGE_ACCESS(0x1b)
#define M2P_CTMU_CH_ENABLE								M2P_MESSAGE_ACCESS(0x1c)
#define M2P_CTMU_CH_DEBUG								M2P_MESSAGE_ACCESS(0x1d)
#define M2P_CTMU_CH_CFG						            M2P_MESSAGE_ACCESS(0x1e)
#define M2P_CTMU_CH_WAKEUP_EN					        M2P_MESSAGE_ACCESS(0x1f)
#define M2P_CTMU_EARTCH_CH						        M2P_MESSAGE_ACCESS(0x20)
#define M2P_CTMU_TIME_BASE          					M2P_MESSAGE_ACCESS(0x21)

#define M2P_CTMU_LONG_TIMEL                             M2P_MESSAGE_ACCESS(0x22)
#define M2P_CTMU_LONG_TIMEH                             M2P_MESSAGE_ACCESS(0x23)
#define M2P_CTMU_HOLD_TIMEL                             M2P_MESSAGE_ACCESS(0x24)
#define M2P_CTMU_HOLD_TIMEH                             M2P_MESSAGE_ACCESS(0x25)
#define M2P_CTMU_SOFTOFF_LONG_TIMEL                     M2P_MESSAGE_ACCESS(0x26)
#define M2P_CTMU_SOFTOFF_LONG_TIMEH                     M2P_MESSAGE_ACCESS(0x27)
#define M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_L  		M2P_MESSAGE_ACCESS(0x28)//长按复位
#define M2P_CTMU_LONG_PRESS_RESET_TIME_VALUE_H  		M2P_MESSAGE_ACCESS(0x29)//长按复位

#define M2P_CTMU_INEAR_VALUE_L                          M2P_MESSAGE_ACCESS(0x2a)
#define M2P_CTMU_INEAR_VALUE_H							M2P_MESSAGE_ACCESS(0x2b)
#define M2P_CTMU_OUTEAR_VALUE_L                         M2P_MESSAGE_ACCESS(0x2c)
#define M2P_CTMU_OUTEAR_VALUE_H                         M2P_MESSAGE_ACCESS(0x2d)
#define M2P_CTMU_EARTCH_TRIM_VALUE_L                    M2P_MESSAGE_ACCESS(0x2e)
#define M2P_CTMU_EARTCH_TRIM_VALUE_H                    M2P_MESSAGE_ACCESS(0x2f)

#define M2P_MASSAGE_CTMU_CH0_CFG0L                                         0x30
#define M2P_MASSAGE_CTMU_CH0_CFG0H                                         0x31
#define M2P_MASSAGE_CTMU_CH0_CFG1L                                         0x32
#define M2P_MASSAGE_CTMU_CH0_CFG1H                                         0x33
#define M2P_MASSAGE_CTMU_CH0_CFG2L                                         0x34
#define M2P_MASSAGE_CTMU_CH0_CFG2H                                         0x35

#define M2P_CTMU_CH0_CFG0L                              M2P_MESSAGE_ACCESS(0x30)
#define M2P_CTMU_CH0_CFG0H                              M2P_MESSAGE_ACCESS(0x31)
#define M2P_CTMU_CH0_CFG1L                              M2P_MESSAGE_ACCESS(0x32)
#define M2P_CTMU_CH0_CFG1H                              M2P_MESSAGE_ACCESS(0x33)
#define M2P_CTMU_CH0_CFG2L                              M2P_MESSAGE_ACCESS(0x34)
#define M2P_CTMU_CH0_CFG2H                              M2P_MESSAGE_ACCESS(0x35)

#define M2P_CTMU_CH1_CFG0L                              M2P_MESSAGE_ACCESS(0x38)
#define M2P_CTMU_CH1_CFG0H                              M2P_MESSAGE_ACCESS(0x39)
#define M2P_CTMU_CH1_CFG1L                              M2P_MESSAGE_ACCESS(0x3a)
#define M2P_CTMU_CH1_CFG1H                              M2P_MESSAGE_ACCESS(0x3b)
#define M2P_CTMU_CH1_CFG2L                              M2P_MESSAGE_ACCESS(0x3c)
#define M2P_CTMU_CH1_CFG2H                              M2P_MESSAGE_ACCESS(0x3d)

#define M2P_CTMU_CH2_CFG0L                              M2P_MESSAGE_ACCESS(0x40)
#define M2P_CTMU_CH2_CFG0H                              M2P_MESSAGE_ACCESS(0x41)
#define M2P_CTMU_CH2_CFG1L                              M2P_MESSAGE_ACCESS(0x42)
#define M2P_CTMU_CH2_CFG1H                              M2P_MESSAGE_ACCESS(0x43)
#define M2P_CTMU_CH2_CFG2L                              M2P_MESSAGE_ACCESS(0x44)
#define M2P_CTMU_CH2_CFG2H                              M2P_MESSAGE_ACCESS(0x45)

#define M2P_CTMU_CH3_CFG0L                              M2P_MESSAGE_ACCESS(0x48)
#define M2P_CTMU_CH3_CFG0H                              M2P_MESSAGE_ACCESS(0x49)
#define M2P_CTMU_CH3_CFG1L                              M2P_MESSAGE_ACCESS(0x4a)
#define M2P_CTMU_CH3_CFG1H                              M2P_MESSAGE_ACCESS(0x4b)
#define M2P_CTMU_CH3_CFG2L                              M2P_MESSAGE_ACCESS(0x4c)
#define M2P_CTMU_CH3_CFG2H                              M2P_MESSAGE_ACCESS(0x4d)

#define M2P_CTMU_CH4_CFG0L                              M2P_MESSAGE_ACCESS(0x50)
#define M2P_CTMU_CH4_CFG0H                              M2P_MESSAGE_ACCESS(0x51)
#define M2P_CTMU_CH4_CFG1L                              M2P_MESSAGE_ACCESS(0x52)
#define M2P_CTMU_CH4_CFG1H                              M2P_MESSAGE_ACCESS(0x53)
#define M2P_CTMU_CH4_CFG2L                              M2P_MESSAGE_ACCESS(0x54)
#define M2P_CTMU_CH4_CFG2H                              M2P_MESSAGE_ACCESS(0x55)
#define M2P_RVD2PVDD_EN                              	M2P_MESSAGE_ACCESS(0x56)
#define M2P_PVDD_EXTERN_DCDC                           	M2P_MESSAGE_ACCESS(0x57)

#define M2P_USER_PEND                                   (0x58)
#define M2P_USER_MSG_TYPE                               (0x59)
#define M2P_USER_MSG_LEN0                               (0x5a)
#define M2P_USER_MSG_LEN1                               (0x5b)
#define M2P_USER_ADDR0                                  (0x5c)
#define M2P_USER_ADDR1                                  (0x5d)
#define M2P_USER_ADDR2                                  (0x5e)
#define M2P_USER_ADDR3                                  (0x5f)


/*
 *  Must Sync to P11 code
 */
enum {
    M2P_LP_INDEX    = 0,
    M2P_PF_INDEX,
    M2P_LLP_INDEX,
    M2P_P33_INDEX,
    M2P_SF_INDEX,
    M2P_CTMU_INDEX,
    M2P_CCMD_INDEX,       //common cmd
    M2P_VAD_INDEX,
    M2P_USER_INDEX,
    M2P_WDT_INDEX,
};

enum {
    P2M_LP_INDEX    = 0,
    P2M_PF_INDEX,
    P2M_LLP_INDEX,
    P2M_WK_INDEX,
    P2M_WDT_INDEX,
    P2M_LP_INDEX2,
    P2M_CTMU_INDEX,
    P2M_CTMU_POWUP,
    P2M_REPLY_CCMD_INDEX,  //reply common cmd
    P2M_VAD_INDEX,
    P2M_USER_INDEX,
    P2M_BANK_INDEX,
};

enum {
    CLOSE_P33_INTERRUPT = 1,
    OPEN_P33_INTERRUPT,
    LOWPOWER_PREPARE,
};

void wdt_isr(void);
u8 p11_run_query(void);

#endif /* #ifndef __P11_APP_H__ */


