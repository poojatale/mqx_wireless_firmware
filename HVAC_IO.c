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
#include <lwgpio.h>

#if defined BSP_BUTTON1 
   #define TEMP_PLUS      BSP_BUTTON1
#endif
#if defined BSP_BUTTON2 
   #define TEMP_MINUS     BSP_BUTTON2
#endif

#if (!defined BSP_BUTTON1 && !defined BSP_BUTTON2)
   #warning "Buttons are not available in this BSP"
#else
   #define BUTTONS   1
#endif

#if defined(BSP_BUTTON3)
#define FAN_ON_OFF      BSP_BUTTON3
#endif

//#ifdef BSP_LED1
 //  #define LED_1        BSP_LED1
//#endif
//#ifdef BSP_LED2
//   #define LED_2        BSP_LED2
//#endif
#ifdef BSP_LED3
   #define LED_3        BSP_LED3
#endif
#ifdef BSP_LED4
   #define LED_4        BSP_LED4
#endif


const char *HVAC_OutputName[] = { "Fan", "Furnace", "A/C", "HeartBeat" };

bool HVAC_OutputState[HVAC_MAX_OUTPUTS] ={0};
static bool output_port=0,input_port=0;
//#ifdef LED_1
//LWGPIO_STRUCT led1;
//#endif
#ifdef LED_2
LWGPIO_STRUCT led2;
#endif
#ifdef LED_3
LWGPIO_STRUCT led3;
#endif
#ifdef LED_4
LWGPIO_STRUCT led4;
#endif
#ifdef BSP_BUTTON1
LWGPIO_STRUCT button1;
#endif
/*#ifdef BSP_BUTTON2
LWGPIO_STRUCT button2;
#endif
*/
#ifdef BSP_BUTTON3
LWGPIO_STRUCT button3;
#endif



void HVAC_ResetOutputs(void) 
{
   uint32_t i;
   
   for (i=0;i<HVAC_MAX_OUTPUTS;i++) {
      HVAC_SetOutput((HVAC_Output_t)i,FALSE);
   }
}


void HVAC_SetOutput(HVAC_Output_t signal,bool state) 
{    
   /*if (HVAC_OutputState[signal] != state) {
      HVAC_OutputState[signal] = state;
      if (output_port) {
          switch (signal) {
#ifdef LED_1
              case HVAC_FAN_OUTPUT:
                  (state) ? lwgpio_set_value(&led1, LWGPIO_VALUE_HIGH):lwgpio_set_value(&led1, LWGPIO_VALUE_LOW);            	
                   break;
#endif   
#ifdef LED_2
              case HVAC_HEAT_OUTPUT:
                  (state) ? lwgpio_set_value(&led2, LWGPIO_VALUE_HIGH):lwgpio_set_value(&led2, LWGPIO_VALUE_LOW);            	 
                   break;
#endif
#ifdef LED_3
             case HVAC_COOL_OUTPUT:
                  (state) ? lwgpio_set_value(&led3, LWGPIO_VALUE_HIGH):lwgpio_set_value(&led3, LWGPIO_VALUE_LOW);            	 
                   break;
#endif   

         }
      }
   }*/
}


bool HVAC_GetOutput(HVAC_Output_t signal)
{
   return HVAC_OutputState[signal];
} 


char *HVAC_GetOutputName(HVAC_Output_t signal)
{
   return (char *)HVAC_OutputName[signal];
} 


bool HVAC_GetInput(HVAC_Input_t signal)
{
   bool  value=FALSE;
    if (input_port){
        switch (signal) {
#ifdef BSP_BUTTON1 
            case HVAC_TEMP_UP_INPUT:         
               // value = !lwgpio_get_value(&button1);
                break;
#endif  
#ifdef BSP_BUTTON2
            case HVAC_TEMP_DOWN_INPUT:
               // value = !lwgpio_get_value(&button2);
                //added 6/21
                break;
#endif   
#if defined(FAN_ON_OFF)
         case HVAC_FAN_ON_INPUT:
            value = !lwgpio_get_value(&button3);
            break;
#endif   
   }
  }
   return value;
}




static uint32_t    AmbientTemperature = 200; // 20.0 celsius, 68.0 fahrenheit
static TIME_STRUCT LastUpdate  = {0,0};

uint32_t HVAC_GetAmbientTemperature(void)
{
   return AmbientTemperature;
}

void HVAC_ReadAmbientTemperature(void)
{
   TIME_STRUCT time;
   
   _time_get(&time);
   if (time.SECONDS>=(LastUpdate.SECONDS+HVAC_TEMP_UPDATE_RATE)) {
      LastUpdate=time;
      if (HVAC_GetOutput(HVAC_HEAT_OUTPUT)) {
         AmbientTemperature += HVAC_TEMP_UPD_DELTA;
      } else if (HVAC_GetOutput(HVAC_COOL_OUTPUT)) {
         AmbientTemperature -= HVAC_TEMP_UPD_DELTA;
      }
   }
}

// #if ENABLE_ADC

static MQX_FILE_PTR fd_adc, fd_ch = NULL;

const ADC_INIT_STRUCT adc_init = {
    ADC_RESOLUTION_DEFAULT,     /* resolution */
};

const ADC_INIT_CHANNEL_STRUCT adc_ch_param = {
    26, // ADC_CH_HEART,
    ADC_CHANNEL_MEASURE_LOOP | ADC_CHANNEL_START_NOW,
    10,            /* number of samples in one run sequence */
    0,             /* time offset from trigger point in ns */
    50000,         /* period in us (50ms) */
    0x10000,       /* scale range of result (not used now) */
    1,             /* circular buffer size (sample count) */
    0,  // MY_TRIGGER,    /* logical trigger ID that starts this ADC channel */
    NULL,          /* no lwevent used */
    0              /* lwevent bits */
};

/*
 *  Setup ADC module to read in accelerometer and potentiometer values
 */   
void HVAC_InitializeADC(void) {
    fd_adc = fopen( 0, (const char*)&adc_init);
    if (NULL == fd_adc) {    
        printf("ADC device open failed\n");
        return;
    }
    
    fd_ch = fopen( 0, (const char*)&adc_ch_param);
    if (fd_ch == NULL) {    
        printf("%s0 channel open failed\n", 0 );
        return;
    }
}

/*
 *  Read in ADC value on the channel given
 */
_mqx_int ReadADC(void) {
    ADC_RESULT_STRUCT val;
    
    if (fd_ch == NULL)
        return 0;
    read(fd_ch, &val, sizeof(val));
    return (_mqx_int) val.result;
}

// #endif /* ENABLE_ADC*/
