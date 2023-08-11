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
#ifdef HW_SPI_HOST_MAP

#include  "api/api_spi_host.h"
#include  "api/api_gpio.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/
// #define HW_SPI_HOST_MAP {\
// 			{PB_07,PB_05,PB_06,PB_04,SPI1,VAL2FLD(SPI_BADU,1000)},	\
// 			}

/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/
struct spi_platform_data spi0_p_data;
struct spi_platform_data spi1_p_data;
struct spi_platform_data spi2_p_data;



/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/
//bd19: SPI1:  CLK  ,  DO       ,  DI
//SPI0: NONE
//SPI1: {IO_PORTA_07,IO_PORTA_08,IO_PORTA_06}
//SPI2: {IO_PORTB_06,IO_PORTB_07,IO_PORTB_05}
//BR23
// ref spi_io_map[SPI_MAX_HW_NUM] 
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
bool hal_spi_host_write(uint8_t id,uint16_t addr, uint8_t * buf, uint16_t len)
{
	bool ret;
	uint16_t i;
	int8_t spi = m_spi_map[id].peripheral;

	if(addr >> 8){
		ret = !spi_send_byte(spi,(addr >> 8));
		ret &= !spi_send_byte(spi,addr);
	}else{
		ret = !spi_send_byte(spi,addr);
	}

	if(ret){
		ret = (len == spi_dma_send(spi, buf, len));
	}
	
	return true;
}
bool hal_spi_host_read(uint8_t id,uint16_t addr, uint8_t * buf, uint16_t len)
{
	bool ret;
	uint16_t i;
	int8_t spi = m_spi_map[id].peripheral;
	
	if(addr >> 8){
		ret = !spi_send_byte(spi,(addr >> 8));
		ret &= !spi_send_byte(spi,addr);
	}else{
		ret = !spi_send_byte(spi,addr);
	}
	if(ret){
		// ret = (0 < spi_dma_recv(spi, buf, len));	//全双工模式下dma 没有清除上一次发送的数据
		for(i=0; i<len; i++){
			buf[i] = spi_recv_byte(spi, NULL);
		}
		ret = true;
	}
	return ret;
}
bool hal_spi_host_isr_write(uint8_t id,uint16_t addr, uint8_t * buf, uint16_t len)
{
	return hal_spi_host_write(id,addr,buf,len);
}
bool hal_spi_host_isr_read(uint8_t id,uint16_t addr, uint8_t * buf, uint16_t len)
{
	return hal_spi_host_read(id,addr,buf,len);
}
bool hal_spi_host_init(uint8_t id)
{
	uint32_t badu = SPI_BADU_ATT(id) * 1000;
	uint8_t spi = m_spi_map[id].peripheral;
	struct spi_platform_data *pspi;

	if(spi == SPI0){
		#if defined CONFIG_CPU_BD19 || defined CONFIG_CPU_BR28
		logd("bd19 spi0 not support\n");
		return false;
		#endif
		pspi = &spi0_p_data;
	}else if(spi == SPI1){
		pspi = &spi1_p_data;
	}else if(spi == SPI2){
		pspi = &spi2_p_data;
	}else{
		return false;
	}

	#if defined CONFIG_CPU_BD19 || defined CONFIG_CPU_BR28
	pspi->port[0] = m_spi_map[id].clk;
	pspi->port[1] = m_spi_map[id].mosi;
	pspi->port[2] = m_spi_map[id].miso;
	pspi->port[3] = PIN_NULL;
	pspi->port[4] = PIN_NULL;
	#elif defined CONFIG_CPU_BR23
	extern const struct spi_io_mapping spi_io_map[SPI_MAX_HW_NUM];
	for(uint8_t i=0; i<spi_io_map[spi].num; i++){
		if(m_spi_map[id].clk == spi_io_map[spi].io->clk_pin){
			pspi->port = 'A' + i;
		}
	}	
	#endif
	pspi->mode = SPI_MODE_ATT(id);
	pspi->clk = badu;
	pspi->role = SPI_ROLE_MASTER;

	return !spi_open(m_spi_map[id].peripheral);
}
bool hal_spi_host_deinit(uint8_t id)
{
	spi_close(m_spi_map[id].peripheral);
	return true;
}
#endif




