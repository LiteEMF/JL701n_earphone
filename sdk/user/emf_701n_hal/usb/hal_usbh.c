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
#if API_USBH_BIT_ENABLE
#include  "api/usb/usb_typedef.h"
#include  "api/usb/host/usbh.h"

#include "asm/usb.h"
#include "usb/ch9.h"
#include "usb/usb_phy.h"
#include "usb_ctrl_transfer.h"
#include "device_drive.h"
#include "usb_std_class_def.h"

#include "system/includes.h"

#include "api/api_log.h"

/******************************************************************************************************
** Defined
*******************************************************************************************************/

/******************************************************************************************************
**	public Parameters
*******************************************************************************************************/

/******************************************************************************************************
**	static Parameters
*******************************************************************************************************/
struct usb_ep_addr_t host_ep_addr[USBH_NUM];
mem_buf_t usbh_mem[USBH_NUM];
static uint8_t ep0_dma[USBH_NUM][64 + 4]  __attribute__((aligned(4)));
static uint8_t ep_dma_buf[USBH_NUM][(USBH_ENDP_NUM-1) * 2 * (64 + 4)];
static uint8_t* m_ep_pbuffer[USBH_NUM][USBH_ENDP_NUM][2];
static uint8_t m_target_ep[USBH_NUM][USBH_ENDP_NUM][2];			//数组下标是host_ep

/*****************************************************************************************************
**	static Function
******************************************************************************************************/

#if USB_HOST_ASYNC
OS_SEM *usb_host_sem[USBH_NUM];

static int usbh_sem_init(usb_dev usb_id)
{
    OS_SEM *sem = zalloc(sizeof(OS_SEM));
    ASSERT(sem, "usb alloc sem error");
    usb_host_sem[usb_id] = sem;
    logd_g("%s %x %x ", __func__, usb_id, sem);
    os_sem_create(usb_host_sem[usb_id], 0);
    return 0;
}

static int usbh_sem_pend(usb_dev usb_id, u32 timeout)
{
    if (usb_host_sem[usb_id] == NULL) {
        return 1;
    }
    int ret = os_sem_pend(usb_host_sem[usb_id], timeout);
    if (ret) {
        logd_r("%s %d ", __func__, ret);
    }
    return ret;
}

static int usbh_sem_post(usb_dev usb_id)
{
    if (usb_host_sem[usb_id] == NULL) {
        return 1;
    }
    int ret = os_sem_post(usb_host_sem[usb_id]);
    if (ret) {
        logd_r("%s %d ", __func__, ret);
    }
    return 0;
}

static int usbh_sem_del(usb_dev usb_id)
{
    if (usb_host_sem[usb_id] == NULL) {
        return 0;
    }
#if USB_HUB
    if (host_dev && host_ep->sem && host_dev->father == NULL) {
        os_sem_del(usb_host_sem[usb_id]);
    }
#else
    if (usb_host_sem[usb_id]) {
        os_sem_del(usb_host_sem[usb_id], 0);
    }
#endif
    logd_r("%s %x %x ", __func__, usb_id, usb_host_sem[usb_id]);
    free(usb_host_sem[usb_id]);
    usb_host_sem[usb_id] = NULL;
    return 0;
}
#endif

void usb_h_isr(const usb_dev usb_id)
{
    u32 intr_usb, intr_usbe;
    u32 intr_tx, intr_txe;
    u32 intr_rx, intr_rxe;

    __asm__ volatile("ssync");

    usb_read_intr(usb_id, &intr_usb, &intr_tx, &intr_rx);
    usb_read_intre(usb_id, &intr_usbe, &intr_txe, &intr_rxe);

    intr_usb &= intr_usbe;
    intr_tx &= intr_txe;
    intr_rx &= intr_rxe;

    if (intr_usb & INTRUSB_SUSPEND) {
        loge("usb suspend");
    }
    if (intr_usb & INTRUSB_RESET_BABBLE) {
        loge("usb reset");
    }
    if (intr_usb & INTRUSB_RESUME) {
        loge("usb resume");
    }

    if (intr_tx & BIT(0)) {
		// usbh_endp_in_event(usb_id<<4,0);
		#if USB_HOST_ASYNC
		usbh_sem_post(usb_id);
		#endif
    }

    for (int i = 1; i < USBH_ENDP_NUM; i++) {
        if (intr_tx & BIT(i)) {
			usbh_endp_out_event(usb_id<<4,m_target_ep[usb_id][i][0]);
        }
    }

    for (int i = 1; i < USBH_ENDP_NUM; i++) {
        if (intr_rx & BIT(i)) {
			usbh_endp_in_event(usb_id<<4, m_target_ep[usb_id][i][1] | TUSB_DIR_IN_MASK);
        }
    }
    __asm__ volatile("csync");
}
SET_INTERRUPT
void usb0_h_isr()
{
    usb_h_isr(0);
}
SET_INTERRUPT
void usb1_h_isr()
{
    usb_h_isr(1);
}


void usb_h_isr_reg(const usb_dev usb_id, u8 priority, u8 cpu_id)
{
    if (usb_id == 0) {
        request_irq(IRQ_USB_CTRL_IDX, priority, usb0_h_isr, cpu_id);
#if USBH_NUM > 1
    } else if (usb_id == 1) {
        request_irq(IRQ_USB1_CTRL_IDX, priority, usb1_h_isr, cpu_id);
#endif
    }
}

__attribute__((always_inline_when_const_args))
uint8_t *hal_usbh_get_endp_buffer(uint8_t id, uint8_t ep)
{
    uint8_t *ep_buffer = NULL;
    uint8_t ep_addr = ep & 0x0f;
    usb_dev usb_id = id>>4;

    if(0 == ep_addr){
        ep_buffer = ep0_dma[usb_id];
    }else{
        if(TUSB_DIR_IN_MASK & ep){
            ep_buffer = m_ep_pbuffer[usb_id][ep_addr][1];
        }else{
            ep_buffer = m_ep_pbuffer[usb_id][ep_addr][0];
        }
    }
    return ep_buffer;
}

void usb_host_config(usb_dev usb_id)
{
    usb_var_init(usb_id, &host_ep_addr[usb_id]);
}


uint8_t hal_usbh_find_host_ep(uint8_t id, uint8_t ep)
{
    uint8_t i, host_ep = 0; 
    uint8_t ep_addr = ep & 0X7F;
    uint8_t usb_id = id >> 4;
    
    for(i=1; i<USBH_ENDP_NUM; i++){
        if(TUSB_DIR_IN_MASK & ep){
            if(m_target_ep[usb_id][i][1] == ep_addr){
                host_ep = i;
                break;
            }
        }else{
            if(m_target_ep[usb_id][i][0] == ep_addr){
                host_ep = i;
                break;
            }
        }
    }
    return host_ep;
}




static int usb_ctlXfer(uint8_t id, uint8_t mtu, struct ctlXfer *urb)
{
    u32 ret = DEV_ERR_NONE;
    u8 reg = 0;
    u32 data_len;
    usb_dev usb_id = id>>4;

    #ifdef USB_HW_20
    usb_write_txfuncaddr(usb_id, 0, devnum);
    #endif

    switch (urb->stage) {
    case USB_PID_SETUP :
        usb_write_ep0(usb_id, (u8 *)&urb->setup, 8);
        reg = CSR0H_SetupPkt | CSR0H_TxPktRdy;
        break;
    case USB_PID_IN :
        if (urb->setup.wLength) {
            reg = CSR0H_ReqPkt;
        } else {
            reg = CSR0H_StatusPkt | CSR0H_ReqPkt;
        }
        break;
    case USB_PID_OUT:
        if (urb->setup.wLength) {
            data_len = MIN(urb->setup.wLength, mtu);
            reg = CSR0H_TxPktRdy;
            usb_write_ep0(usb_id, urb->buffer, data_len);
            urb->setup.wLength -= data_len;
            urb->buffer += data_len;
        } else {
            reg = CSR0H_StatusPkt | CSR0H_TxPktRdy;
        }
        break;
    default :
        break;
    }

#if USB_HOST_ASYNC
    //config ep0 callback fun
    usb_set_intr_txe(usb_id, 0);
#endif

    #ifdef USB_HW_20
    usb_write_csr0(usb_id, reg | CSR0H_DISPING);
    #else
    usb_write_csr0(usb_id, reg);
    #endif

    u32 st = 0;
    u32 ot = usb_get_jiffies() + 500;
    while (1) {
        if (usb_host_timeout(ot)) {
            loge("time out %x\n", reg);
            ret = -DEV_ERR_TIMEOUT;
            goto __exit;
        }
        if (usb_h_dev_status(usb_id)) {
            st ++;
        } else {
            st = 0;
        }
        if (((usb_read_devctl(usb_id) & BIT(2)) == 0) || (st > 1000)) {
            loge("usb%d_offline\n", usb_id);
            ret = -DEV_ERR_OFFLINE;
            goto __exit;
        }

#if USB_HOST_ASYNC
        ret = usbh_sem_pend(usb_id, 250); //wait isr
        if (ret) {
            loge("usb%d_offline\n", usb_id);
            ret = -DEV_ERR_OFFLINE;
            goto __exit;
        }
#endif

        reg = usb_read_csr0(usb_id);
        if (reg & CSR0H_RxStall) {
            loge(" rxStall CSR0:0x%x\n", reg);
            ret = -DEV_ERR_CONTROL_STALL;
            goto __exit;
        }
        if (reg & CSR0H_Error) {
            loge(" Error CSR0:0x%x\n", reg);
            usb_write_csr0(usb_id, 0);
            ret = -DEV_ERR_CONTROL;
            goto __exit;
        }
        if (USB_PID_IN == urb->stage) {

            if (reg & CSR0H_RxPktRdy) {
                data_len = usb_read_count0(usb_id);
                data_len = min(data_len, urb->setup.wLength);
                usb_read_ep0(usb_id, urb->buffer, data_len);;
                urb->buffer += data_len;
                urb->setup.wLength -= data_len;
                if (data_len < mtu) {
                    urb->setup.wLength = 0;
                }
                if (urb->setup.wLength) {
#ifdef USB_HW_20
                    usb_write_csr0(usb_id, CSR0H_ReqPkt | CSR0H_DISPING);
#else
                    usb_write_csr0(usb_id, CSR0H_ReqPkt);
#endif
                } else {
#ifdef USB_HW_20
                    usb_write_csr0(usb_id, CSR0H_DISPING);
#else
                    usb_write_csr0(usb_id, 0);
#endif
                    break;
                }
            }
        } else {
            if (!(reg & CSR0H_TxPktRdy)) {
                break;
            }
        }
    }
__exit:
    usb_clr_intr_txe(usb_id, 0);
    usb_dis_ep0_txdly(usb_id);
    return ret;
}


static int usb_control_transfers(uint8_t id, uint8_t mtu, struct ctlXfer *urb)
{
    int res;
    /*SETUP*/

    urb->stage = USB_PID_SETUP;		//SETUP transaction

    res = usb_ctlXfer(id, mtu, urb);

    if (res) {
        return res;
    }

    /*IN or OUT*/
    urb->stage = USB_PID_IN;

    while (urb->setup.wLength) {
        if (urb->setup.bRequestType & USB_DIR_IN) {	//Request Direction
            urb->stage = USB_PID_IN;	//IN transaction

            res = usb_ctlXfer(id, mtu, urb);

            if (res) {
                return res;
            }

            urb->stage = USB_PID_OUT;
        } else {
            urb->stage = USB_PID_OUT;	//OUT transaction

            res = usb_ctlXfer(id, mtu, urb);

            if (res) {
                return res;
            }

            urb->stage = USB_PID_IN;
        }
    }

    res = usb_ctlXfer(id, mtu, urb);

    if (res) {
        return res;
    }

    return DEV_ERR_NONE;
}



/*****************************************************************************************************
**  Function
******************************************************************************************************/


/*******************************************************************
** Parameters:		
** Returns:	
** Description:		
*******************************************************************/
error_t hal_usbh_port_en(uint8_t id,uint8_t en, usb_speed_t* pspeed)
{
	u32 ret = 1;
    usb_dev usb_id = id>>4;

	if(en){
		void *const dma = hal_usbh_get_endp_buffer(id, 0);
		usb_set_dma_taddr(usb_id, 0, dma);

        usb_sie_enable(usb_id);//enable sie intr
        // usb_mdelay(20);
        usb_otg_resume(usb_id);  //打开usb host之后恢复otg检测, 这样不会反复检测接入和拔出usb
	}else{
		usb_sie_close(usb_id);
	}
	return ERROR_SUCCESS;
}
error_t hal_usbh_set_speed(uint8_t id, usb_speed_t speed)
{
    if(TUSB_SPEED_LOW == speed){    
        usb_set_low_speed(id>>4, 1);
    }else{
        usb_set_low_speed(id>>4, 0);    //set default speed, 如果不设置切换low/fullspeed设备时候full speed设备会出错
    }
	return ERROR_SUCCESS;
}

error_t hal_usbh_port_reset(uint8_t id, uint8_t reset_ms)
{
	error_t ret = ERROR_FAILE;
    usb_dev usb_id = id>>4;

	usb_h_sie_init(usb_id);
    ret = usb_host_init(usb_id, reset_ms, 250);

	return ret;
}
error_t hal_usbh_set_addr(uint8_t id,uint8_t addr)
{
    usb_dev usb_id = id>>4;

	usb_write_faddr(usb_id, addr);
	return ERROR_SUCCESS;
}

//调用后中断是没有关闭的,有小缺陷
error_t hal_usbh_endp_unregister(uint8_t id, usb_endp_t *endpp)
{
	uint8_t endp_dir = endpp->dir? TUSB_DIR_IN_MASK:0;
	uint8_t host_ep;
    usb_dev usb_id = id>>4;

	mem_buf_init(&usbh_mem[usb_id], ep_dma_buf[usb_id], sizeof(ep_dma_buf[usb_id]), 4);
	host_ep = hal_usbh_find_host_ep(id, endpp->addr | endp_dir);

    if(host_ep){
        if(endpp->dir){
            usb_clr_intr_txe(usb_id,host_ep);		//这个函数能关闭中断
        }else{
            usb_clr_intr_rxe(usb_id,host_ep);
        }
        // usb_free_ep(usb_id,host_ep,endp_dir);
    }

	return ERROR_SUCCESS;
}

error_t hal_usbh_endp_register(uint8_t id, usb_endp_t *endpp)
{
	uint8_t endp_dir;
	uint8_t *ep_buffer;
	uint8_t isr_en = false;
    usb_dev usb_id = id>>4;

	endp_dir = endpp->dir? TUSB_DIR_IN_MASK:0;
	if(endp_dir || (endpp->type == TUSB_ENDP_TYPE_ISOCH)){	//endp in and iso endp used isr
		isr_en = true;
	}

	u32 host_ep = usb_get_ep_num(usb_id, endp_dir, endpp->type);
    if((u32)-1 == host_ep){
        logd("usbh ep null!\n");
        return ERROR_FAILE;
    }
    
	ep_buffer = mem_buf_alloc(&usbh_mem[usb_id], endpp->mtu + 4);
    if(NULL == ep_buffer){
        logd("usbh mem null!\n");
        return ERROR_NO_MEM;
    }

	if(endp_dir){
		m_target_ep[usb_id][host_ep][1] = endpp->addr;
		m_ep_pbuffer[usb_id][host_ep][1] = ep_buffer;
	}else{
		m_target_ep[usb_id][host_ep][0] = endpp->addr;
		m_ep_pbuffer[usb_id][host_ep][0] = ep_buffer;
	}
	usb_h_ep_config(usb_id, host_ep | endp_dir, endpp->type,isr_en,endpp->interval, ep_buffer, endpp->mtu);
	
	if(isr_en){
		usb_h_ep_read_async(usb_id, host_ep, endpp->addr, NULL, 0, endpp->type, 1);
	}
	logd("usbh_endp_register ep=%x %x\n",host_ep,endpp->addr | endp_dir);

	return ERROR_SUCCESS;
}


error_t hal_usbh_ctrl_transfer( uint8_t id, usb_control_request_t* preq,uint8_t* buf, uint16_t* plen)
{
	int8_t ret;
    usbh_dev_t* pdev = get_usbh_dev(id);

	struct ctlXfer urb;
	memcpy(&urb.setup, preq, 8);
    urb.buffer = buf;
    ret = usb_control_transfers(id, pdev->endp0_mtu, &urb);

	if(NULL != plen) *plen = (uint16_t)((uint32_t)(urb.buffer) - (uint32_t)buf);

	if((-DEV_ERR_CONTROL_STALL == ret) || (-DEV_ERR_RXSTALL == ret) || (-DEV_ERR_TXSTALL == ret)){
		ret = ERROR_STALL;
	}else if(DEV_ERR_NONE != ret){
		logd("usbh crtl err=%d\n",ret);
		ret = ERROR_UNKNOW;
	}

	return ret;
}



error_t hal_usbh_in(uint8_t id, usb_endp_t *endpp, uint8_t* buf, uint16_t* plen, uint16_t timeout_ms)
{
	error_t err = ERROR_NACK;
    usb_dev usb_id = id>>4;
	int32_t rx_len;
	usbh_dev_t* pdev = get_usbh_dev(id);
	uint8_t endp_dir = endpp->dir? TUSB_DIR_IN_MASK:0;
	uint8_t host_ep = hal_usbh_find_host_ep(id, endpp->addr | endp_dir);


	#if USBH_LOOP_ENABLE
	*plen = usb_h_ep_read(usb_id, host_ep, endpp->mtu, endpp->addr, buf, *plen, endpp->type);
	#else
	*plen = usb_h_ep_read_async(usb_id, host_ep, endpp->addr, buf, endpp->mtu, endpp->type, 0);
    usb_h_ep_read_async(usb_id, host_ep, endpp->addr, NULL, 0, endpp->type, 1);
	#endif

	if(*plen) {
		err = ERROR_SUCCESS;
	}

	return err;
}

error_t hal_usbh_out(uint8_t id, usb_endp_t *endpp,uint8_t* buf, uint16_t len)
{
	int tx_len;
    usb_dev usb_id = id>>4;
	usbh_dev_t* pdev = get_usbh_dev(id);
	uint8_t endp_dir = endpp->dir? TUSB_DIR_IN_MASK:0;
	uint8_t host_ep = hal_usbh_find_host_ep(id, endpp->addr | endp_dir);

	tx_len = usb_h_ep_write(usb_id, host_ep, endpp->mtu, endpp->addr, buf, len, endpp->type);
	//tx_len = usb_h_ep_write_async(usb_id, host_ep, endpp->mtu, endpp->addr, buf, len, endpp->type, !len);
	
	if(tx_len == len) {
		return ERROR_SUCCESS;
	}else{
		return ERROR_UNKNOW;
	}
}

error_t hal_usbh_driver_init(uint8_t id)
{
    usb_dev usb_id = id>>4;

	memset(&host_ep_addr[usb_id], 0, sizeof(host_ep_addr[usb_id]));
    memset(m_ep_pbuffer[usb_id], 0, sizeof(m_ep_pbuffer[usb_id]));
	memset(m_target_ep[usb_id], 0, sizeof(m_target_ep[usb_id]));
    m_ep_pbuffer[usb_id][0][0] = ep0_dma[usb_id];
    m_ep_pbuffer[usb_id][0][1] = ep0_dma[usb_id];
    mem_buf_init(&usbh_mem[usb_id], ep_dma_buf[usb_id], sizeof(ep_dma_buf[usb_id]), 4);

    // usb_otg_resume(usb_id);  //打开usb host之后恢复otg检测
    #if USB_HOST_ASYNC
    usbh_sem_init(usb_id);
    #endif
    usb_host_config(usb_id);
    usb_h_isr_reg(usb_id, 1, 0);
    
	return ERROR_SUCCESS;
}
error_t hal_usbh_driver_deinit(uint8_t id)
{
    usb_dev usb_id = id>>4;
	#if USB_HOST_ASYNC
    usbh_sem_post(usb_id);		//拔掉设备时，让读写线程快速释放
	#endif
    usb_sie_disable(usb_id);

	#if USB_HOST_ASYNC
	usbh_sem_del(usb_id);
	#endif

    usb_otg_resume(usb_id);  //打开usb host之后恢复otg检测
	return ERROR_SUCCESS;
}
void hal_usbh_driver_task(uint32_t dt_ms)
{

}

#endif


