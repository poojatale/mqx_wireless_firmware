/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*   The main configuration file for WEB_HVAC demo
*
*
*END************************************************************************/

#ifndef __HVAC_H
#define __HVAC_H

#include <mqx.h>
#include <bsp.h>
#include <lwevent.h>
#include <message.h>
#include <rtcs.h>
#include <ftpc.h> 
#include <ftpsrv.h> 
#include "iwcfg.h"
#include "httpsrv.h"
#include <shell.h>

#if MQX_KERNEL_LOGGING
    #include <klog.h>
#endif

extern void HVAC_initialize_networking(void);

#define DEMOCFG_ENABLE_SERIAL_SHELL   1  // enable shell task for serial console 
#define DEMOCFG_ENABLE_DHCP           0  // enable DHCP for dynamic IP address 
#define DEMOCFG_ENABLE_DHCP_SERVER    1  // enable DHCP server 
 
#define ENET_IPADDR     IPADDR( 192, 168,   0, 1 ) 
#define ENET_IPMASK     IPADDR( 255, 255, 255, 0 ) 
#define ENET_IPGATEWAY  IPADDR( 192, 168,   0, 1 ) 

#if DEMOCFG_ENABLE_DHCP_SERVER
    #define DHCP_SERVER_IP          IPADDR( 192, 168, 0,   1 )
    #define DHCP_BROADCAST_IP       IPADDR( 192, 168, 0, 255 )
    #define DHCP_MIN_IP_ASSIGNED    IPADDR( 192, 168, 0,  10 )
    #define DHCP_NUM_IPs_ASSIGN     10
#endif
   
#define DEMOCFG_SSID            "FRDM-K22F Wireless"

// Possible Values managed, softap, or adhoc
#define DEMOCFG_NW_MODE         "softap" 

// Possible vales 
//   1. "wep"
//   2. "wpa"
//   3. "wpa2"
//   4. "none"
#define DEMOCFG_SECURITY        "wpa2"
#define DEMOCFG_PASSPHRASE      "password"

// Possible values 1,2,3,4
#define DEMOCFG_WEP_KEY_INDEX   1
#define DEMOCFG_DEFAULT_DEVICE  BSP_DEFAULT_ENET_DEVICE
#define DEMOCFG_DEFAULT_CIPHER  CCMP	 // TKIP or CCMP

#if !BSPCFG_ENABLE_FLASHX
    #error  BSPCFG_ENABLE_FLASHX needs to be enabled in user_config.h to store Wi-Fi parameters
#else
    #define FLASH_NAME  "flashx:"
#endif

#ifndef DEMOCFG_DEFAULT_DEVICE
    #define DEMOCFG_DEFAULT_DEVICE   BSP_DEFAULT_ENET_DEVICE
#endif 

// Define Task IDs
/*
enum 
{
    HVAC_TASK = 1,
    SHELL_TASK,
    HEARTBEAT_TASK,
    WMICONFIG_TASK1,
    WMICONFIG_TASK2
};
*/

void  HVAC_Task(uint32_t param);
void  Shell_Task(uint32_t param);
void  HeartBeat_Task(uint32_t param);
void  Button_Task(uint32_t param);
void  RControl_Task(uint32_t param);

#endif
