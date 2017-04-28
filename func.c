/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : func.c

PURPOSE   : Contains general functions required by this application.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include "func.h" 
#include "defines.h"
#include "global.h"
//#include "eeprom.h"

#include "web_func.h"

#define CRC_8_SEED    0xFF  // Starting CRC value in calc_i2c_crc()

// The text that forms an error message is preceded by a new line,
//    spaces, and a "bullet" point.
//
#define ERROR_MESSAGE_PROMPT "<br/>&bull;  "



//
//   update_sensor_pointers()
//
//   The purpose of this function is to maintain the proper relation
//   between the "sensor pointers", which are part of the type "OUTPUT"
//   structure, and the sensor that is being referenced. The sensor
//   that is referenced is defined by the "SENS" parameter that is
//   part of an output's setup information.
//
//   When the content of an output setup structure is changed, particularly
//   the reference sensor ID, these pointers need to be changed, such 
//   that they point to the sensor structure as indicated by the reference
//   sensor ID.
//
void           
update_sensor_pointers( DATABASE *db )
{
    int sens_id, k;

    // Re-establish the sensor pointer, which may have changed
    //   with the new output setup parameters (sensor_id). 
    for( k=0; k<MAX_OUTPUTS; k++ )
    {
        sens_id              =  db->output[k].setup.output.relay.sensor_id;
        db->output[k].sensor = &db->sensor[ sens_id ];
    }
}


//
//  get_sensor_type_from_id() - Given a specific Sensor ID, this function
//                              determines what type of sensor has been
//                              configured for that sensor input (Sn-1,2,3, etc).
//
int
get_sensor_type_from_id( DATABASE * db, int sensor_id )
{
    int sens_type = SENSOR_TYPE_NONE;

    if( (sensor_id >= MIN_SENSOR_ID) && (sensor_id <= MAX_SENSOR_ID) )
        sens_type = db->sensor[ sensor_id ].setup.sensor_type;

    return( sens_type );
}


//
//  get_output_type_from_id() - Given a specific Output ID, this function
//                              determines what type of output has been
//                              discovered for that output ID.
//
int
get_output_type_from_id( DATABASE * db, int output_id )
{
    int output_type = OUTPUT_TYPE_NONE;

    if( (output_id >= MIN_OUTPUT_ID) && (output_id <= MAX_OUTPUT_ID) )
        output_type = db->output[ output_id ].setup.output_type;

    return( output_type );
}

//
//    valid_sensor_id() - This function checks if the sensor ID is
//                        valid. It is used by code that receives
//                        output setup data, and needs to determine
//                        if the sensor ID that the output references
//                        is valid.
//
//   In this sense, Sn-1, 2, and 3 are always "valid", although their
//   type could potentially be set to "None". Sensors Sn-d, HI-2, and
//   HI-3 are NOT always valid. They only become an acceptable choice
//   when their type is something other than "None". They are "virtual"
//   sensors.
//                    
//   Proper use of this routine can prevent output from referencing
//   Sn-d, HI-2, or HI-3 when those virtual sensors are not "valid".
//     
//
bool
valid_sensor_id( DATABASE * db, int sensor_id )
{
    bool reply;

    switch( sensor_id )
    {
        case SENSOR_ID_NONE:
        case SENSOR_ID_ONE:
        case SENSOR_ID_TWO:
        case SENSOR_ID_THREE:
            reply = TRUE;
        break;

        case SENSOR_ID_DIFF:
        case SENSOR_ID_HIGH_SIGNAL_2:
        case SENSOR_ID_HIGH_SIGNAL_3:
            if( db->sensor[sensor_id].setup.sensor_type == SENSOR_TYPE_NONE )
                reply = FALSE;
            else
                reply = TRUE;
        break;

        default:
            reply = FALSE;
        break;
    }

    return( reply );
}


//
//    valid_sensor_setup() - This function checks if the content of the
//                           sensor setup struct, passed as a parameter,
//                           contains valid data. It replies with
//                           a True / False value to indicate if the
//                           setup data is valid.
//
//                           The error_msg parameter will be loaded with
//                           an error message if one is found, and the
//                           pointer is not NULL.
//
bool  valid_sensor_setup( SENSOR_SETUP * sens, char * error_msg )
{
    char  str[100];
    bool  reply = TRUE;

    if( error_msg != NULL )   // Start with an empty error message string.
        error_msg[0] = 0;     //   Errors will be added as necessary.

    // Check for valid Sensor Type
    if( (sens->sensor_type < MIN_SENSOR_TYPE) || (sens->sensor_type > MAX_SENSOR_TYPE) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The Sensor Type must be in the range of %s to %s.<br/>\n", MIN_SENSOR_TYPE, MAX_SENSOR_TYPE );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for valid Sensor Offset (temp sensors only)
    if( (sens->sensor_type == SENSOR_TYPE_TEMP_F) || (sens->sensor_type == SENSOR_TYPE_TEMP_HI_F) )
    {
        if( (sens->offset < MIN_DEG_F_OFFSET) || (sens->offset > MAX_DEG_F_OFFSET) )
        {
            reply = FALSE;
            if( error_msg != NULL )
            {
                 sprintf( str, "The Offset must be in the range of %d to %d.<br/>\n", MIN_DEG_F_OFFSET, MAX_DEG_F_OFFSET );
                 strcat( error_msg, ERROR_MESSAGE_PROMPT );
                 strcat( error_msg, str );
            }
        }
    }
    else if( (sens->sensor_type == SENSOR_TYPE_TEMP_C) || (sens->sensor_type == SENSOR_TYPE_TEMP_HI_C) )
    {
        if( (sens->offset < MIN_DEG_C_OFFSET) || (sens->offset > MAX_DEG_C_OFFSET) )
        {
            reply = FALSE;
            if( error_msg != NULL )
            {
                 sprintf( str, "The Offset must be in the range of -%d.%d to %d.%d.<br/>\n", 
                               (-MIN_DEG_C_OFFSET) / 10, (-MIN_DEG_C_OFFSET) % 10, 
                               MAX_DEG_C_OFFSET / 10, MAX_DEG_C_OFFSET % 10);

                 strcat( error_msg, ERROR_MESSAGE_PROMPT );
                 strcat( error_msg, str );
            }
        }
    }
    else  // Else offset = 0 for all sensor types other than F, C, HI-F, HI-C
    {
        sens->offset = 0;
    }

    return( reply );
}


//
//    valid_relay_setup() - This function checks if the content of the
//                          relay setup struct, passed as a parameter,
//                          contains valid data. It replies with
//                          a True / False value to indicate if the
//                          setup data is valid.
//
//                          The error_msg parameter will be loaded with
//                          an error message if one is found, and the
//                          pointer is not NULL.
//
bool  valid_relay_setup( DATABASE * db, RELAY_SETUP * relay, int sens_type, char * error_msg )
{
    char  str[100], min_str[20], max_str[20], min_diff_str[20];
    int   min_sp, max_sp, min_diff, diff;
    bool  reply = TRUE;

    if( error_msg != NULL )   // Start with an empty error message string.
        error_msg[0] = 0;     //   Errors will be added as necessary.

    // Check for a valid reference Sensor ID
    if( !valid_sensor_id( db, relay->sensor_id ) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, "Invalid sensor referenced.<br/>\n" );
        }
    }

    // Look up the min/max setpoint values. These are a function
    //    of the sensor type that is referenced, and whether
    //    or not the sensor is a differential sensor.
    //
    if( relay->sensor_id == SENSOR_ID_DIFF )
    {
        min_sp = get_minimum_diff_setpoint( sens_type );
        max_sp = get_maximum_diff_setpoint( sens_type );
    }
    else
    {
        min_sp = get_minimum_setpoint( sens_type );
        max_sp = get_maximum_setpoint( sens_type );
    }

    min_diff = get_minimum_differential( sens_type );

#ifdef ENET_HARDWARE
    build_setpoint_string( min_sp,   sens_type, min_str );
    build_setpoint_string( max_sp,   sens_type, max_str );
    build_setpoint_string( min_diff, sens_type, min_diff_str );
#endif

    // The Binary sensor is a special case. There are no On\Off points
    //    used with binary sensors, therefore, we do not check the
    //    range or differential between these parameters if the output
    //    references a binary sensor.
    //
    if( sens_type != SENSOR_TYPE_BINARY )
    {
        // Check for valid ON sepoint
        if( (relay->cut_on < min_sp) || (relay->cut_on > max_sp) )
        {
            reply = FALSE;
            if( error_msg != NULL )
            {
                sprintf( str, "The ON point must be in the range of %s to %s.<br/>\n", min_str, max_str );
                strcat( error_msg, ERROR_MESSAGE_PROMPT );
                strcat( error_msg, str );
            }
        }

        // Check for valid OFF sepoint
        if( (relay->cut_off < min_sp) || (relay->cut_off > max_sp) )
        {
             reply = FALSE;
            if( error_msg != NULL )
            {
                 sprintf( str, "The OFF point must be in the range of %s to %s.<br/>\n", min_str, max_str );
                 strcat( error_msg, ERROR_MESSAGE_PROMPT );
                 strcat( error_msg, str );
            }
        }

        // Check for valid Minimum Difference; ON <-> OFF
        diff = relay->cut_on - relay->cut_off;
        if( diff < 0 )                       // Get the absolute value.
            diff = 0 - diff;

        if( diff < min_diff )
        {
            reply = FALSE;
            if( error_msg != NULL )
            {
                sprintf( str, "A minimum difference between ON and OFF of %s must be maintained.<br/>\n", min_diff_str );
                strcat( error_msg, ERROR_MESSAGE_PROMPT );
                strcat( error_msg, str );
            }
        }
    }

    // Check for a valid On Delay
    if( (relay->on_delay < MIN_ON_DELAY) || (relay->on_delay > MAX_ON_DELAY) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The ON Delay must be in the range of %d to %d seconds.<br/>\n", MIN_ON_DELAY, MAX_ON_DELAY );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Off Delay
    if( (relay->off_delay < MIN_OFF_DELAY) || (relay->off_delay > MAX_OFF_DELAY) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The OFF Delay must be in the range of %d to %d seconds.<br/>\n", MIN_OFF_DELAY, MAX_OFF_DELAY );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Minimum On Time
    if( (relay->min_on_time < MIN_ON_TIME) || (relay->min_on_time > MAX_ON_TIME) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The Minimum ON Time must be in the range of %d to %d seconds.<br/>\n", MIN_ON_TIME, MAX_ON_TIME );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Minimum Off Time
    if( (relay->min_off_time < MIN_OFF_TIME) || (relay->min_off_time > MAX_OFF_TIME) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The Minimum OFF Time must be in the range of %d to %d seconds.<br/>\n", MIN_OFF_TIME, MAX_OFF_TIME );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // The Binary sensor is a special case. There is no Sensor Fail Mode
    //     with binary sensors, therefore, we do not check the range
    //    of this parameter when the output references a binary sensor.
    //
    if( sens_type != SENSOR_TYPE_BINARY )
    {
        // Check for a valid Sensor Fail Mode
        if( (relay->sensor_fail_mode != SENSOR_FAIL_ON) && (relay->sensor_fail_mode != SENSOR_FAIL_OFF) )
        {
            reply = FALSE;
            if( error_msg != NULL )
            {
                strcpy( str, "The SNF mode must be either Fail ON or Fail OFF.<br/>\n" );
                strcat( error_msg, ERROR_MESSAGE_PROMPT );
                strcat( error_msg, str );
            }
        }
    }

    return( reply );
}

//
//   valid_analog_setup() - This function checks if the content of the
//                          analog setup struct, passed as a parameter,
//                          contains valid data. It replies with
//                          a True / False value to indicate if the
//                          setup data is valid.
//
//                          The error_msg parameter will be loaded with
//                          an error message if one is found, and the
//                          pointer is not NULL.
//
bool  valid_analog_setup( DATABASE * db, ANALOG_SETUP * analog, int sens_type, char * error_msg )
{
    char  str[100], min_str[20], max_str[20], min_diff_str[20];
    int   min_sp, max_sp, min_diff, diff;
    bool  reply = TRUE;

    if( error_msg != NULL )   // Start with an empty error message string.
        error_msg[0] = 0;     //   Errors will be added as necessary.

    // Check for a valid reference Sensor ID
    if( !valid_sensor_id( db, analog->sensor_id ) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, "Invalid sensor referenced.<br/>\n" );
        }
    }

    // Look up the min/max setpoint values. These are a function
    //    of the sensor type that is referenced, and whether
    //    or not the sensor is a differential sensor.
    //
    if( analog->sensor_id == SENSOR_ID_DIFF )
    {
        min_sp = get_minimum_diff_setpoint( sens_type );
        max_sp = get_maximum_diff_setpoint( sens_type );
    }
    else
    {
        min_sp = get_minimum_setpoint( sens_type );
        max_sp = get_maximum_setpoint( sens_type );
    }

    min_diff = get_minimum_differential( sens_type );

#ifdef ENET_HARDWARE
    build_setpoint_string( min_sp,   sens_type, min_str );
    build_setpoint_string( max_sp,   sens_type, max_str );
    build_setpoint_string( min_diff, sens_type, min_diff_str );
#endif
    // Check for valid SP sepoint
    if( (analog->sp < min_sp) || (analog->sp > max_sp) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The SP value must be in the range of %s to %s.<br/>\n", min_str, max_str );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for valid EP sepoint
    if( (analog->ep < min_sp) || (analog->ep > max_sp) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The EP value must be in the range of %s to %s.<br/>\n", min_str, max_str );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for valid Minimum Difference; SP <-> EP
    diff = analog->sp - analog->ep;
    if( diff < 0 )                       // Get the absolute value.
        diff = 0 - diff;

    if( diff < min_diff )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "A minimum difference between SP and EP of %s must be maintained.<br/>\n", min_diff_str );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a Output at Setpoint
    if( (analog->sp_output < MIN_SP_OUTPUT) || (analog->sp_output > MAX_SP_OUTPUT) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The OSP value must be in the range of %d to %d.<br/>\n", MIN_SP_OUTPUT, MAX_SP_OUTPUT );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Minimum On Time
    if( ((int)analog->ep_output < MIN_EP_OUTPUT) || (analog->ep_output > MAX_EP_OUTPUT) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The OEP value must be in the range of %d to %d.<br/>\n", MIN_EP_OUTPUT, MAX_EP_OUTPUT );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Integration Constant
    if( ((int)analog->int_constant < MIN_I_TERM) || (analog->int_constant > MAX_I_TERM) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The I-C value must be in the range of %d to %d.<br/>\n", MIN_OFF_TIME, MAX_OFF_TIME );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Update Rate
    if( ((int)analog->update_rate < MIN_UPDATE_RATE) || (analog->update_rate > MAX_UPDATE_RATE) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The UP-R value must be in the range of %d to %d.<br/>\n", MIN_UPDATE_RATE, MAX_UPDATE_RATE );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Control Band
    if( ((int)analog->output_band < MIN_OUTPUT_BAND) || (analog->output_band > MAX_OUTPUT_BAND) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            sprintf( str, "The bND value must be in the range of %d to %d.<br/>\n", MIN_OUTPUT_BAND, MAX_OUTPUT_BAND );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    // Check for a valid Sensor Fail Mode
    if( (analog->sensor_fail_mode != SENSOR_FAIL_ON) && (analog->sensor_fail_mode != SENSOR_FAIL_OFF) )
    {
        reply = FALSE;
        if( error_msg != NULL )
        {
            strcpy( str, "The SNF mode must be either Fail ON or Fail OFF.<br/>\n" );
            strcat( error_msg, ERROR_MESSAGE_PROMPT );
            strcat( error_msg, str );
        }
    }

    return( reply );
}


//
//    read_ethernet_reset_button()
//
bool
read_ethernet_reset_button( void )
{
    GPIO_MemMapPtr  ptrC;
    uint32_t        read_port_c;
    bool            pressed;

    ptrC = PTC_BASE_PTR;

    // Read the 32 bit Port B Data Input register.
    //
    read_port_c = ptrC->PDIR;

    // When Port C, bit 12 is LOW, the button is pressed.
    //
    pressed = ((read_port_c & 0x00001000) == 0) ? TRUE : FALSE;

    return( pressed );
}


//
//  load_default_setup() - This routine loads an Output Setup structure
//                         with default values.
//                     
//                         The default values that are loaded are a
//                         function of the sensor type associated with
//                         the output setup. Different sensors support
//                         different ranges of setpoints, and have
//                         different default values. 
//
// Global Vars Affected:  One of the following global arrays will be
//                        referenced as a source for the default values.
//
//                        DefaultRelaySetup[]  
//                        DefaultAnalogSetup[]
//
//           Parameters:  Output *  - location where the parameter
//                                   and related items will be loaded.
//
//                     sensor_type - indicates the type of sensor
//                                   to use when selecting the
//                                   default setup values. 
//
//              Returns:  None
//
void          
load_default_setup( OUTPUT * output, uint8_t sensor_type )
{      
    if( sensor_type > MAX_SENSOR_TYPE )   // Ensure that the sensor_type
        sensor_type = SENSOR_TYPE_NONE;   //   is in a valid range.
        
    if( output->setup.output_type == OUTPUT_TYPE_RELAY )
        output->setup.output.relay  = DefaultRelaySetup[ sensor_type ];
    else
        output->setup.output.analog = DefaultAnalogSetup[ sensor_type ];
}


//
//  load_default_diff_setup() - This routine loads an Output Setup structure
//                              with default values.
//                     
//                         The default values that are loaded are a
//                         function of the sensor type associated with
//                         the output setup. Different sensors support
//                         different ranges of setpoints, and have
//                         different default values. 
//
// Global Vars Affected:  One of the following global arrays will be
//                        referenced as a source for the default values.
//                        THESE VALUES ARE UNIQUE TO "Differential" SENSORS.
//
//                        DefaultDiffRelaySetup[]  
//                        DefaultDiffAnalogSetup[]
//
//           Parameters:  Output *  - location where the parameter
//                                   and related items will be loaded.
//
//                     sensor_type - indicates the type of sensor
//                                   to use when selecting the
//                                   default setup values. 
//
//              Returns:  None
//
void          
load_default_diff_setup( OUTPUT * output, unsigned char sensor_type )
{      
    if( sensor_type > MAX_SENSOR_TYPE )   // Ensure that the sensor_type
        sensor_type = SENSOR_TYPE_NONE;   //   is in a valid range.
        
    if( output->setup.output_type == OUTPUT_TYPE_RELAY )
        output->setup.output.relay  = DefaultDiffRelaySetup[ sensor_type ];
    else
        output->setup.output.analog = DefaultDiffAnalogSetup[ sensor_type ];
}


//
//  get_minimum_setpoint() - The minimum setpoint is the lowest value
//                           that a user is permitted to enter as a 
//                           setpoint for control. 
//
//                           This routine determines the minimum setpoint
//                           and returns it to the calling function.
//                           That determination is dependent on the 
//                           type of sensor being used.
//
// Global Vars Affected:  MinimumSetpoint[] - Referenced to load
//                                            the minimum setpoint.
//
//           Parameters:  sensor_type - indicates the sensor type.
//
//              Returns:  Minimum Setpoint
//
int
get_minimum_setpoint( unsigned char sensor_type )
{
    return( MinimumSetpoint[ sensor_type ] );
}


//
//  get_maximum_setpoint() - The maximum setpoint is the highest value
//                           that a user is permitted to enter as a 
//                           setpoint for control. 
//
//                           This routine determines the maximum setpoint
//                           and returns it to the calling function.
//                           That determination is dependent on the 
//                           type of sensor being used.
//
// Global Vars Affected:  MinimumSetpoint[] - Referenced to load
//                                            the maximum setpoint.
//
//           Parameters:  sensor_type - indicates the sensor type.
//
//              Returns:  Maximum Setpoint
//
int
get_maximum_setpoint( unsigned char sensor_type )
{
    return( MaximumSetpoint[ sensor_type ] );
}


//
//  get_minimum_differential() - The minimum differential sets the
//                               minimum difference that is permitted
//                               between Cut-On \ Cut-Off (Relay control)
//                               or between SP \ EP (Analog control)
//
//                               Using the minimum differential, users
//                               will be forced to maintain a span
//                               between these values when entering
//                               setpoints. The differential is dependent
//                               upon the type of sensor being used.
//
// Global Vars Affected:  MinimumDifferential[] - Referenced to load
//                                                the min. differential.
//
//           Parameters:  sensor_type - indicates the sensor type.
//
//              Returns:  Minimum Differential
//
int            
get_minimum_differential( unsigned char sensor_type )
{
    return( MinimumDifferential[ sensor_type ] );
}


//
//  get_minimum_diff_setpoint() - The minimum differential setpoint is the
//                                lowest value that a user is permitted to
//                                enter as a setpoint for differential control. 
//
//                           This routine determines the minimum differential 
//                           setpoint and returns it to the calling function.
//                           That determination is dependent on the type
//                           of sensor being used.
//
// Global Vars Affected:  MinDiffSetpoint[] - Referenced to load
//                                            the minimum setpoint.
//
//           Parameters:  sensor_type - indicates the sensor type.
//
//              Returns:  Minimum Differential Setpoint
//
int
get_minimum_diff_setpoint( unsigned char sensor_type )
{
    return( MinDiffSetpoint[ sensor_type ] );
}


//
//  get_maximum_diff_setpoint() - The maximum differential setpoint is the
//                                largest value that a user is permitted to
//                                enter as a setpoint for differential control. 
//
//                           This routine determines the maximum differential 
//                           setpoint and returns it to the calling function.
//                           That determination is dependent on the type
//                           of sensor being used.
//
// Global Vars Affected:  MinDiffSetpoint[] - Referenced to calculate
//                                            the maximum setpoint.
//
//           Parameters:  sensor_type - indicates the sensor type.
//
//              Returns:  Maximum Differential Setpoint
//
int
get_maximum_diff_setpoint( unsigned char sensor_type )
{
    return( (0 - MinDiffSetpoint[ sensor_type ]) );
}


//
//  get_sensor_increment() - The sensor increment determines how much
//                           a setpoint should change as the user edits
//                           its value with the Up\Down keys. The
//                           increment varies by sensor type.
//
//                           Setpoints are stored as integers. For a 
//                           given sensor type, an increment value
//                           of 5 could relate to +/- 0.5 deg C or
//                           +/- 0.005 INWC
//
// Global Vars Affected:  None
//
//           Parameters:  sensor_type - indicates the sensor type.
//
//              Returns:  Increment
//
int
get_sensor_increment( unsigned char sensor_type )
{
    int  reply;
    
    switch( sensor_type )
    {
        case SENSOR_TYPE_NONE:    
            reply = 0;           
        break;
        
        case SENSOR_TYPE_TEMP_F:    // +/- 1    deg F    
        case SENSOR_TYPE_TEMP_HI_F: // +/- 1    deg F    
        case SENSOR_TYPE_RH:        // +/- 1    %rH
        case SENSOR_TYPE_P_15:      // +/- 0.1  BAR
        case SENSOR_TYPE_P_30:      // +/- 0.1  BAR
        case SENSOR_TYPE_P200:      // +/- 1    PSI
        case SENSOR_TYPE_P500:      // +/- 1    PSI
        case SENSOR_TYPE_BINARY:    // +/- 1    no units
            reply = 1;
        break;        
        
        case SENSOR_TYPE_TEMP_C:    // +/- 0.5   deg C
        case SENSOR_TYPE_TEMP_HI_C: // +/- 0.5   deg C
        case SENSOR_TYPE_P_0pt25:   // +/- 0.005 INWC
        case SENSOR_TYPE_P_0pt5:    // +/- 0.005 INWC
        case SENSOR_TYPE_P_5:       // +/- 0.05  INWC
        case SENSOR_TYPE_P__8:      // +/- 0.05  BAR
        case SENSOR_TYPE_P_10:      // +/- 0.05  INWC
        case SENSOR_TYPE_P100:      // +/- 0.5   PSI
        case SENSOR_TYPE_P110:      // +/- 0.5   PSI
            reply = 5;   
        break;

        case SENSOR_TYPE_P_2pt5:    // +/- 0.02  INWC
        case SENSOR_TYPE_P_50:      // +/- 0.2   BAR
        case SENSOR_TYPE_P750:      // +/- 2     PSI
            reply = 2;
        break;

        default:
            reply = 0;
        break;
    }

    return( reply );
}


//
//  get_sensor_decimal_pt() - Both sensor and setpoint data is
//                            typically stored as an integer. In
//                            cases where the real value is less than
//                            1.0, the integer data is presented
//                            as a real value by the placement of 
//                            a decimal point. 
//
//                            The decimal point position is a function
//                            of the type of sensor being used.
//
// Global Vars Affected:  None
//
//           Parameters:  sensor_type - indicates the sensor type.
//
//              Returns:  Decimal Pt Position - 0 = No decimal point
//                                              1 = 000.0
//                                              2 = 00.00
//                                              3 = 0.000
//
unsigned char
get_sensor_decimal_pt( unsigned char sensor_type )
{
    unsigned char reply;
    
    switch( sensor_type )
    {      
        case SENSOR_TYPE_TEMP_F:
        case SENSOR_TYPE_TEMP_HI_F:
        case SENSOR_TYPE_RH:
        case SENSOR_TYPE_P200:
        case SENSOR_TYPE_P500:
        case SENSOR_TYPE_P750:
        case SENSOR_TYPE_BINARY:
            reply = 0;
        break;

        case SENSOR_TYPE_TEMP_C:
        case SENSOR_TYPE_TEMP_HI_C:
        case SENSOR_TYPE_P_15:
        case SENSOR_TYPE_P_30:
        case SENSOR_TYPE_P_50:
        case SENSOR_TYPE_P100:
        case SENSOR_TYPE_P110:
            reply = 3;   
        break;

        case SENSOR_TYPE_P_2pt5:
        case SENSOR_TYPE_P_5:
        case SENSOR_TYPE_P__8:
        case SENSOR_TYPE_P_10:
            reply = 2;   
        break;

        case SENSOR_TYPE_P_0pt25:    
        case SENSOR_TYPE_P_0pt5:    
            reply = 1;   
        break;

        default:
            reply = 0;
        break;
    }

    return( reply );
}


//
//  differential_sensor_used() - Determine if any of the outputs are
//                               using the differential sensor (Sn1 - Sn2)
//
// Global Vars Affected:  The global output setup data is referenced.
//
//        Parameters:  None.
//
//           Returns:  TRUE  - if any output references the differential sensor
//                     FALSE - No outputs references differential sensor
//
unsigned char
differential_sensor_used(  OUTPUT * output, int num_outputs )
{
    int k;

    for( k=0; k<num_outputs; k++ )
        if( output[k].setup.output.relay.sensor_id == SENSOR_ID_DIFF )
	    return( TRUE );     
	
    return( FALSE );
}


//
//  delay_msec() - This function results in a delay in milliseconds.
//
// Global Vars Affected:  None.
//
//           Parameters:  delay_msec  -  The amount of time to delay (mSec)
//
//              Returns:  None
//
//  Notes: This function uses the MQX Function and Data Structures to 
//         implement the delay. One of the ways that MQX expresses time
//         is in "Ticks" and "Hardware Ticks". A single hardware tick
//         is a singe CPU cycle. If the CPU is running at 100MHz, a
//         hardware tick is 10 nanoseconds.
//
//         A tick is a number of hardware ticks, and it is established
//         in the MQX library value. A common practice results in 
//         there being 100 Ticks per Hardware Tick.
//
//         Refer to the MQX Reference manual for further details.
//
void delay_msec( uint8_t delay_msec )
{
    uint32_t         hw_tick_delay, hw_tick_diff, hw_tick_per_tick;
    MQX_TICK_STRUCT  start_tick, now_tick, diff_tick;

    hw_tick_per_tick  = _time_get_hwticks_per_tick();

    // Calculate the delay, expressed in hardware ticks.
    //  Steps:
    //
    //    1) Get the number of hardware ticks / tick, and divide that
    //       number by 1000. This step assumes that the number
    //       of hardware ticks / tick is greater than 1000. In the case
    //       of the Ethernet and RS485 versions of the C450, the number
    //       of hw ticks / tick is 1,000,000 and 480,000 respectively.
    //
    //    2) Multiply the value from step 1 by the number of ticks 
    //       per second. Since the value in step 1 was already divided
    //       by 1000, this step results in the number of hardware ticks
    //       per millisecond.
    //
    //    3) Multiply the value from step 2 by the number of milliseconds
    //       to delay. Note that the parameter passed to this function
    //       is of type "uint_8", ensuring that the delay is a maximum
    //       of 255 milliseconds.
    //

    // Step 1
    hw_tick_delay  = hw_tick_per_tick / 1000;

    // Step 2
    hw_tick_delay *= _time_get_ticks_per_sec();

    // Step 3
    hw_tick_delay  = hw_tick_delay * (uint32_t)delay_msec;

    // Get the start time. The data type MQX_TICK_STRUCT will be loaded
    //   with a value representing the time in both TICKS and HWTICKS
    _time_get_ticks( &start_tick );

    // Routine call "_time_get_ticks()" to get the current time. Then
    //   call "_time_diff_ticks()" to get the time difference between the
    //   start and current times. Convert the time difference in the
    //   the "hw_tick_diff" structure to a value expressed solely 
    //   in HWTICKS. That value is used to determine when the delay time
    //   has transpired.
    do
    {
        _time_get_ticks( &now_tick );

        _time_diff_ticks( &now_tick, &start_tick, &diff_tick );

        hw_tick_diff  = diff_tick.TICKS[0] * hw_tick_per_tick;
        hw_tick_diff += diff_tick.HW_TICKS;

    }  while( hw_tick_diff < hw_tick_delay );
}


//
//  delay_usec() - This function results in a delay in microseconds.
//
// Global Vars Affected:  None.
//
//           Parameters:  delay_usec  -  The amount of time to delay (uSec)
//
//              Returns:  None
//
//  Notes: This function uses the MQX Function and Data Structures to 
//         implement the delay. One of the ways that MQX expresses time
//         is in "Ticks" and "Hardware Ticks". A single hardware tick
//         is a singe CPU cycle. If the CPU is running at 100MHz, a
//         hardware tick is 10 nanoseconds.
//
//         A tick is a number of hardware ticks, and it is established
//         in the MQX library value. A common practice results in 
//         there being 100 Ticks per Hardware Tick.
//
//         Refer to the MQX Reference manual for further details.
//
void delay_usec( uint16_t delay_usec )
{
    uint32_t         hw_tick_delay, hw_tick_diff, hw_tick_per_tick;
    MQX_TICK_STRUCT  start_tick, now_tick, diff_tick;

    hw_tick_per_tick = _time_get_hwticks_per_tick();

    // Calculate the delay, expressed in hardware ticks.
    //  Steps:
    //
    //    1) Get the number of hardware ticks / tick, and divide that
    //       number by 1000. This step assumes that the number
    //       of hardware ticks / tick is greater than 1000. In the case
    //       of the Ethernet and RS485 versions of the C450, the number
    //       of hw ticks / tick is 1,000,000 and 480,000 respectively.
    //
    //    2) Multiply the value from step 1 by the number of ticks 
    //       per second. Since the value in step 1 was already divided
    //       by 1000, this step results in the number of hardware ticks
    //       per millisecond.
    //
    //    3) Multiply the value from step 2 by the number of microseconds
    //       to delay. Note that the parameter passed to this function
    //       is of type "uint_16", ensuring that the delay is a maximum
    //       of 65,535 microseconds.
    //
    //    4) Multiply the value from step 3 by 1000, converting the
    //       value from milliseconds to microseconds.
    //

    // Step 1
    hw_tick_delay  = hw_tick_per_tick / 1000;

    // Step 2
    hw_tick_delay *= _time_get_ticks_per_sec();

    // Step 3
    hw_tick_delay  = hw_tick_delay * (uint32_t)delay_usec;

    // Step 4
    hw_tick_delay  = hw_tick_delay / 1000;

    // Get the start time. The data type MQX_TICK_STRUCT will be loaded
    //   with a value representing the time in both TICKS and HWTICKS
    _time_get_ticks( &start_tick );

    // Routine call "_time_get_ticks()" to get the current time. Then
    //   call "_time_diff_ticks()" to get the time difference between the
    //   start and current times. Convert the time difference in the
    //   the "hw_tick_diff" structure to a value expressed solely 
    //   in HWTICKS. That value is used to determine when the delay time
    //   has transpired.
    do
    {
        _time_get_ticks( &now_tick );

        _time_diff_ticks( &now_tick, &start_tick, &diff_tick );

        hw_tick_diff  = diff_tick.TICKS[0] * hw_tick_per_tick;
        hw_tick_diff += diff_tick.HW_TICKS;

    }  while( hw_tick_diff < hw_tick_delay );
}


//
//  calc_i2c_crc () - Calculates an 8-bit CRC. This particular CRC is used
//                    in I2C messages between modules. It is an addition
//                    to (not part of) the I2C spec.
//
//  Written By - G Hindmon
//
// Global Vars Affected:  CRC_8_Table[] - References this table to
//                                        speed up calculation of the CRC.
//
//           Parameters:  data - Pointer to data containing the source
//                               data for the CRC calc.
//
//                        len  - Size of the buffer pointed to by "data".
//
//              Returns:  crc  - 8-bit CRC calculation of the data buffer.
//
uint8_t calc_i2c_crc( uint8_t * data, uint8_t len )
{
    uint8_t crc;

    crc = CRC_8_SEED;              // Initialize CRC value

    do
    {
        crc ^= *data;              // Xor next byte from buffer with CRC
        crc  = CRC_8_Table[crc];   // Get CRC for new value from CRC table
        data++;                    // Increment the buffer pointer
    }
    while( --len > 0 );            // See if we are finished

    return( crc );                 // Return CRC value
}
