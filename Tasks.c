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
*END************************************************************************/

#include "HVAC.h"
#include "atheros_driver_includes.h"

// MQX initialization information

const TASK_TEMPLATE_STRUCT MQX_template_list[] =
{
    // Task Index,      Function,         Stack,  Priority, Name,         Attributes,            Param,  Time Slice 
    { WMICONFIG_TASK1,  wmiconfig_Task1,  3000,    9,       "WMICONFIG1", MQX_AUTO_START_TASK,   0,      0     },   
    { WMICONFIG_TASK2,  wmiconfig_Task2,  3000,    9,       "WMICONFIG2", MQX_AUTO_START_TASK,   0,      0     },
    { HVAC_TASK,        HVAC_Task,        1400,   10,       "HVAC",       MQX_AUTO_START_TASK,   0,      0     },

#if DEMOCFG_ENABLE_SERIAL_SHELL
    { SHELL_TASK,       Shell_Task,       2500,   15,       "Shell",      MQX_AUTO_START_TASK,   0,      0     },
#endif

    { HEARTBEAT_TASK,   HeartBeat_Task,   1500,   14,       "HeartBeat",  0,                     0,      0     },
    { 0 }
};

 
/* EOF */
