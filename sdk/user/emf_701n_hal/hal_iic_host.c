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
#ifdef HW_IIC_MAP

#include  "api/api_iic_host.h"
#include  "api/api_gpio.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/
// #define HW_IIC_MAP {	\
// 			{PB_00,PB_01,PB_02,0,VAL2FLD(IIC_BADU,400000)},	\
// 			}

//BD19/BD28: IIC: CLK  ,  SDA       
  	// {IO_PORT_DP   IO_PORT_DM},		//'A'
  	// {IO_PORTA_09  IO_PORTA_10},		//'B'
  	// {IO_PORTA_07  IO_PORTA_08},		//'C'
  	// {IO_PORTA_05  IO_PORTA_06},		//'D'
//BR23 IIC: CLK  ,  SDA   
	// {IO_PORT_DP, IO_PORT_DM},    	// A
	// {IO_PORTC_04, IO_PORTC_05},  	// B
	// {IO_PORTB_04, IO_PORTB_06},  	// C
	// {IO_PORTA_05, IO_PORTA_06},  	// D

#define IIC_DEV_ID	 0

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/
//硬件IIC设备数据初始化
struct hw_iic_config hw_iic_cfg[1];


struct iic_iomapping {
	u8 scl;
	u8 sda;
};
/*****************************************************************************************************
**	static Function
******************************************************************************************************/

/*****************************************************************************************************
**  Function
******************************************************************************************************/

/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
bool hal_iic_write(uint8_t id,uint8_t dev_addr,uint16_t addr, uint8_t const *buf, uint16_t len)
{
	uint8_t ret, i;

	hw_iic_start(IIC_DEV_ID);
	ret = hw_iic_tx_byte(IIC_DEV_ID, dev_addr);
	if(!ret)	goto err;

	if(addr >> 8){
		ret = hw_iic_tx_byte(IIC_DEV_ID, addr >> 8);
		if(!ret)	goto err;

	}
	ret = hw_iic_tx_byte(IIC_DEV_ID, addr);
	if(!ret)	goto err;


	i = 0;
	while (i < len) {
		ret = hw_iic_tx_byte(IIC_DEV_ID, buf[i]);
		if(!ret)	goto err;
		i++;

	}

err:
	hw_iic_stop(IIC_DEV_ID);  
	delay_us(30); 	//stop后需要delay一段时间后再start,JL6321必要的延时,两次通信之间至少20us

	return ret;
}
bool hal_iic_read(uint8_t id,uint8_t dev_addr,uint16_t addr, uint8_t* buf, uint16_t len)
{
	uint8_t ret, i;

	hw_iic_start(IIC_DEV_ID);
	ret = hw_iic_tx_byte(IIC_DEV_ID, dev_addr);
	if(!ret)	goto err;

	if(addr >> 8){
		ret = hw_iic_tx_byte(IIC_DEV_ID, addr >> 8);
		if(!ret)	goto err;
	}

	ret = hw_iic_tx_byte(IIC_DEV_ID, addr);
	if(!ret)	goto err;

	hw_iic_start(IIC_DEV_ID);
	ret = hw_iic_tx_byte(IIC_DEV_ID, dev_addr | 0x01UL);
	if(!ret)	goto err;

	i = 0;
	while (i < len - 1) {
		buf[i] = hw_iic_rx_byte(IIC_DEV_ID, 1);
		i++;
	}
	buf[i] = hw_iic_rx_byte(IIC_DEV_ID, 0);  //IIC主机接收最后1 byte NACK

err:
	hw_iic_stop(IIC_DEV_ID);
	delay_us(30);  	//stop后需要delay一段时间后再start,必要的延时,两次通信之间至少20us

	return ret;
}
bool hal_iic_isr_write(uint8_t id,uint8_t dev_addr,uint16_t addr, uint8_t const *buf, uint16_t len)
{
	return hal_iic_write(id, dev_addr, addr, buf, len);
}
bool hal_iic_isr_read(uint8_t id,uint8_t dev_addr,uint16_t addr, uint8_t* buf, uint16_t len)
{
	return hal_iic_read(id, dev_addr, addr, buf, len);
}
bool hal_iic_init(uint8_t id)
{
	#if defined CONFIG_CPU_BD19  || defined CONFIG_CPU_BR28
	hw_iic_cfg[0].port[0] = m_iic_map[id].clk;
	hw_iic_cfg[0].port[1] = m_iic_map[id].sda;
	#elif defined CONFIG_CPU_BR23
	extern const struct iic_iomapping hwiic_iomap[IIC_HW_NUM][IIC_PORT_GROUP_NUM];
	for(uint8_t i=0; i<IIC_PORT_GROUP_NUM; i++){
		if((hwiic_iomap[0][i].scl == m_iic_map[id].clk) && (hwiic_iomap[0][i].sda == m_iic_map[id].sda)){
			hw_iic_cfg[0].port = 'A' + i;
		}
	}	
	#endif
	hw_iic_cfg[0].baudrate = IIC_BADU_ATT(id);
	hw_iic_cfg[0].hdrive = 0;		//是否打开IO口强驱 
	hw_iic_cfg[0].io_filter = 1;	//是否打开滤波器（去纹波） 
	hw_iic_cfg[0].io_pu = 1;		//是否打开上拉电阻，如果外部电路没有焊接上拉电阻需要置1 
	hw_iic_cfg[0].role = IIC_MASTER;

	return !hw_iic_init(IIC_DEV_ID);
}
bool hal_iic_deinit(uint8_t id)
{
	hw_iic_uninit(IIC_DEV_ID);
	return true;
}

#endif





