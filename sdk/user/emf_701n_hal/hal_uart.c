/* 
*   BSD 2-Clause License
*   Copyright (c) 2022, LiteEMF
*   All rights reserved.
*   This software component is licensed by LiteEMF under BSD 2-Clause license,
*   the "License"; You may not use this file except in compliance with the
*   License. You may obtain a copy of the License at:
*       opensource.org/licenses/BSD-2-Clause
* 
*/

/************************************************************************************************************
**	Description:	
************************************************************************************************************/
#include "hw_config.h"
#include "hw_board.h"
#ifdef HW_UART_MAP

#include  "api/api_system.h"
#include  "api/api_uart.h"
#include  "api/api_gpio.h"
#include  "asm/clock.h"

#include  "api/api_log.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/
	// #define HW_UART_MAP {\
	// 	{PA_04, PIN_NULL,	0, 0, 0, VAL2FLD(UART_BAUD,1000000)},	\
	// 	{PA_09, PIN_NULL, 	0, 64, 0, VAL2FLD(UART_BAUD,1000000)|VAL2FLD(UART_PRI,1)},		\
	// 	}
/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/

static uart_bus_t *uart_bus[4]={NULL,NULL};         //串口模块指针
static uint8_t rx_buf[4][UART_CBUF_SIZE];			//初始化参数的buf 还没测试这个buf内容 应该是串口寄存器DMA接收用的buf


/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/

/*****************************************************************************************************
**	static Function
******************************************************************************************************/

//中断回调
static void uart_callback(void *arg, u32 status)
{
    const uart_bus_t *ubus = (uart_bus_t*)arg;

    uint16_t len;
	uint8_t id = (uint8_t)ubus->argv;

    if(status == UT_RX_OT || UT_RX == status){          //添加长度溢出中断接收
        uint8_t temp_buf[UART_CBUF_SIZE];						//读串口接收的buf
        // 使用callback 回调中调用read 不会延时,因为 callback 调用时候一定是有数据的
        len = ubus->read(temp_buf,UART_CBUF_SIZE,0);//读接收buf  timeout 10倍数,中断中读取不超时间
		api_uart_rx_hook(id, temp_buf, len);
    }
}
/*****************************************************************************************************
**  Function
******************************************************************************************************/

/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
bool hal_uart_set_baud(uint8_t id, uint32_t baud)
{
	if(NULL != uart_bus[id]){
        uart_bus[id]->set_baud(baud);
    }
	return true;
}
bool hal_uart_tx(uint8_t id,void * buf, uint16_t len)
{
	if(NULL != uart_bus[id]){
		uart_bus[id]->write(buf,len);
	}
	return true;
}

bool hal_uart_init(uint8_t id,uint32_t baudrate)
{
	struct uart_platform_data_t uart_para = {0};

    uart_para.tx_pin = m_uart_map[id].tx;
    uart_para.rx_pin = m_uart_map[id].rx;
    uart_para.argv = (void*)((uint32_t)id);
    uart_para.baud = baudrate;
    uart_para.rx_timeout = 4;	//40us (460800 一个字节22us)
    uart_para.isr_cbfun = uart_callback;
    uart_para.rx_cbuf = rx_buf[id];
    uart_para.rx_cbuf_size = UART_CBUF_SIZE;            //uart fifo 长度
    uart_para.frame_length = UART_CBUF_SIZE;            //串口收到数据长度到frame_length就会起一次中断
    uart_bus[id] = (uart_bus_t *)uart_dev_open(&uart_para);
    if(uart_bus[id] == NULL){
        logd("UART init ERROR!\n");
		return false;
    }

	return true;
}
bool hal_uart_deinit(uint8_t id)
{
	if(uart_bus[id]){
		uart_dev_close(uart_bus[id]);
		api_gpio_dir(m_uart_map[id].tx,PIN_IN, PIN_PULLNONE);
		api_gpio_dir(m_uart_map[id].rx, PIN_IN,PIN_PULLNONE);
		uart_bus[id] = NULL;
	}
	return false;
}


#endif





