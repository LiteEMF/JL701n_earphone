/**@file  		power_api.c
* @brief        电源应用流程
* @details		HW API
* @author		JL
* @date     	2021-11-30
* @version  	V1.0
* @copyright    Copyright:(c)JIELI  2011-2020  @ , All Rights Reserved.
 */
#include "asm/power_interface.h"
#include "app_config.h"
#include "includes.h"
#include "asm/clock_hw.h"

void pmut_put_u8hex(u8 dat);
void pmu_put_u16hex(u16 dat);
void pmu_put_u32hex(u32 dat);
void pmu_put_u64hex(u64 dat);
void pmu_put_buf(const u8 *buf, int len);

AT_VOLATILE_RAM_CODE
u8 sleep_safety_check(void)
{
#if 0
    u32 get_rch_freq();
    JL_PORTA->DIR &= ~BIT(5);
    JL_OMAP->PA5_OUT = FO_UART0_TX;
    MAIN_CLOCK_SEL(MAIN_CLOCK_IN_RC);
    HSB_CLK_DIV(0);
    LSB_CLK_DIV(0);
    UART_CLOCK_IN(UART_CLOCK_IN_LSB);

    JL_UART0->BAUD = (get_rch_freq() * 1000 / 115200) / 4 - 1;

    void check_gpio_safety();
    check_gpio_safety();

    void wdt_close();
    wdt_close();
    while (1);
#endif
    return 0;
}

AT_VOLATILE_RAM_CODE
u8 soff_safety_check(void)
{
#if 0
    u32 get_rch_freq();
    JL_PORTA->DIR &= ~BIT(5);
    JL_OMAP->PA5_OUT = FO_UART0_TX;
    MAIN_CLOCK_SEL(MAIN_CLOCK_IN_RC);
    HSB_CLK_DIV(0);
    LSB_CLK_DIV(0);
    UART_CLOCK_IN(UART_CLOCK_IN_LSB);

    JL_UART0->BAUD = (get_rch_freq() * 1000 / 115200) / 4 - 1;

    void check_gpio_safety();
    check_gpio_safety();
#endif
    return 0;
}

#if (TCFG_LOWPOWER_LOWPOWER_SEL == SLEEP_EN)


static enum LOW_POWER_LEVEL power_app_level(void)
{
    return LOW_POWER_MODE_SLEEP;
}

static u8 power_app_idle(void)
{
    return 1;
}

REGISTER_LP_TARGET(power_app_lp_target) = {
    .name = "power_app",
    .level = power_app_level,
    .is_idle = power_app_idle,
};

#endif




#if 0

extern volatile u8 extern_flash_stable;
volatile static u8 extern_flash_post = 0;
volatile static u8 extern_flash_post1 = 0;

u8 extern_flash_handler(u32 timeout)
{
    //no need handler
    if (extern_flash_stable == 0) {
        extern_flash_post = 0;
        return 0;
    }

    //handlering
    if (extern_flash_post == 1) {
        return 1;
    }

    //request handler
    extern_flash_post = 1;

    os_taskq_post_msg("pmu_task", 1, 1);

    return 1;
}

u8 extern_flash_handler1(u32 timeout)
{
    //handlering
    if (extern_flash_stable == 1) {
        return 0;
    }

    extern_flash_stable = 1;

    os_taskq_post_msg("pmu_task", 1, 2);

    return 0;
}

//低功耗线程请求所有模块关闭，由对应线程处理
REGISTER_LP_REQUEST(power_app_lp_target) = {
    .name = "extern_flash",
    .request_enter = extern_flash_handler,
    .request_exit = extern_flash_handler1,
};

#endif









