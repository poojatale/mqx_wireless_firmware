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
*
*
*END************************************************************************/

#include "HVAC.h"
#include "HVAC_Shell_Commands.h"
#include "atheros_driver_includes.h"

const SHELL_COMMAND_STRUCT Shell_commands[] = {
   { "exit",      Shell_exit },      
   { "fan",       Shell_fan },
   { "help",      Shell_help }, 
   { "hvac",      Shell_hvac },
   { "info",      Shell_info },
   { "scale",     Shell_scale },
   { "temp",      Shell_temp },       

   { "netstat",   Shell_netstat },  
   { "ipconfig",  Shell_ipconfig },
   { "iwconfig",  Shell_iwconfig },
   { "wmiconfig", worker_cmd_handler }, //wmiconfig_handler },
   { "wifi", Shell_wifi_params },

#if RTCSCFG_ENABLE_ICMP
   { "ping",      Shell_ping },      
#endif   

   { "?",         Shell_command_list },     
   { NULL,        NULL } 
};

const SHELL_COMMAND_STRUCT Telnet_commands[] = {
   { "exit",      Shell_exit },      
   { "fan",       Shell_fan },
   { "help",      Shell_help }, 
   { "hvac",      Shell_hvac },
   { "info",      Shell_info },

#if RTCSCFG_ENABLE_ICMP
   { "ping",      Shell_ping },      
#endif   

   { "scale",     Shell_scale },
   { "temp",      Shell_temp },
   { "?",         Shell_command_list },     
   
   { NULL,        NULL } 
};

/* EOF */
