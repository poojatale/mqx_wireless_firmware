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
*   Example of shell using RTCS.
*
*
*END************************************************************************/

#include "hvac.h"
#include "cgi.h"
#include "web_func.h"
#include "global.h"
#include <string.h>
#include <stdlib.h>


extern LWSEM_STRUCT      USB_Stick;

static _mqx_int usb_status_fn(HTTPSRV_SSI_PARAM_STRUCT* param);

const HTTPSRV_SSI_LINK_STRUCT fn_lnk_tbl[] = { { "usb_status_fn", usb_status_fn }, { 0, 0 } };

const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    { "hvacdata",     cgi_hvac_data,   0 },
    { "hvacoutput",   cgi_hvac_output, 0 },
    { "status_data",  cgi_status_data, 0 },
    { "adc_data",     cgi_adc_data,    0 },
    { "usbstat",      cgi_usbstat,     0 },
    { "web_data",     cgi_web_data,    0 },
    { "write_relay",  cgi_write_relay, 0 },
    { 0, 0 }    // DO NOT REMOVE - last item - end of table
};


char  cgiResp[1000];


//
//    cgi_status_data() - This CGI call provides the status data that
//                        appears in the overview pages. It includes
//                        up to 4 sensors (Sn-1,2,3 and Sn-d) and
//                        the output state of up to 10 outputs.
//
//    For each parameter a string is created and transmitted to the
//    caller (Java Script in the web page). 
//  
//    Parameters are delimited with '\n'
//
//    Using a zero based index for the strings, these parameters
//    are supplied;
//
//      Index       Parameter
//        0         Number of sensors, 3 or 4
//        1         Number of outputs
//        2         Sn-1 status
//        3         Sn-2 status
//        4         Sn-3 status
//        5         Sn-d status
//      6-15        Output 1 - 10 state (On/Off or 0-100%)
//
static _mqx_int  
cgi_status_data( HTTPSRV_CGI_REQ_STRUCT * param )
{
    HTTPSRV_CGI_RES_STRUCT response;
    char                   str[60];
    int                    k;

    TIME_STRUCT  time;
    uint32_t     min;
    uint32_t     hour;
    uint32_t     sec;
    
    if (param->request_method != HTTPSRV_REQ_GET)
        return(0);

    _time_get(&time);
    
    sec = time.SECONDS % 60;
    min = time.SECONDS / 60;
    hour = min / 60;
    min %= 60;

    uint32_t Ta = HVAC_GetActualTemperature();
    uint32_t Td = HVAC_GetDesiredTemperature();
    
  

    // The overview web page displays either 3 or 4 sensors. At a minimum,
    //    the three sensors Sn-1, Sn-2, and Sn-3 are always displayed, 
    //    whether they have been configured or not. The fourth sensor, Sn-d
    //    is the differential sensor. It is only displayed if it has
    //    been configured (Sn-1 type = Sn-2 type), and referenced by one
    //    or more outputs.
    //
  /*  strcpy( str, "4\n" );
    strcpy( cgiResp, str );       // 1st parameter is the number of sensors

    sprintf( str, "%d\n", 1 );
    strcat( cgiResp, str );       // 2nd parameter is the number of outputs
*/
                                  // 3rd parameter is the Sn-1 status
    //sprintf( str, "%d.%d C\n", Ta / 10, Ta % 10 ); 
    /*if(gRelayState == 1){
    sprintf( str, "ON\n");
    }else{
       sprintf( str, "OFF\n");
    }
      strcat( cgiResp, str );      // Append delimiter

//    strcat( cgiResp, "112 F" );   // 4th parameter is the Sn-2 status
//    strcat( cgiResp, "\n" );      // Append delimiter

    web_build_float_string( str, coreDB.cpu_temp, 3 );
    strcat( str, " F\n" );
    strcat( cgiResp, str );
*/
    //sprintf( str, "%d\n", min );  // 5th parameter is the minutes
    //strcat( cgiResp, str ); 

   // sprintf( str, "%d\n", sec );  // 6th parameter is the seconds
    //strcat( cgiResp, str );  

    //sprintf( str, "%d.%d\n", Td / 10, Td % 10 );  // 7th parameter is the seconds
   // strcat( cgiResp, str );  

    //for( k=1; k<10; k++ )
    //{
      //  strcat( cgiResp, "0" );   // Parameters 8-15 are output status
       // strcat( cgiResp, "\n" );  // Append delimiter
    //}

    response.ses_handle     = param->ses_handle;
    response.content_type   = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code    = 200;
    response.data           = cgiResp;
    response.data_length    = strlen( cgiResp );
    response.content_length = response.data_length;

    HTTPSRV_cgi_write( &response ); 

    return( response.content_length );
}

_mqx_int cgi_web_data(  HTTPSRV_CGI_REQ_STRUCT * param )
{
    HTTPSRV_CGI_RES_STRUCT response;
    char                   str[60];
    
    if (param->request_method != HTTPSRV_REQ_GET)
        return(0);
     
    
     
    if(gRelayState == 1){
    sprintf( cgiResp, "ON \n");
    }else{
       sprintf( cgiResp, "OFF \n");
    }
      
    response.ses_handle     = param->ses_handle;
    response.content_type   = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code    = 200;
    response.data           = cgiResp;
    response.data_length    = strlen( cgiResp );
    response.content_length = response.data_length;

    HTTPSRV_cgi_write( &response ); 

    return( response.content_length );
}

//
//    cgi_adc_data() - debugging tool, used with ""adc.shtm"
//
//      Index       Parameter
//        0         Sn-1 raw ADC
//        1         Sn-2 raw ADC
//        2         Sn-3 raw ADC
//        3         5v raw ADC
//        4         10v raw ADC
//        5         CPU Temp raw ADC
//
//        6         Sn-1 eng units
//        7         Sn-2 eng units
//        8         Sn-3 eng units
//        9         5v eng units (vdc)
//       10         10v eng units (vdc)
//       11         CPU Temp eng units (F)
//
//       12         Sn-1 resistance or voltage
//       13         Sn-2 resistance or voltage
//       14         Sn-3 resistance or voltage
//
_mqx_int  cgi_adc_data( HTTPSRV_CGI_REQ_STRUCT * param )
{
    HTTPSRV_CGI_RES_STRUCT response;
    char                   str[80];

    if( param->request_method != HTTPSRV_REQ_GET )
        return( 0 );

    web_blink_comm_leds();

    // The 1st parameter is Sn-1, raw ADC
    sprintf( str, "%d\n", Sample[IDX_ANA_SENSOR_1].raw );     
    strcpy( cgiResp, str );

    // The 2nd parameter is Sn-2, raw ADC
    sprintf( str, "%d\n", Sample[IDX_ANA_SENSOR_2].raw );  
    strcat( cgiResp, str );

    // The 3rd parameter is Sn-3, raw ADC
    sprintf( str, "%d\n", Sample[IDX_ANA_SENSOR_3].raw );     
    strcat( cgiResp, str );

    // The 4th parameter is 5v ext, raw ADC
    sprintf( str, "%d\n", Sample[IDX_ANA_5_VOLT].raw );     
    strcat( cgiResp, str );

    // The 5th parameter is 10v ext, raw ADC
    sprintf( str, "%d\n", Sample[IDX_ANA_10_VOLT].raw );     
    strcat( cgiResp, str );

    // The 6th parameter is CPU Temp, raw ADC
    sprintf( str, "%d\n", Sample[IDX_ANA_CPU_TEMP].raw );     
    strcat( cgiResp, str );

    web_build_float_string( str, coreDB.sensor[SENSOR_ID_ONE].value_float, 3 );   
    strcat( str, SensorUnits[ coreDB.sensor[SENSOR_ID_ONE].setup.sensor_type ] );
    strcat( cgiResp, str );
    strcat( cgiResp, "\n" );

    web_build_float_string( str, coreDB.sensor[SENSOR_ID_TWO].value_float, 3 );   
    strcat( str, SensorUnits[ coreDB.sensor[SENSOR_ID_TWO].setup.sensor_type ] );
    strcat( cgiResp, str );
    strcat( cgiResp, "\n" );

    web_build_float_string( str, coreDB.sensor[SENSOR_ID_THREE].value_float, 3 ); 
    strcat( str, SensorUnits[ coreDB.sensor[SENSOR_ID_THREE].setup.sensor_type ] );
    strcat( cgiResp, str );
    strcat( cgiResp, "\n" );

    web_build_float_string( str, coreDB.five_volt_ext, 3 ); 
    strcat( str, " vdc\n" );
    strcat( cgiResp, str );

    web_build_float_string( str, coreDB.ten_volt_ref, 3 );
    strcat( str, " vdc\n" );
    strcat( cgiResp, str );

    web_build_float_string( str, coreDB.cpu_temp, 3 );
    strcat( str, " F\n" );
    strcat( cgiResp, str );

    web_build_float_string( str, coreDB.sensor[SENSOR_ID_ONE].signal, 3 );  
    if( resistive_input( coreDB.sensor[SENSOR_ID_ONE].setup.sensor_type ) )
        strcat( str, " ohms\n" );
    else
        strcat( str, " vdc\n" );
    strcat( cgiResp, str );

    web_build_float_string( str, coreDB.sensor[SENSOR_ID_TWO].signal, 3 );    
    if( resistive_input( coreDB.sensor[SENSOR_ID_TWO].setup.sensor_type ) )
        strcat( str, " ohms\n" );
    else
        strcat( str, " vdc\n" );
    strcat( cgiResp, str );

    web_build_float_string( str, coreDB.sensor[SENSOR_ID_THREE].signal, 3 );  
    if( resistive_input( coreDB.sensor[SENSOR_ID_THREE].setup.sensor_type ) )
        strcat( str, " ohms\n" );
    else
        strcat( str, " vdc\n" );
    strcat( cgiResp, str );

    response.ses_handle     = param->ses_handle;
    response.content_type   = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code    = 200;
    response.data           = cgiResp;
    response.data_length    = strlen( cgiResp );
    response.content_length = response.data_length;

    HTTPSRV_cgi_write( &response ); 

    return( response.content_length );
}



static bool usbstick_attached()
{
    return FALSE;
}

static _mqx_int usb_status_fn(HTTPSRV_SSI_PARAM_STRUCT* param)
{
    char* str;
    
    if (usbstick_attached())
    {
        str = "visible";
    } 
    else
    {
        str = "hidden";
    }
    HTTPSRV_ssi_write(param->ses_handle, str, strlen(str));
    return 0;
}

static _mqx_int cgi_usbstat(HTTPSRV_CGI_REQ_STRUCT* param)
{
    HTTPSRV_CGI_RES_STRUCT response;
    
    if (param->request_method != HTTPSRV_REQ_GET)
    {
        return(0);
    }
    
    response.ses_handle = param->ses_handle;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code = 200;
    
    if (usbstick_attached())
    {
        response.data = "visible\n";
    }
    else
    {
        response.data = "hidden\n";
    }
    response.data_length = strlen(response.data);
    response.content_length = response.data_length;
    HTTPSRV_cgi_write(&response);
    return (response.content_length);
}

_mqx_int cgi_write_relay( HTTPSRV_CGI_REQ_STRUCT * param )
{
   
  HTTPSRV_CGI_RES_STRUCT response;
  uint32_t  len = 0;
  int       read_len;
  char      buffer[60];
 
 
    
    if (param->request_method != HTTPSRV_REQ_POST)
        return(0);

        
    response.ses_handle = param->ses_handle;
    response.content_type = HTTPSRV_CONTENT_TYPE_HTML;
    response.status_code = 200;
    
    //len = param->content_length;
    read_len = (param->content_length < sizeof(buffer)) ? param->content_length : sizeof(buffer);
    len = HTTPSRV_cgi_read( param->ses_handle, buffer, read_len ); //read post from browser 
    
    //parse buffer so that we can evaluate on/off state of relay
    if(buffer[18] == 'f'){
      gRelayState = 0;
    }
    else{
      gRelayState = 1;
    }
    
   
    

//    response.data = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"
//        "<html><head><title>HVAC Settings response</title>"
//        "<meta http-equiv=\"REFRESH\" content=\"0;url=hvac.shtml\"></head>\n<body>\n";
    response.data = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">"
        "<html><head><title>HVAC Settings response</title>"
        "<meta http-equiv=\"REFRESH\" content=\"0;url=index.shtm\"></head>\n<body>\n";
    response.data_length = strlen(response.data);
    response.content_length = response.data_length;
    HTTPSRV_cgi_write(&response);
    return( response.content_length );
  
}
