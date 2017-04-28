/***************************************************************************
(C)Copyright Johnson Controls, Inc. 2012. Use or copying of all or any 
part of the document, except as permitted by the License Agreement, is
prohibited.

FILENAME           : web_func.h
                   : 
PURPOSE            : Function prototypes for web server pages.                  
                   :
INPUTS             : 
                   : 
                   : 
OUTPUTS            : 
                   :
CALLING ROUTINES   : 
                   : 
                   : 
UNIQUE  FEATURES   : 

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __web_func_inc
#define  __web_func_inc

#include "defines.h"


#define  UID_RANDOM_LENGTH  30   // Length of the random ID string used to
                                 //   identify users who have logged in.

#define  LOGIN_TIMEOUT     900   // The login timeout is set to 15 minutes
                                 //   (900 seconds). If a user accesses a
                                 //    protected page, and they do not
                                 //   navigate to another protected page,
                                 //   their access times out after 10 
                                 //   minutes and they will need to login
                                 //   again to access secure pages.

#define MAX_LOGGED_IN        4   // Maximum number of users logged in at
                                 //   any given time.

//
//   UID_LOGIN - This data structure is used to hold the User ID string,
//               IP address, and login time, that identifies a user of the 
//               created by this web server as having been authenticated. 
//               User IDs are application following a successful login, and 
//               track users while they navigate the web site. 
//
typedef struct
{
    char     str[ UID_RANDOM_LENGTH+1 ];  // random string of chars, 'a'-'z'
    uint32_t time;                        // time in seconds
    uint32_t ip;                          // IP addr

}    UID_LOGIN;

extern UID_LOGIN uidDatabase[ MAX_LOGGED_IN ];

//
//    Function Prototypes
//
void     restart_firmware( void );

bool     add_uid_to_database( uint32_t seconds, uint32_t ip, UID_LOGIN * uid );
bool     remove_uid_from_database( char * uid_str );
void     create_new_uid( uint32_t seconds, uint32_t ip, UID_LOGIN * uid );
void     update_uid_time( uint32_t seconds, UID_LOGIN * uid );
bool     uid_login_timeout( uint32_t seconds, UID_LOGIN * uid );
bool     valid_uid( UID_LOGIN * uid );
void     safe_copy_uid_string( UID_LOGIN * uid, char *src_str );

void     load_default_enet_setup( ENET_SETUP * enet );
void     web_sensor_string( SENSOR * sens, int sensor_id, char * str );
void     web_build_float_string( char * str, float value, int num_decimal );
void     web_output_status_string( OUTPUT * out, char * str );
void     build_setpoint_string( int setpoint, int sensor_type, char * str );
void     build_offset_string( int setpoint, int sensor_type, char * str );
void     build_setpoint_range_string( int sensor_id, int sensor_type, char * str  );
void     build_sensor_data_string( int value, int dec_pt, char * str );
void     build_device_revision_string( uint32_t sim_sdid, char * str );
void     build_device_family_id_string( uint32_t sim_sdid, char * str );
void     build_device_pin_count_string( uint32_t sim_sdid, char * str );
void     web_blink_comm_leds( void );

#endif