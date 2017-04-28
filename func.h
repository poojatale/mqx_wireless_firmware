/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : func.h

PURPOSE   : Function prototypes for "func.c" module.                  

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __func_inc
#define  __func_inc

#include "defines.h"

//
//             Function Prototypes
//

bool           read_ethernet_reset_button( void );

void           update_sensor_pointers( DATABASE *db );
int            get_sensor_type_from_id( DATABASE * db, int sensor_id );
int            get_output_type_from_id( DATABASE * db, int output_id );

bool           valid_sensor_setup( SENSOR_SETUP * sens, char * error_msg );
bool           valid_sensor_id( DATABASE * db, int sensor_id );
bool           valid_relay_setup( DATABASE * db, RELAY_SETUP * relay, int sens_type, char * error_msg );
bool           valid_analog_setup( DATABASE * db, ANALOG_SETUP * analog, int sens_type, char * error_msg );

void           load_default_setup( OUTPUT * output, uint8_t sensor_type );
void           load_default_diff_setup( OUTPUT * output, unsigned char sensor_type );

int            get_minimum_setpoint( unsigned char sensor_type );
int            get_maximum_setpoint( unsigned char sensor_type );
int            get_minimum_differential( unsigned char sensor_type );
int            get_minimum_diff_setpoint( unsigned char sensor_type );
int            get_maximum_diff_setpoint( unsigned char sensor_type );
int            get_sensor_increment( unsigned char sensor_type );
unsigned char  get_sensor_decimal_pt( unsigned char sensor_type );
unsigned char  differential_sensor_used(  OUTPUT * output, int num_outputs );

void           delay_msec( uint8_t  delay_msec );
void           delay_usec( uint16_t delay_usec );

uint8_t        calc_i2c_crc( uint8_t * data, uint8_t len );

#endif