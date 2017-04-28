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

HVAC_PARAMS HVAC_Params = {0};

const char *HVACModeName[] = {"Off", "Cool", "Heat", "Auto"};

// Function Prototypes
void   Switch_Poll( void );

void HVAC_InitializeParameters(void) 
{
   _lwevent_create(&HVAC_Params.Event, 0);
   HVAC_Params.HVACMode = HVAC_Auto;
   HVAC_Params.FanMode = Fan_Automatic;
   HVAC_Params.TemperatureScale = Celsius;
   HVAC_Params.DesiredTemperature = HVAC_DEFAULT_TEMP;
}


bool HVAC_WaitParameters(int32_t timeout) 
{
    bool catched;

    // poll the keys frequenly until the event or timeout expires
    uint32_t poll_wait = BSP_ALARM_FREQUENCY/20; // 50ms
    int32_t  elapsed;
    MQX_TICK_STRUCT tickstart, ticknow;
        
    _time_get_elapsed_ticks(&tickstart);

    do
    {
        Switch_Poll();
        _lwevent_wait_ticks(&HVAC_Params.Event, HVAC_PARAMS_CHANGED, TRUE, poll_wait);

        _time_get_elapsed_ticks(&ticknow);
        elapsed = _time_diff_ticks_int32 (&ticknow, &tickstart, NULL);
            
    } while (elapsed < timeout);
        
    catched = (HVAC_Params.Event.VALUE & HVAC_PARAMS_CHANGED) != 0;
    _lwevent_clear(&HVAC_Params.Event, HVAC_PARAMS_CHANGED);

    return catched;  
}


char *HVAC_HVACModeName(HVAC_Mode_t mode)
{
   return (char *)HVACModeName[mode];
}

uint32_t HVAC_ConvertCelsiusToDisplayTemp(uint32_t temp)
{
   uint32_t  display_temp;
   
   if (HVAC_Params.TemperatureScale ==  Celsius) {
      display_temp = temp;
   } else {
      display_temp = temp*9/5+320;
   }
   
   return display_temp;
}

uint32_t HVAC_ConvertDisplayTempToCelsius(uint32_t display_temp)
{
   uint32_t  temp;
   
   if (HVAC_Params.TemperatureScale ==  Celsius) {
      temp = display_temp;
   } else {
      temp = (display_temp-320)*5/9;
   }
   
   return temp;
}
     

uint32_t HVAC_GetDesiredTemperature(void) {
   return HVAC_ConvertCelsiusToDisplayTemp(HVAC_Params.DesiredTemperature);
}


void HVAC_SetDesiredTemperature(uint32_t temp)
{
   HVAC_Params.DesiredTemperature = HVAC_ConvertDisplayTempToCelsius(temp);
   _lwevent_set(&HVAC_Params.Event, HVAC_PARAMS_CHANGED);
}


FAN_Mode_t HVAC_GetFanMode(void) {
   return HVAC_Params.FanMode;
}


void HVAC_SetFanMode(FAN_Mode_t mode)
{
   HVAC_Params.FanMode = mode;
   _lwevent_set(&HVAC_Params.Event, HVAC_PARAMS_CHANGED);
}


HVAC_Mode_t HVAC_GetHVACMode(void) 
{
   return HVAC_Params.HVACMode;
}


void HVAC_SetHVACMode(HVAC_Mode_t mode)
{
   HVAC_Params.HVACMode = mode;
   _lwevent_set(&HVAC_Params.Event, HVAC_PARAMS_CHANGED);
}


uint32_t HVAC_GetActualTemperature(void) {
   return HVAC_ConvertCelsiusToDisplayTemp(HVAC_State.ActualTemperature);
}


Temperature_Scale_t HVAC_GetTemperatureScale(void) 
{
   return HVAC_Params.TemperatureScale;
}


void HVAC_SetTemperatureScale(Temperature_Scale_t scale)
{
   HVAC_Params.TemperatureScale = scale;
   _lwevent_set(&HVAC_Params.Event, HVAC_PARAMS_CHANGED);
}

char HVAC_GetTemperatureSymbol(void) 
{
   return (HVAC_Params.TemperatureScale==Celsius)?'C':'F';
}


void Switch_Poll(void) 
{
   static bool    InputState[HVAC_MAX_INPUTS] = {0};
   static bool    LastInputState[HVAC_MAX_INPUTS] = {0};
   HVAC_Input_t   input;

   // Save previous input states
   for (input=(HVAC_Input_t)0;input<HVAC_MAX_INPUTS;input++) {
      LastInputState[input]=InputState[input];
      InputState[input]=HVAC_GetInput(input);
   }

   // Check each input for a low-high transition (button press)
   if (InputState[HVAC_TEMP_UP_INPUT]) {
      if (!LastInputState[HVAC_TEMP_UP_INPUT] && !InputState[HVAC_TEMP_DOWN_INPUT]) {
         if (HVAC_GetDesiredTemperature() < HVAC_ConvertCelsiusToDisplayTemp(HVAC_TEMP_MAXIMUM - HVAC_TEMP_SW_DELTA)) 
         {
            HVAC_SetDesiredTemperature(HVAC_GetDesiredTemperature()+HVAC_TEMP_SW_DELTA);
         }
      }
   } else if (InputState[HVAC_TEMP_DOWN_INPUT] && !LastInputState[HVAC_TEMP_DOWN_INPUT] ) {
      if (HVAC_GetDesiredTemperature() > HVAC_ConvertCelsiusToDisplayTemp(HVAC_TEMP_MINIMUM + HVAC_TEMP_SW_DELTA)) 
      {
         HVAC_SetDesiredTemperature(HVAC_GetDesiredTemperature()-HVAC_TEMP_SW_DELTA);
      }
   }

   if (InputState[HVAC_FAN_ON_INPUT] & !LastInputState[HVAC_FAN_ON_INPUT] ) {
      HVAC_SetFanMode(HVAC_GetFanMode()==Fan_On?Fan_Automatic:Fan_On);
   }

   if (InputState[HVAC_HEAT_ON_INPUT] & !LastInputState[HVAC_HEAT_ON_INPUT] ) {
      HVAC_SetHVACMode(HVAC_Heat);
   } else if (InputState[HVAC_AC_ON_INPUT] & !LastInputState[HVAC_AC_ON_INPUT] ) {
      HVAC_SetHVACMode(HVAC_Cool);
   } else if (LastInputState[HVAC_HEAT_ON_INPUT] | LastInputState[HVAC_AC_ON_INPUT]) {
      HVAC_SetHVACMode(HVAC_Off);
   }

   HVAC_ReadAmbientTemperature();
}

/* EOF */
