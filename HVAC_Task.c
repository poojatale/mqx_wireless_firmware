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
*
*
*END************************************************************************/


#include "hvac.h"
#include "hvac_public.h"
#include "hvac_private.h"
#include "global.h"
#include <ipcfg.h>
#include <lwgpio.h>


#if ! BSPCFG_ENABLE_IO_SUBSYSTEM
#error This application requires BSPCFG_ENABLE_IO_SUBSYSTEM defined non-zero in user_config.h. Please recompile BSP with this option.
#endif


#ifndef BSP_DEFAULT_IO_CHANNEL_DEFINED
#error This application requires BSP_DEFAULT_IO_CHANNEL to be not NULL. Please set corresponding BSPCFG_ENABLE_TTYx to non-zero in user_config.h and recompile BSP with this option.
#endif

#define RED_LED_ON     GPIOA_PCOR = 0x00000002  // Clear Port A1
#define RED_LED_OFF    GPIOA_PSOR = 0x00000002  // Set   Port A1
#define RED_LED_TGL    GPIOA_PTOR = 0x00000002  // Tgl   Port A1

#define GREEN_LED_ON   GPIOA_PCOR = 0x00000004  // Clear Port A2
#define GREEN_LED_OFF  GPIOA_PSOR = 0x00000004  // Set   Port A2
#define GREEN_LED_TGL  GPIOA_PTOR = 0x00000004  // Tgl   Port A2


#define BLUE_LED_ON    GPIOD_PCOR = 0x00000020
#define BLUE_LED_OFF   GPIOD_PSOR = 0x00000020
#define BLUE_LED_TGL   GPIOD_PTOR = 0x00000020

#define OFF_BOARD_LED_ON  GPIOD_PCOR = 0x00000004 //clear PortD pin 2 (0.0v)
#define OFF_BOARD_LED_OFF GPIOD_PSOR = 0x00000004 //set PortD pin 2 (3.3v)

#define FAN_OFF        GPIOC_PCOR = 0x00000010 //clear Port C4
#define FAN_ON         GPIOC_PSOR = 0x00000010 //Set Port C4


int GPIOC_PSOR_MASK = 0x00000000;
int GPIOC_PDIR_MASK  = 0x00000002;
int GPIOB_PDIR_MASK  = 0x00020000;


HVAC_STATE  HVAC_State =  {HVAC_Off};



void HVAC_Task(uint32_t param)
{
   HVAC_Mode_t mode;
   uint32_t counter = HVAC_LOG_CYCLE_IN_CONTROL_CYCLES;

   /* Initialize operating parameters to default values */
   HVAC_InitializeParameters();


   /* Configure and reset outputs */
  // HVAC_InitializeIO();
//   _task_create(0, HEARTBEAT_TASK, 0);


   while( TRUE ) 
   {
      // Read current temperature
      HVAC_State.ActualTemperature = HVAC_GetAmbientTemperature();

      // Examine current parameters and set state accordingly 
      HVAC_State.HVACState = HVAC_Off;
      HVAC_State.FanOn = FALSE;

      mode = HVAC_GetHVACMode();
      
      if (mode == HVAC_Cool || mode == HVAC_Auto)  
      {
         if (HVAC_State.ActualTemperature > (HVAC_Params.DesiredTemperature+HVAC_TEMP_TOLERANCE)) 
         {
            HVAC_State.HVACState = HVAC_Cool;
            HVAC_State.FanOn = TRUE;
         }
      } 
      
      if (mode == HVAC_Heat || mode == HVAC_Auto) 
      {
         if (HVAC_State.ActualTemperature < (HVAC_Params.DesiredTemperature-HVAC_TEMP_TOLERANCE)) 
         {
            HVAC_State.HVACState = HVAC_Heat;
            HVAC_State.FanOn = TRUE;
         }
      }

      if (HVAC_GetFanMode() == Fan_On) {
         HVAC_State.FanOn = TRUE;
      }         

      // Set outputs to reflect new state 
   //   HVAC_SetOutput( HVAC_FAN_OUTPUT,  HVAC_State.FanOn );
    //  HVAC_SetOutput( HVAC_HEAT_OUTPUT, HVAC_State.HVACState == HVAC_Heat );
     // HVAC_SetOutput( HVAC_COOL_OUTPUT, HVAC_State.HVACState == HVAC_Cool );

      // Log Current state 
      if( ++counter >= HVAC_LOG_CYCLE_IN_CONTROL_CYCLES ) 
      {
         counter = 0;       
      }
      
      // Wait for a change in parameters, or a new cycle 
      if( HVAC_WaitParameters(HVAC_CONTROL_CYCLE_IN_TICKS) ) 
      {
         counter = HVAC_LOG_CYCLE_IN_CONTROL_CYCLES;
      }
      
      ipcfg_task_poll();

   }
}


/*TASK*-----------------------------------------------------------------
*
* Function Name  : HeartBeat_Task
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

void HeartBeat_Task(uint32_t param)
{ 
    _mqx_int  value = 0;
    //RED_LED_OFF;
    //GREEN_LED_OFF;
    
    while (TRUE) 
    {
      
      _time_delay( 1000 );    

      if(gRelayState == 0)
      {
        GREEN_LED_OFF;
        if(value == 0)
        {
            RED_LED_ON;  
        }
        else
        {
          RED_LED_OFF;
        }
          value ^= 1;   // toggle next value 
      }  
         
    }
}
/*End of HeartBeat_Task*/


int read_button(void)
{
  if((GPIOC_PDIR &= GPIOC_PDIR_MASK) == 0) return 1; //button pressed
  else return 0; 
}


/*TASK*-----------------------------------------------------------------
*
* Function Name  : Button_Task
* Returned Value : void
* Comments       :
*
*END------------------------------------------------------------------*/

int led_state = 0;
void Button_Task(uint32_t param)
{
  int button_pressed, button_previously_pressed;
  RED_LED_OFF;
  
  while(TRUE)
  {
    _time_delay( 100 );
    button_pressed = read_button(); //read current button state
  
      if(button_pressed == 1 && button_previously_pressed == 0){ //button is now pressed for the first time
        if(gRelayState == 1){
          gRelayState = 0;
        }
        else{
          gRelayState = 1;
      }      
    }
      button_previously_pressed = button_pressed; 
  }
}
void RControl_Task(uint32_t param)
{
  while( TRUE )
  {
    _time_delay( 100 );
  
      if( gRelayState == 1 ){
        FAN_ON;
        GREEN_LED_ON;
        RED_LED_OFF;
        OFF_BOARD_LED_OFF;
      }
      else{
        FAN_OFF;
        GREEN_LED_OFF;
        RED_LED_OFF;
        OFF_BOARD_LED_ON;
      }
  }
    
  
  
}