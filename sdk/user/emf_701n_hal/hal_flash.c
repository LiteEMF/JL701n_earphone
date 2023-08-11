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
#include  "api/api_flash.h"

#if API_STORAGE_ENABLE

#include "system/includes.h"
/******************************************************************************************************
** Defined
*******************************************************************************************************/
#define USER_FILE_NAME       SDFILE_APP_ROOT_PATH   "USERIF"
#define FLASH_SECTION_SIZE  0X1000


typedef enum _FLASH_ERASER {
    CHIP_ERASER,
    BLOCK_ERASER,
    SECTOR_ERASER,
} FLASH_ERASER;

extern bool sfc_erase(FLASH_ERASER cmd, u32 addr);
extern u32 sdfile_cpu_addr2flash_addr(u32 offset);
/*****************************************************************************************************
**  Function
******************************************************************************************************/

/*
    擦除4k, 60ms
*/
bool hal_flash_erase(uint16_t offset)
{
    bool ret;
    struct vfs_attr attr;
    u32 flash_addr;
    FILE *code_fp = NULL;
    code_fp = fopen(USER_FILE_NAME, "r+w");
    if (code_fp == NULL) {
        logd("USERIF open err!!!");
        return false;
    }

    fget_attrs(code_fp, &attr);
    logd("USERIF add=%x len=%d\n",attr.sclust , attr.fsize);

    flash_addr = sdfile_cpu_addr2flash_addr(attr.sclust);
    flash_addr += offset;

    logd("serase offset=%x, add=%x\n",offset,flash_addr);
    ret = sfc_erase(SECTOR_ERASER, flash_addr);
    
    fclose(code_fp);
	return ret;
}

/*******************************************************************
** Parameters:		
** Returns:	
** Description: 写4K 38ms	
    写入前会对所有可写入进行判断,如果有(bit 0 写  1)不会写入
	如果协议相同数据,可以写入成功, 会占用写flash 时间
*******************************************************************/
//hal
bool hal_flash_write(uint16_t offset,uint8_t *buf,uint16_t len)
{
    bool ret = true;
    FILE *code_fp = NULL;
    code_fp = fopen(USER_FILE_NAME, "r+w");
    if (code_fp == NULL) {
        logd("USERIF open err!!!");
        return false;
    }

    if(fseek(code_fp, offset, 0)){
        logd("USERIF fseek err!!!");
        return false;
    }

    int r = fwrite(code_fp, buf, len);
    if(r != len){
        logd("write file error code %x", r);
        ret = false;
    }
    fclose(code_fp);
	return ret;
}
bool hal_flash_read(uint16_t offset,uint8_t *buf,uint16_t len)
{
    bool ret = true; 

    FILE *code_fp = NULL;
    code_fp = fopen(USER_FILE_NAME, "r+w");
    if (code_fp == NULL) {
        logd("USERIF open err!!!");
        return false;
    }

    if(fseek(code_fp, offset, 0)){
        logd("USERIF fseek err!!!");
        return false;
    }

    int r = fread(code_fp, buf, len);
    if(r != len){
        logd("read file error code %x", r);
        ret = false;
    }
    fclose(code_fp);

	return ret;
}

bool hal_flash_init(void)
{
	return true;
}

#endif









