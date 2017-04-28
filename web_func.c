//
// 
//  FileName: web_func.c
// 
//  Comments:
// 
//

#include <string.h>
#include <stdlib.h>
#include "defines.h"
#include "rtcs_func.h"
#include "web_func.h"
#include "func.h"
#include "html_util.h"
#include "cgi.h"
#include "defines.h"
#include "global.h"
#include "sensors.h"


UID_LOGIN uidDatabase[ MAX_LOGGED_IN ];


#define RESTART_LOCATION (_CODE_PTR_) 0x0420

//
//   restart_firmware()
//
void
restart_firmware( void )
{
}

//
//   add_uid_to_database()
//
bool
add_uid_to_database( uint32_t seconds, uint32_t ip, UID_LOGIN * uid )
{
    bool reply = FALSE;
    int  k;

    for( k=0; k<MAX_LOGGED_IN; k++ )
        if( strlen( uidDatabase[k].str ) == 0 )
        {
            create_new_uid( seconds, ip, &uidDatabase[k] );
            *uid  = uidDatabase[k]; // Provide a copy to the calling function
            reply = TRUE;
            k     = MAX_LOGGED_IN;  // Exit the for() loop
        }

    return( reply );
}

//
//   remove_uid_from_database()
//
bool
remove_uid_from_database( char * uid_str )
{
    bool reply = FALSE;
    int  k;

    for( k=0; k<MAX_LOGGED_IN; k++ )
        if( strcmp( uidDatabase[k].str, uid_str ) == 0 )
        {
            memset( &uidDatabase[k], 0, sizeof(UID_LOGIN) );
            reply = TRUE;
            k     = MAX_LOGGED_IN;  // Exit the for() loop
        }

    return( reply );
}

//
//  create_new_uid() - This function creates a string that will be used
//                     to identify a user who has successfully logged
//                     in to the web site.
//
void
create_new_uid( uint32_t seconds, uint32_t ip, UID_LOGIN * uid )
{
    int  k;

    for( k=0; k<UID_RANDOM_LENGTH; k++ )
    {
        uid->str[k] = (char)(rand() % 26) + 'a';
    }

    // Ensure null termination. Array length is actually UID_RANDOM_LENGTH + 1
    uid->str[UID_RANDOM_LENGTH] = 0;

    update_uid_time( seconds, uid );

    uid->ip = ip;  
}

//
//   valid_uid()
//
bool
valid_uid( UID_LOGIN * uid )
{
    bool reply = FALSE;
    int  k;

    // Find a matching string. Note that a null string (length = 0) will 
    //   never provide a match.
    //
    for( k=0; k<MAX_LOGGED_IN; k++ )
        if( (strlen(uid->str) == UID_RANDOM_LENGTH) && (strcmp(uidDatabase[k].str, uid->str) == 0) )
            reply = TRUE;

    // If the Remote Lockout has been enabled, always reply "FALSE" to
    //    a UID check.
    //
    if( coreDB.enet_setup.remote_lockout == REMOTE_LOCKOUT_ON )
        reply = FALSE;

    return( reply );
}


//
//  safe_copy_uid_string() - The purpose of this function is to copy a
//                           string containing the UID, into the string
//                           field of the UID_LOGIN structure. The 
//                           source string is typically the string that
//                           is bounced from page to page after a user
//                           has succsessfully logged in.
//                          
//                           This function prevents a potential buffer
//                           overrun if the length of the source string,
//                           from the client browser, is too long.
//
void
safe_copy_uid_string( UID_LOGIN * uid, char *src_str )
{
    strncpy( uid->str, src_str, UID_RANDOM_LENGTH );
    uid->str[ UID_RANDOM_LENGTH ] = 0;
}

//
//  update_uid_time() - This functions updates a previously built 
//                           access string with a new time value
//
void
update_uid_time( uint32_t seconds, UID_LOGIN * uid )
{
    int k;

    // Find a matching string. Note that a null string (length = 0) will 
    //   never provide a match.
    //
    for( k=0; k<MAX_LOGGED_IN; k++ )
        if( (strlen(uid->str) == UID_RANDOM_LENGTH) && (strcmp(uidDatabase[k].str, uid->str) == 0) )
            uidDatabase[k].time = seconds;
}

//
//  uid_login_timeout() - This functions checks if a User ID has timed out.
//                
//    Parameters:       seconds - current time, in seconds
//                      uid     - User ID to check
//        
//    Returns   :       True - user has timed out
//                     False - user "time" remains valid
//
bool
uid_login_timeout( uint32_t seconds, UID_LOGIN * uid )
{
    uint32_t  uid_time, elapsed_time;
    bool      reply;

    uid_time = uid->time;

    // Check for potential rollover condition. This occurs if "uid_time"
    //   was established when the SecCounter (the clock) is near its
    //   maximum value (0xFFFFFFFF).
    //
    //   "near" = < LOGIN_TIMEOUT
    //
    if( seconds < uid_time )
        elapsed_time = (0xFFFFFFFF - uid_time) + 1 + seconds;
    else
        elapsed_time = seconds - uid_time;

    if( elapsed_time > LOGIN_TIMEOUT )
        reply = TRUE;
    else
        reply = FALSE;

    return( reply );
}


//
//   load_default_enet_setup()
//
void
load_default_enet_setup( ENET_SETUP * enet )
{
    enet->static_ip_addr    = DEFAULT_STATIC_IP_ADDR;
    enet->default_gateway   = DEFAULT_GATEWAY;
    enet->subnet_mask       = DEFAULT_SUBNET_MASK;

    enet->http_port_number  = DEFAULT_HTTP_PORT;
    enet->dyn_dns_provider  = DEFAULT_DYN_DNS_PROVIDER;
    enet->addr_mode         = DEFAULT_ENET_ADDR_MODE;
    enet->remote_lockout    = DEFAULT_REMOTE_LOCKOUT;

    strcpy( enet->user_name, DEFAULT_HTTP_USER_NAME );
    strcpy( enet->password,  DEFAULT_HTTP_PASSWORD );

    // Null out the entire site name
    memset( enet->site_name,  0, SIZE_HTTP_SITE_NAME );

    // Use Google's Public DNS Server as a starting point. Users
    //   can replace this with their own.
    enet->dns_server_ip_addr  = GOOGLE_DNS_SERVER_1;
}


//
//   web_sensor_string
//
void
web_sensor_string( SENSOR * sens, int sensor_id, char * str )
{
    if( (sens->setup.sensor_type != SENSOR_TYPE_NONE) && sens->fail )
    {
        strcpy( str, "fail" );
        return;
    }

    switch( sens->setup.sensor_type )
    {
        case SENSOR_TYPE_TEMP_F:     // A99 sensor, units = degrees F
        case SENSOR_TYPE_TEMP_HI_F:  // High Temp sensor, units = degrees F
            build_sensor_data_string( sens->value_int, 0, str );
            strcat( str, " &deg;F" );
        break;
            
        case SENSOR_TYPE_TEMP_C:     // A99 sensor, units = degrees C
        case SENSOR_TYPE_TEMP_HI_C:  // High Temp sensor, units = degrees C
            build_sensor_data_string( sens->value_int, 1, str );
            strcat( str, " &deg;C" );
        break;

        case SENSOR_TYPE_RH:         // Relative humidity, units = %RH
            build_sensor_data_string( sens->value_int, 0, str );
            strcat( str, " &#37;RH" );
        break;

        case SENSOR_TYPE_P_0pt25:
        case SENSOR_TYPE_P_0pt5:       // DPT-2005, units = INWC
            build_sensor_data_string( sens->value_int, 3, str );
            strcat( str, " inwc" );
        break;

        case SENSOR_TYPE_P_2pt5:     // , units = INWC
        case SENSOR_TYPE_P_5:        // , units = INWC
        case SENSOR_TYPE_P_10:       // DPT_2100, units = INWC
            build_sensor_data_string( sens->value_int, 2, str );
            strcat( str, " inwc" );
        break;
            
        case SENSOR_TYPE_P__8:       // P499Rxx-401C, units = bAR
            build_sensor_data_string( sens->value_int, 2, str );
            strcat( str, " bar" );
        break;
            
        case SENSOR_TYPE_P_15:       // P499Rxx-402C, units = bAR
        case SENSOR_TYPE_P_30:       // P499Rxx-404C, units = bAR
        case SENSOR_TYPE_P_50:       // P499Rxx-405C, units = bAR
            build_sensor_data_string( sens->value_int, 1, str );
            strcat( str, " bar" );
        break;
            
        case SENSOR_TYPE_P100:       // P499RxS-101C, units = PSI
            build_sensor_data_string( sens->value_int, 1, str );
            strcat( str, " psi" );
        break;            
            
        case SENSOR_TYPE_P110:       // units = PSI / inHg
            // Special case for sensor type P110. If the PSI value is less
            //   than 0, the units are converted to inHG AND the sign
            //   is reversed (-PSI => +inHg). An exception to this rule
            //   is when it is a differential sensor, in which case the
            //   units are always PSI.
            //
            if( (sens->value_int < 0) && (sensor_id != SENSOR_ID_DIFF) )
            {
                build_sensor_data_string( (int) (sens->value_float * -2.036), 0, str );
                strcat( str, " inHg" );
             }
            else
            {
                build_sensor_data_string( sens->value_int, 1, str );
                strcat( str, " psi" );
            }
        break;            

        case SENSOR_TYPE_P200:       // P499RxP-102C, units = PSI
        case SENSOR_TYPE_P500:       // P499RxS-105C, units = PSI
        case SENSOR_TYPE_P750:       // P499RxS-107C, units = PSI
            build_sensor_data_string( sens->value_int, 0, str );
            strcat( str, " psi" );
        break;            

        case SENSOR_TYPE_BINARY:
            if( sens->value_int == BIN_SENSOR_OPEN )
                strcpy( str, "OPEN" );
            else
                strcpy( str, "CLOSED" );
        break;

        case SENSOR_TYPE_NONE:       // Sensor type is UNCONFIGURED, unused
        default:
            sprintf( str, "n/a" );
        break;
    }
}

//
//    web_build_float_string()
//
void
web_build_float_string( char * str, float value, int num_decimal )
{
    long  lvalue;
    long  rem;
    bool  neg_value;

    neg_value = (value < 0.0) ? TRUE : FALSE;

    if( neg_value )
        value = 0.0 - value;

    switch( num_decimal )
    {
        case 1:
            lvalue = (long) ((value * 10) + 0.5);
            rem    = lvalue % 10;
            if( neg_value )
                sprintf( str, "-%d.%d", lvalue / 10, rem );
            else
                sprintf( str, "%d.%d", lvalue / 10, rem );
        break;

        case 2:
            lvalue = (long) ((value * 100) + 0.5);
            rem    = lvalue % 100;
            if( neg_value )
                sprintf( str, "-%d.%02d", lvalue / 100, rem );
            else
                sprintf( str, "%d.%02d", lvalue / 100, rem );
        break;

        case 3:
            lvalue = (long) ((value * 1000) + 0.5);
            rem    = lvalue % 1000;
            if( neg_value )
                sprintf( str, "-%d.%03d", lvalue / 1000, rem );
            else
                sprintf( str, "%d.%03d", lvalue / 1000, rem );
        break;

        case 0:
        default:
            lvalue = (long) (value + 0.5);
            if( neg_value )
                sprintf( str, "-%d", lvalue );
            else
                sprintf( str, "%d", lvalue );
        break;
    }
}


//
//   web_output_status_string
//
void
web_output_status_string( OUTPUT * out, char * str )
{
    int percent;

    switch( out->setup.output_type )
    {
        case OUTPUT_TYPE_RELAY:     // Output state is On or Off
            strcpy( str, (out->output_state == 0) ? "Off" : "On");
        break;
            
        case OUTPUT_TYPE_ANALOG:   // Output state is 0 - 100%
            // For analog outputs, the "output_state" has a range
            //    of 0 - 255. This value is used directly when
            //    establishing a PWM duty cycle, which ultimately
            //    generates a 0-10 vdc output signal. This 0-255
            //    representation is converted to 0-100% for 
            //    display in a web page.
            percent = (int) out->output_state;
            percent = ((percent * 100) + 128) / 255;

            sprintf( str, "%d%", percent );
        break;
            
        case OUTPUT_TYPE_NONE:     // Unused output
        default:
            strcpy( str, "--");
        break;
    }
}


//
//  build_setpoint_string() - This function builds a string that represents
//                            the setpoint, expressed as an integer, based
//                            on the sensor type.
//
//   For instance, for sensor type "SENSOR_TYPE_P_0pt25", the integer
//   value of 500 is the equivalent of 0.050 inwc.
//
void
build_setpoint_string( int setpoint, int sensor_type, char * str )
{
    switch( sensor_type )
    {
        case SENSOR_TYPE_NONE:       // Sensor type is UNCONFIGURED, unused
        case SENSOR_TYPE_TEMP_F:     // A99 sensor, units = degrees F
        case SENSOR_TYPE_TEMP_HI_F:  // High Temp sensor, units = degrees F
            build_sensor_data_string( setpoint, 0, str );
        break;
            
        case SENSOR_TYPE_TEMP_C:     // A99 sensor, units = degrees C
        case SENSOR_TYPE_TEMP_HI_C:  // High Temp sensor, units = degrees C
            build_sensor_data_string( setpoint, 1, str );
        break;
            
        case SENSOR_TYPE_RH:         // Relative humidity, units = %RH
            build_sensor_data_string( setpoint, 0, str );
        break;
            
        case SENSOR_TYPE_P_0pt25:
        case SENSOR_TYPE_P_0pt5:       // DPT-2005, units = INWC
            build_sensor_data_string( setpoint, 3, str );
        break;

        case SENSOR_TYPE_P_2pt5:     // , units = INWC
        case SENSOR_TYPE_P_5:        // , units = INWC
        case SENSOR_TYPE_P_10:       // DPT_2100, units = INWC
            build_sensor_data_string( setpoint, 2, str );
        break;
            
        case SENSOR_TYPE_P__8:       // P499Rxx-401C, units = bAR
            build_sensor_data_string( setpoint, 2, str );
        break;
            
        case SENSOR_TYPE_P_15:       // P499Rxx-402C, units = bAR
        case SENSOR_TYPE_P_30:       // P499Rxx-404C, units = bAR
        case SENSOR_TYPE_P_50:       // P499Rxx-405C, units = bAR
            build_sensor_data_string( setpoint, 1, str );
        break;
            
        case SENSOR_TYPE_P100:       // P499RxS-101C, units = PSI
        case SENSOR_TYPE_P110:       // , units = PSI
            build_sensor_data_string( setpoint, 1, str );
        break;            

        case SENSOR_TYPE_P200:       // P499RxP-102C, units = PSI
        case SENSOR_TYPE_P500:       // P499RxS-105C, units = PSI
        case SENSOR_TYPE_P750:       // P499RxS-107C, units = PSI
            build_sensor_data_string( setpoint, 0, str );
        break;            

        default:
            sprintf( str, "" );
        break;
    }
}


//
//  build_offset_string() - This function builds a string that represents
//                          the sensor offset, expressed as an integer, based
//                          on the sensor type.
//
//                          Only the temperature sensors support the concept
//                          of a sensor offset. All other sensor types
//                          generate the string "0".
//
void
build_offset_string( int setpoint, int sensor_type, char * str )
{
    switch( sensor_type )
    {
        case SENSOR_TYPE_TEMP_F:     // A99 sensor, units = degrees F
        case SENSOR_TYPE_TEMP_HI_F:  // High Temp sensor, units = degrees F
            build_sensor_data_string( setpoint, 0, str );
        break;
            
        case SENSOR_TYPE_TEMP_C:     // A99 sensor, units = degrees C
        case SENSOR_TYPE_TEMP_HI_C:  // High Temp sensor, units = degrees C
            build_sensor_data_string( setpoint, 1, str );
        break;

        default:
            strcpy( str, "0" );
        break;
    }
}


// 
//  build_setpoint_range_string() -
//
//    This function builds a text string that displays the acceptable
//    setpoint range for a sensor. The range is based on the sensor type,
//    and in the case of differential sensors, on the sensor ID. 
//
//    This string is used in the Relay / Analog setup web pages to
//    indicate the range for On/Off and Sp/Ep input.
//
void
build_setpoint_range_string( int sensor_id, int sensor_type, char * str  )
{
    char min_str[20], max_str[20];
    int  min_sp, max_sp;

    if( sensor_id == SENSOR_ID_DIFF )
    {
        min_sp = get_minimum_diff_setpoint( sensor_type );
        max_sp = get_maximum_diff_setpoint( sensor_type );
    }
    else
    {
        min_sp = get_minimum_setpoint( sensor_type );
        max_sp = get_maximum_setpoint( sensor_type );
    }

    build_setpoint_string( min_sp, sensor_type, min_str );
    build_setpoint_string( max_sp, sensor_type, max_str );

    sprintf( str, "%s to %s", min_str, max_str );

    // Append the units, based on sensor type
    switch( sensor_type )
    {
        case SENSOR_TYPE_TEMP_F:  
        case SENSOR_TYPE_TEMP_HI_F:
            strcat( str, " &deg;F" );   
        break;

        case SENSOR_TYPE_TEMP_C:  
        case SENSOR_TYPE_TEMP_HI_C:
            strcat( str, " &deg;C" );
        break;

        case SENSOR_TYPE_RH:
            strcat( str, " %rH" );
        break;

        case SENSOR_TYPE_P_0pt25:  
        case SENSOR_TYPE_P_0pt5:  
        case SENSOR_TYPE_P_10:
        case SENSOR_TYPE_P_2pt5:
        case SENSOR_TYPE_P_5:
            strcat( str, " inwc" );
        break;

        case SENSOR_TYPE_P__8:
        case SENSOR_TYPE_P_15:
        case SENSOR_TYPE_P_30:
        case SENSOR_TYPE_P_50:
            strcat( str, " bar" );
        break;

        case SENSOR_TYPE_P100:
        case SENSOR_TYPE_P500:
        case SENSOR_TYPE_P750:
        case SENSOR_TYPE_P200:
        case SENSOR_TYPE_P110:
            strcat( str, " psi" );
        break;

        case SENSOR_TYPE_BINARY:
            strcpy( str, "open or closed" );
        break;

        case SENSOR_TYPE_NONE:
        default:
            str[0] = 0;     // Null, empty string these sensor types
        break;
    }
}

//
// build_sensor_data_string() - This routine builds a string containing a
//                              numeric value, with an optional decimal point.
//
// Global Vars Affected:  None 
//
//           Parameters:  value  - the value to be displayed
//                        dec_pt - indicates where to place the decimal point
//                                  3 - thousands digit
//                                  2 - hundreds digit
//                                  1 - tens digit
//                                  0 - no decimal point used
//
//              Returns:  None
//
void 
build_sensor_data_string( int value, int dec_pt, char * str )
{
    char sign_str[4];

    if( value < 0 )
    {
        sign_str[0] = '-';
        sign_str[1] = 0;
        value       = 0 - value;
    }
    else
    {
        sign_str[0] = 0;
    }

    switch( dec_pt )
    {
        case 1:  sprintf( str, "%s%d.%d",  sign_str, value/10,    value%10 );
        break;
 
        case 2:  sprintf( str, "%s%d.%02d", sign_str, value/100,  value%100 );
        break;
 
        case 3:  sprintf( str, "%s%d.%03d", sign_str, value/1000, value%1000 );
        break;

        case 0:
        default:
                 sprintf( str, "%s%d", sign_str, value );
        break;
    }
}


//
//   build_device_revision_string()
//
void
build_device_revision_string( uint32_t sim_sdid, char * str )
{
    uint32_t  temp;

    // An early batch of k30 microcontrollers did not have the SIM_SDID
    //   programmed correctly. This condition is identified when the
    //   value of SIM_SDID = 0x000000ff. If this condition is encountered,
    //   the string is set to "n/a"
    //
    if( sim_sdid == 0x000000FF )
    {
        strcpy( str, "n/a" );
    }
    else
    {
        // Microcontroller Device Revision is in bits 15-12
        temp  = (sim_sdid >> 12); 
        temp &= 0x0000000F;      
        sprintf( str, "%d", temp);    
    }
}

//
//   build_device_family_id_string()
//
void
build_device_family_id_string( uint32_t sim_sdid, char * str )
{
    uint32_t  temp;

    // An early batch of k30 microcontrollers did not have the SIM_SDID
    //   programmed correctly. This condition is identified when the
    //   value of SIM_SDID = 0x000000ff. If this condition is encountered,
    //   the string is set to "n/a"
    //
    if( sim_sdid == 0x000000FF )
    {
        strcpy( str, "n/a" );
    }
    else
    {
        temp  = (sim_sdid >> 4); 
        temp &= 0x00000007;

        switch( temp )
        {
            case 0:   strcpy( str, "k10" );         break;
            case 1:   strcpy( str, "k20" );         break;
            case 2:   strcpy( str, "k30" );         break;
            case 3:   strcpy( str, "k40" );         break;
            case 4:   strcpy( str, "k60" );         break;
            case 5:   strcpy( str, "k70" );         break;
            case 6:   strcpy( str, "k50 / k52" );   break;
            case 7:   strcpy( str, "k51 / k53" );   break;
        }
    }
}

//
//   build_device_pin_count_string()
//
void
build_device_pin_count_string( uint32_t sim_sdid, char * str )
{
    uint32_t  temp;

    // An early batch of k30 microcontrollers did not have the SIM_SDID
    //   programmed correctly. This condition is identified when the
    //   value of SIM_SDID = 0x000000ff. If this condition is encountered,
    //   the string is set to "n/a"
    //
    if( sim_sdid == 0x000000FF )
    {
        strcpy( str, "n/a" );
    }
    else
    {
        // Microcontroller Pin Count is in bits 3-0
        temp  = (sim_sdid & 0x0000000F); 

        switch( temp )
        {
            case  2:   strcpy( str, "32-pin" );    break;
            case  4:   strcpy( str, "48-pin" );    break;
            case  5:   strcpy( str, "64-pin" );    break;
            case  6:   strcpy( str, "80-pin" );    break;
            case  7:   strcpy( str, "81-pin" );    break;
            case  8:   strcpy( str, "100-pin" );   break;
            case  9:   strcpy( str, "121-pin" );   break;
            case 10:   strcpy( str, "144-pin" );   break;
            case 12:   strcpy( str, "196-pin" );   break;
            case 14:   strcpy( str, "256-pin" );   break;
            default:   strcpy( str, "n/a" );       break;
        }
    }
}

//
//   web_blink_comm_leds()
//
// Loading a web page, or an element of a page results in the 
//    blinking of the Recv and Xmit LEDs. The UI Task is
//    responsible for sending this request to the terminal.
//
void     
web_blink_comm_leds( void )
{
    // Beginning with v1.20 code, the slow or "failed" LED blink
    // pattern is used with the C450CEN ethernet model when indicating
    // network communication.
    CommLedRequest.update_recv_led = TRUE;
    CommLedRequest.recv_led        = COMM_LED_FAIL; 
    CommLedRequest.update_xmit_led = TRUE;
    CommLedRequest.xmit_led        = COMM_LED_FAIL; 
}

