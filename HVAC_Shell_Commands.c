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
#include <string.h>
#include <shell.h>

#include "hvac_public.h"
#include "HVAC_Shell_Commands.h"
#include "WiFi_GT202.h"



/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_temp
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_temp(int32_t argc, char *argv[] )
{
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   uint32_t           temp,temp_fract;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > 2) {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         if (argc == 2) {
            temp_fract = 0;
            if (sscanf(argv[1],"%d.%d",&temp,&temp_fract)>=1) {
               if (temp_fract<10) {
                  HVAC_SetDesiredTemperature(temp*10+temp_fract);
               } else {
                  printf("Invalid temperature specified, format is dd.d\n");
               }
            } else {
               printf("Invalid temperature specified\n");
            } 
         }

         temp  = HVAC_GetDesiredTemperature();
         printf("Desired Temperature is %d.%1d %c\n", temp/10, temp%10, HVAC_GetTemperatureSymbol());
         temp  = HVAC_GetActualTemperature();
         printf("Actual Temperature is %d.%1d %c\n", temp/10, temp%10, HVAC_GetTemperatureSymbol());
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<temperature>]\n", argv[0]);
      } else  {
         printf("Usage: %s [<temperature>]\n", argv[0]);
         printf("   <temperature> = desired temperature (degrees)\n");
      }
   }
   return return_code;
} 


/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_scale
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_scale(int32_t argc, char *argv[] )
{
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   uint32_t           temp;
 
   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > 2) {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         if (argc == 2) {
            if (*argv[1] == 'f') {
               HVAC_SetTemperatureScale(Fahrenheit);
            } else if (*argv[1] == 'c') {
               HVAC_SetTemperatureScale(Celsius);
            } else {
               printf("Invalid temperature scale\n");
            } 
         }

         if (HVAC_GetTemperatureScale() == Fahrenheit) {
            printf("Temperature scale is Fahrenheit\n");
         } else {
            printf("Temperature scale is Celsius\n");
         }         
         temp  = HVAC_GetDesiredTemperature();
         printf("Desired Temperature is %d.%1d %c\n", temp/10, temp%10, HVAC_GetTemperatureSymbol());
         temp  = HVAC_GetActualTemperature();
         printf("Actual Temperature is %d.%1d %c\n", temp/10, temp%10, HVAC_GetTemperatureSymbol());
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [c | f]\n", argv[0]);
      } else  {
         printf("Usage: %s [c | f]\n", argv[0]);
         printf("   c = switch to Celsius scale\n");
         printf("   f = switch to Fahrenheit scale\n");
      }
   }
   return return_code;
} 

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_fan
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_fan(int32_t argc, char *argv[] )
{
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   FAN_Mode_t        fan;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > 2) {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         if (argc == 2) {
            if (strcmp(argv[1],"on")==0) {
               HVAC_SetFanMode(Fan_On);
            } else if (strcmp(argv[1],"off")==0) {
               HVAC_SetFanMode(Fan_Automatic);
            } else {
               printf("Invalid fan mode specified\n");
            } 
         }

         fan  = HVAC_GetFanMode();
         printf("Fan mode is %s\n", fan == Fan_Automatic ? "Automatic" : "On");
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<mode>]\n", argv[0]);
      } else  {
         printf("Usage: %s [<mode>]\n", argv[0]);
         printf("   <mode> = on or off (off = automatic mode)\n");
      }
   }
   return return_code;
} 

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_hvac
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_hvac(int32_t argc, char *argv[] )
{
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > 2) {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         if (argc == 2) {
            if (strcmp(argv[1],"off")==0) {
               HVAC_SetHVACMode(HVAC_Off);
            } else if (strcmp(argv[1],"cool")==0) {
               HVAC_SetHVACMode(HVAC_Cool);
            } else if (strcmp(argv[1],"heat")==0) {
               HVAC_SetHVACMode(HVAC_Heat);
            } else if (strcmp(argv[1],"auto")==0) {
               HVAC_SetHVACMode(HVAC_Auto);
            } else {
               printf("Invalid hvac mode specified\n");
            } 
         }

         printf("HVAC mode is %s\n", HVAC_HVACModeName(HVAC_GetHVACMode()));
      }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s [<mode>]\n", argv[0]);
      } else  {
         printf("Usage: %s [<mode>]\n", argv[0]);
         printf("   <mode> = off, cool, heat or auto\n");
      }
   }
   return return_code;
} 

/*FUNCTION*-------------------------------------------------------------------
*
* Function Name    :   Shell_info
* Returned Value   :  int32_t error code
* Comments  :  Reads from a file .
*
*END*---------------------------------------------------------------------*/

int32_t  Shell_info(int32_t argc, char *argv[] )
{
   bool           print_usage, shorthelp = FALSE;
   int32_t            return_code = SHELL_EXIT_SUCCESS;
   uint32_t           temp;
   FAN_Mode_t        fan;
   HVAC_Output_t     output;

   print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if (argc > 1) {
         printf("Error, invalid number of parameters\n");
         return_code = SHELL_EXIT_ERROR;
         print_usage=TRUE;
      } else {
         printf("HVAC mode:    %s\n",  HVAC_HVACModeName(HVAC_GetHVACMode()));
         temp  = HVAC_GetDesiredTemperature();
         printf("Desired Temp: %d.%1d %c\n", temp/10, temp%10, HVAC_GetTemperatureSymbol());
         temp  = HVAC_GetActualTemperature();
         printf("Actual Temp:  %d.%1d %c\n", temp/10, temp%10, HVAC_GetTemperatureSymbol());
         fan  = HVAC_GetFanMode();
         printf("Fan mode:     %s\n", fan == Fan_Automatic ? "Automatic" : "On");

         for(output=(HVAC_Output_t)0;output<HVAC_MAX_OUTPUTS;output++) {
            printf("%12s: %s\n", HVAC_GetOutputName(output), HVAC_GetOutput(output) ? "On" : "Off");
         }
       }
   }
   
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s \n", argv[0]);
      } else  {
         printf("Usage: %s \n", argv[0]);
      }
   }
   return return_code;
} 

/*FUNCTION*-------------------------------------------------------------
*
*  Function Name :  Shell_wifi_params
*  Returned Value:  none
*  Comments  :  displays or changes Wi-Fi parameters
*
*END*-----------------------------------------------------------------*/
int32_t  Shell_wifi_params(int32_t argc, char * argv[] )
  { /* Body */
     boolean              print_usage, shorthelp = FALSE;
     int32_t               return_code = SHELL_EXIT_SUCCESS;

     print_usage = Shell_check_help_request(argc, argv, &shorthelp );

   if (!print_usage)  {
      if(argc <= 1)
      {
          // No command, just print params
          wifi_param_print(gp_WIFI_Params);
          printf("   Wi-Fi %s\n", wifi_get_property(gp_WIFI_Params, WIFI_WIFI_CONNECTED) ? "Connected" : "Not Connected");
        #if DEMOCFG_ENABLE_DEVICECLOUD_IO
          printf("   Cloud Server %s\n", (eDevConnected ==  dcio_get_connection_status()) ? "Connected" : "Not Connected");
        #endif //DEMOCFG_ENABLE_DEVICECLOUD_IO
      }
      else {
          // Command should be present
          if(strcmp(argv[1], "set") == 0)
          {
              if(strcmp(argv[2], "ssid") == 0)
              {
                  if(argc >= 4)
                  {
                      char str[WIFI_SSID_LENGTH];
                      _mqx_uint     params=4, str_length=0;
                      
                      str[0] = 0;   // Initialize to NULL
                      
                      // this loop allows for SSIDs with spaces to be entered
                      while(params <= argc)
                      {
                          // Check length of SSID string
                          str_length += strlen(argv[params-1]);
                          if(argc > params)
                              // account for space character
                              str_length++;
                                
                          if(str_length >= WIFI_SSID_LENGTH)
                          {
                             printf("Error, SSID cannot exceed %d characters\n", WIFI_SSID_LENGTH);
                             return_code = SHELL_EXIT_ERROR;
                             print_usage=TRUE;
                             break;
                          }
                          
                          strcat(str, argv[params-1]);      // copy shell param to string
                          
                          if(argc > params)
                              // There are more params to add, add space character
                              strcat(str, " ");
                          
                          params++; // move to next shell parameter
                      }
                      
                      if(print_usage==FALSE)
                      {
                          wifi_set_param(gp_WIFI_Params, WIFI_SET_SSID, str);
                          printf("Wi-Fi parameters are now:\n");
                          wifi_param_print(gp_WIFI_Params);
                      }
                  } else
                  {
                       printf("Error, SSID parameter needs a value after it\n");
                       return_code = SHELL_EXIT_ERROR;
                       print_usage=TRUE;
                  }
              }
              else if(argc != 4)
              {
                 printf("Error, set command needs parameter and value\n");
                 return_code = SHELL_EXIT_ERROR;
                 print_usage=TRUE;
              }
              else 
              {
                  if(strcmp(argv[2], "mode") == 0)
                  {
                      wifi_set_param(gp_WIFI_Params, WIFI_SET_MODE, argv[3]);
                      printf("Wi-Fi parameters are now:\n");
                      wifi_param_print(gp_WIFI_Params);
                  } else if(strcmp(argv[2], "sec") == 0)
                  {
                      wifi_set_param(gp_WIFI_Params, WIFI_SET_SEC, argv[3]);
                      printf("Wi-Fi parameters are now:\n");
                      wifi_param_print(gp_WIFI_Params);
                  } else if(strcmp(argv[2], "pass") == 0)
                  {
                      wifi_set_param(gp_WIFI_Params, WIFI_SET_PASS, argv[3]);
                      printf("Wi-Fi parameters are now:\n");
                      wifi_param_print(gp_WIFI_Params);
                  } else if(strcmp(argv[2], "cipher") == 0)
                  {
                      wifi_set_param(gp_WIFI_Params, WIFI_SET_CIPHER, argv[3]);
                      printf("Wi-Fi parameters are now:\n");
                      wifi_param_print(gp_WIFI_Params);
                  } else if(strcmp(argv[2], "key") == 0)
                  {
                      wifi_set_param(gp_WIFI_Params, WIFI_SET_WEP_KEY, argv[3]);
                      printf("Wi-Fi parameters are now:\n");
                      wifi_param_print(gp_WIFI_Params);
                  } else
                  {
                      printf("Error, invalid parameter for set command\n");
                      return_code = SHELL_EXIT_ERROR;
                      print_usage=TRUE;
                  }
              }
          }
          else if(strcmp(argv[1], "connect") == 0)
          {
              wifi_param_connect(gp_WIFI_Params);
          }
          else if(strcmp(argv[1], "erase") == 0)
              wifi_flash_erase(gp_WIFI_Params);
          else if(strcmp(argv[1], "program") == 0)
              wifi_flash_program(gp_WIFI_Params);
          else
          {
              printf("Error, invalid command\n");
              return_code = SHELL_EXIT_ERROR;
              print_usage=TRUE;                  
          }
      }
   }
     
   if (print_usage)  {
      if (shorthelp)  {
         printf("%s set or read Wi-Fi parameters, or connect\n", argv[0]);
      } else  {
         printf("Usage: %s\n", argv[0]);
         printf("   set <param> <value> - sets Wi-Fi parameter to value\n");
         printf("   connect - Connect to Access Point using Wi-Fi parameters\n");
         printf("   program - Programs Wi-Fi parameters into internal flash\n");
         printf("   erase - Erases flash storing Wi-Fi parameters\n");
         printf("   <no arguments> - prints Wi-Fi parameters\n");
         printf("   <param> options:\n");
         printf("       ssid    - Wi-Fi network name\n");
         printf("       mode    - Network mode: managed\n");
         printf("       sec     - Security type: none, wpa, wpa2, wep\n");
         printf("       pass    - Security password, or WEP key\n");
         printf("       cipher  - encryption for WPA or WPA2: CCMP or TKIP\n");
         printf("       key     - WEP Key index\n");
      }
   }
     return return_code;
   
  } /* Endbody */

  
/* EOF*/
