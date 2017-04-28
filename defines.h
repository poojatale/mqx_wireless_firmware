/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : defines.h

PURPOSE   : This header file contains definitions that are used by this                  
            application. The content of this file is referenced by multiple 
            modules. It serves as a central location for the definitions.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __defines_inc
#define  __defines_inc

#include <mqx.h>
#include <bsp.h>
#include <psptypes.h>  // Platform specific types. This file is used with
                       //    the build of MQX. It is used here to define
                       //    types such as; int16_t, uint32, uchar, etc.
#include <lwevent.h>
#include "app_defines.h"
#include "system450.h"


// Task numbers for each Task running under MQX
#define INIT_TASK        1
#define SENSOR_TASK      2
#define CONTROL_TASK     3
#define UI_TASK          4 
#define WATCHDOG_TASK    5
#define MODBUS_TASK      6
#define ENET_HELP_TASK   6
#define DDNS_TASK        7
#define UDP_TASK         8
#define WEB_SERVER_TASK  9

#define HVAC_TASK        10
#define SHELL_TASK       11
#define HEARTBEAT_TASK   12
#define WMICONFIG_TASK1  13
#define WMICONFIG_TASK2  14
#define BUTTON_TASK      15
#define RCONTROL_TASK    16




// Define TRUE\FALSE if not already defined by the compiler
//
#ifndef  TRUE
    #define  TRUE        1
#endif

#ifndef  FALSE
    #define  FALSE       0
#endif


#define I2C_FAIL_COUNT  30   // If the number of consecutive I2C messages to
                             //   a specific output reach this number, 
                             //   the I2C link to that output is presumed
                             //   to have failed.

// RELAY_ON and RELAY_OFF are used to specify the desired logical
//   state of a relay as part of the control algorithm. Local relays
//   are commanded by a value that is the inverse of this logical value.

#define RELAY_OFF        0          // Relay On
#define RELAY_ON         1          // Relay Off

                                    // Signal to reset the expansion modules.
#define RESET_MODULE_LEVEL      1   // 1 = Reset exp. modules, Rev 9 hw or later
#define NOT_RESET_MODULE_LEVEL  0   // 0 = Exp. modules Not in Reset

// The state of the transmit and receive LEDs on the terminal can be either;
//
//    1) Off
//    2) Fail - slow blink indicating receive failure
//    3) Success - fast blink indicating receive\transmit success
//    4) On
//
// These commands are sent by the control board to the terminal board for
// display. In addition, the override bit can be used to force the terminal
// board to immediately respond to the most recent command as opposed to 
// finishing any ongoing blink pattern.
//
#define COMM_LED_OFF       0x00
#define COMM_LED_FAIL      0x01
#define COMM_LED_SUCCESS   0x02
#define COMM_LED_ON        0x03
#define COMM_LED_OVERRIDE  0x08

//
//  PLATFORM_INFO - This data structure contains information about the
//                  firmware and microcontroller, including;
//
//            Firmware SSM Number : 16 bit unsigned value
//        Firmware Version Number : 16 bit unsigned value
//                       SIM_SDID : 32 bit unsigned value, 
//                                  System Device ID Register
//
//    The C450 communication module consists of two circuit boards, each 
//  with its own microcontroller and firmware. 
//
typedef struct
{
    uint16_t  ssm;          // SSM number of the firmware
    uint16_t  fw_version;   // Firmware version number
    uint32_t  sim_sdid;     // Copy of the SIM_SDID register in the 
                           //   microcontroller.
}    PLATFORM_INFO;


//
//  LED_REQUEST - This data structure contains instructions for driving the
//                transmit and receive LEDs on the C450 UI. The desired
//                state of these LEDs is determined by the Modbus Task.
//                The UI Task will reference this information and 
//                send the desired state to the terminal for display.
//
typedef struct
{
    bool      update_xmit_led;   // Indicates that a new state for the
                                //   xmit LED is requested.
    bool      update_recv_led;   // Indicates that a new state for the
                                //   xmit LED is requested.
    uint8_t   xmit_led;
    uint8_t   recv_led;

}    LED_REQUEST;


enum{  KEY_NONE,               // These values are used to identify
       KEY_UP,                 //    which, if any, of the four
       KEY_NEXT,               //    keys have been pressed.
       KEY_DOWN,               
       KEY_MENU,
       KEY_UP_x5,              // Special case, up key pressed and held in
       KEY_DOWN_x5,            // Special case, down key pressed and held in
       KEY_PASSWORD,           // Special case, up/dn keys, 5 sec = PASSWORD
       KEY_EXIT_MENU };        // Special case, up/dn keys = Exit Menu

#define  INACTIVE_TIME  240    // 240 = 120 Seconds, 2 minutes, the amount of 
                               //    time that must transpire between
                               //    keystroke before an "Inactive Timeout"
                               //    has occurred. This time period is used
                               //    to restore the display to the main status
                               //    screen if a user enters a setup screen 
                               //    and then walks away from the controller.
                               //  The value "240" is used because the code
                               //   that maintains the timer runs every 1/2 sec.
                             

enum{  OUTPUT_TYPE_NONE,       // Module type None means no module found
       OUTPUT_TYPE_RELAY,      // Relay module (Output), output is On/Off
       OUTPUT_TYPE_ANALOG  };  // Analog module (Output), output is variable


enum{ SENSOR_TYPE_NONE,        // Sensor type is UNCONFIGURED, unused
      SENSOR_TYPE_TEMP_F,      // A99 sensor,      -46 to 255,   units = degrees F
      SENSOR_TYPE_TEMP_C,      // A99 sensor,    -43.0 to 124.0, units = degrees C
      SENSOR_TYPE_RH,          // Rel. humidity,     1 to 100,   units = %RH
      SENSOR_TYPE_P_0pt5,      // Pressure,      0.000 to 0.500, units = INWC
      SENSOR_TYPE_P__8,        // Pressure,      -1.00 to 8.00,  units = bAR
      SENSOR_TYPE_P_10,        // Pressure,       0.00 to 10.00, units = INWC
      SENSOR_TYPE_P_15,        // Pressure,       -1.0 to 15.0,  units = bAR
      SENSOR_TYPE_P_30,        // Pressure,        0.0 to 30.0,  units = bAR
      SENSOR_TYPE_P_50,        // Pressure,        0.0 to 50.0,  units = bAR
      SENSOR_TYPE_P100,        // Pressure,        0.0 to 100.0, units = PSI
      SENSOR_TYPE_P500,        // Pressure,          0 to 500,   units = PSI
      SENSOR_TYPE_P750,        // Pressure,          0 to 750,   units = PSI
      SENSOR_TYPE_P200,        // Pressure,          0 to 200,   units = PSI
      SENSOR_TYPE_P_2pt5,      // Pressure,          0 to 2.5,   units = INWC
      SENSOR_TYPE_P_5,         // Pressure,          0 to 5.0,   units = INWC
      SENSOR_TYPE_TEMP_HI_F,   // Temperature,      70 to 330,   units = degrees F
      SENSOR_TYPE_TEMP_HI_C,   // Temperature,    21.0 to 165.0, units = degrees C
      SENSOR_TYPE_P110,        // Pressure,      -10.0 to 100.0, units = PSI
      SENSOR_TYPE_BINARY,      // Binary Input,  Open or Closed, units = n/a
      SENSOR_TYPE_P_0pt25  };  // Pressure,     -0.250 to 0.250, units = INWC

#define MIN_SENSOR_TYPE           SENSOR_TYPE_NONE
#define MAX_SENSOR_TYPE           SENSOR_TYPE_P_0pt25

#define SENSOR_TYPE_MIN_INDEX     0
#define SENSOR_TYPE_MAX_INDEX     MAX_SENSOR_TYPE

#define NUM_SENSOR_TYPES          MAX_SENSOR_TYPE + 1

#define BIN_SENSOR_OPEN           0
#define BIN_SENSOR_CLOSED         1


enum{  CTRL_MODE_1,             // Ramp =   |/   North America
       CTRL_MODE_2,             // Ramp =  \|    North America
       CTRL_MODE_3,             // Ramp =   |\   Europe
       CTRL_MODE_4   };         // Ramp =  /|    Europe

enum{  SENSOR_FAIL_ON,          // Turn output ON, upon sensor failure
       SENSOR_FAIL_OFF };       // Turn output OFF, upon sensor failure

                                 // The parameter IDs are used by the user
                                 //    interface code to identify what is 
                                 //    being displayed or edited by the UI.                               
enum{  PARAM_ID_NONE,            // No parameter selected
       PARAM_ID_SENSOR_STATUS,   // Sensor Status, Sensors 1-3
       PARAM_ID_OUTPUT_STATUS,   // Output Status, Outputs 1-10
       PARAM_ID_CUTON,           // Cut-On point - relay module
       PARAM_ID_CUTOFF,          // Cut-Off point - relay module
       PARAM_ID_ON_DELAY,        // On Delay - relay module
       PARAM_ID_OFF_DELAY,       // Off Delay - relay module
       PARAM_ID_MINON,           // Minimum On time - relay module
       PARAM_ID_MINOFF,          // Minimum Off time - relay module
       PARAM_ID_SEL_SENSOR,      // Sensor referenced by an Output
       PARAM_ID_CTRL_MODE,       // Control mode - analog module
       PARAM_ID_SP,              // Setpoint - analog module
       PARAM_ID_EP,              // End Point - analog module
       PARAM_ID_INT_CONSTANT,    // Integration constant - analog module
       PARAM_ID_OUTPUT_SP,       // Output @ SP - analog module
       PARAM_ID_OUTPUT_EP,       // Output @ EP - analog module
       PARAM_ID_UPDATE_RATE,     // Output Update Rate - analog module
       PARAM_ID_OUTPUT_BAND,     // Output Band (deadband) - analog module

       PARAM_ID_DIFF_CUTON,      // Differential Cut-On point - relay module
       PARAM_ID_DIFF_CUTOFF,     // Differential Cut-Off point - relay module
       PARAM_ID_DIFF_SP,         // Differential Setpoint - analog module
       PARAM_ID_DIFF_EP,         // Differential End Point - analog module
       PARAM_ID_SENSOR_FAILURE,  // Sensor failure action - fault on or off

       PARAM_ID_SENSOR_TYPE,     // Sensor Type (1 of 3) displayed
       PARAM_ID_SENSOR_OFFSET_1, // User offset, Sensor 1, deg F / C only
       PARAM_ID_SENSOR_OFFSET_2, // User offset, Sensor 2, deg F / C only
       PARAM_ID_SENSOR_OFFSET_3, // User offset, Sensor 3, deg F / C only

       PARAM_ID_MODBUS_ADDRESS,   // Modbus Node Address
       PARAM_ID_MODBUS_BAUD_RATE, // Modbus Baud Rate
       PARAM_ID_MODBUS_PARITY,    // Modbus Parity
       PARAM_ID_MODBUS_STOP_BITS, // 1 or 2 stop bits

       PARAM_ID_REMOTE_LOCKOUT, // Ethernet Remote Lockout, T/F (1/0)
       PARAM_ID_ENET_IP_1,      // Ethernet IP Address 1 (ie. IP1.255.255.255)
       PARAM_ID_ENET_IP_2,      // Ethernet IP Address 2 (ie. 255.IP2.255.255)
       PARAM_ID_ENET_IP_3,      // Ethernet IP Address 3 (ie. 255.255.IP3.255)
       PARAM_ID_ENET_IP_4,      // Ethernet IP Address 4 (ie. 255.255.255.IP4)
       PARAM_ID_ENET_ADDR_MODE, // Static IP, DHCP Client, or Direct Connect
       PARAM_ID_ENET_IP_RESET,  // Prompt to reset enet settings to default

       PARAM_ID_ENTER_USER_PW,   // Enter the User PW to advance
       PARAM_ID_EDIT_USER_PW,    // Edit the User PW (system setup)
       PARAM_ID_ENTER_ADMIN_PW,  // Enter the Admin PW to advance
       PARAM_ID_EDIT_ADMIN_PW_1, // Edit the Admin PW (system setup)
       PARAM_ID_EDIT_ADMIN_PW_2, // Edit the Admin PW (system setup)

       PARAM_ID_MESSAGE,         // Message, not an editable param

       PARAM_ID_MENU_PROMPT  };  // Not a "parameter", indicates a menu

                        
enum{  SENSOR_ID_NONE,          // This enumeration is used to identify which 
       SENSOR_ID_ONE,           //   sensor, if any, that is being referenced by
       SENSOR_ID_TWO,           //   an output (control loop). Valid settings 
       SENSOR_ID_THREE,         //   or the Differential sensor (Sn1 - Sn2)
       SENSOR_ID_DIFF,          //   include "None", one of the three sensors,
       SENSOR_ID_HIGH_SIGNAL_2, //   the "highest" signal between Sn-1 and Sn-2, or 
       SENSOR_ID_HIGH_SIGNAL_3  //   the "highest" signal between Sn-1, 2, and 3,
    };                          //   or the Differential sensor (Sn1 - Sn2)

#define MIN_SENSOR_ID  SENSOR_ID_NONE                        
#define MAX_SENSOR_ID  SENSOR_ID_HIGH_SIGNAL_3

#define  MAX_SENSORS      7     // There are up to 3 available sensor inputs.
                                // In addition, there are the high-2 and
                                // high-3 signal selections, and the
                                // differential sensor, all of which are 
                                // stored in this same structure.                               

#define MIN_DEG_F_OFFSET  -5  // Minimum offset, Deg F sensor type (5.0 F)
#define MAX_DEG_F_OFFSET   5  // Maximum offset, Deg F sensor type (-5.0 F)
#define MIN_DEG_C_OFFSET -25  // Minimum offset, Deg C sensor type (2.5 C)
#define MAX_DEG_C_OFFSET  25  // Maximum offset, Deg C sensor type (-2.5 C)


enum{  OUTPUT_ID_ONE,    // Output IDs. These can be used to index into
       OUTPUT_ID_TWO,    //    the global array coreDB.output[] (zero based). If you 
       OUTPUT_ID_THREE,  //    add one to this value you get the corresponding
       OUTPUT_ID_FOUR,   //    Output number, ex. OUTPUT_ID_ONE + 1 = 1
       OUTPUT_ID_FIVE,
       OUTPUT_ID_SIX,
       OUTPUT_ID_SEVEN,
       OUTPUT_ID_EIGHT,
       OUTPUT_ID_NINE,
       OUTPUT_ID_TEN  };

#define  MIN_OUTPUT_ID    OUTPUT_ID_ONE
#define  MAX_OUTPUT_ID    OUTPUT_ID_TEN

#define  MIN_ON_DELAY     0     // Minimum permitted "On Delay"
#define  MAX_ON_DELAY   300     // Maximum permitted "On Delay"                              
#define  MIN_OFF_DELAY    0     // Minimum permitted "Off Delay"
#define  MAX_OFF_DELAY  300     // Maximum permitted "Off Delay"                              
#define  MIN_ON_TIME      0     // Minimum permitted "Minimum On Time"
#define  MAX_ON_TIME    300     // Maximum permitted "Minimum On Time"
#define  MIN_OFF_TIME     0     // Minimum permitted "Minimum Off Time"
#define  MAX_OFF_TIME   300     // Maximum permitted "Minimum Off Time"

#define  MIN_SP_OUTPUT    0     // Minimum permitted "Output % at Setpoint" 
#define  MAX_SP_OUTPUT  100     // Maximum permitted "Output % at Setpoint"
#define  MIN_EP_OUTPUT    0     // Minimum permitted "Output % at Endpoint"
#define  MAX_EP_OUTPUT  100     // Maximum permitted "Output % at Endpoint"

#define  I_TERM_OFF        0    // INTEGRATOR NOT USED
#define  I_TERM_1          1    // 3600 seconds,  1 hour
#define  I_TERM_2          2    // 1800 seconds, 30 minutes
#define  I_TERM_3          3    //  900 seconds, 15 minutes
#define  I_TERM_4          4    //  300 seconds,  5 minutes
#define  I_TERM_5          5    //  120 seconds,  2 minutes
#define  I_TERM_6          6    //   60 seconds,  1 minute

#define  MIN_I_TERM  I_TERM_OFF // Minimum permitted "Integration Constant" 
#define  MAX_I_TERM  I_TERM_6   // Maximum permitted "Integration Constant"

                                // Output Update Period (called "rate")
#define  MIN_UPDATE_RATE    1   // Fastest Rate, min period = 1 second
#define  MAX_UPDATE_RATE  240   // Slowest Rate, max period = 240 seconds

                                // Output Band (limits changes to output)
#define  MIN_OUTPUT_BAND    0   //  0% = no restriction to output changes
#define  MAX_OUTPUT_BAND   50   // 50% = max delta requirement

                                // Sensor Fail Mode settings (0 = On, 1 = Off)
#define  MIN_SNF_MODE  SENSOR_FAIL_ON
#define  MAX_SNF_MODE  SENSOR_FAIL_OFF

#define  NUM_RELAY_PARAMS  8    // Number of user settable parameters 
                                //    associated with Relay output.

#define  NUM_BINARY_PARAMS 5    // Number of user settable parameters 
                                //    associated with Relay output that
                                //    references a Binary sensor.

#define  NUM_ANALOG_PARAMS 9    // Number of user settable parameters
                                //    associated with Analog output.
                              
                                // This is the maximum number of parameters
                                //    that may be configured for an output.
                                //    Relay outputs have 7 parameters,
                                //    Analog outputs have 7 parameters.
#define  MAX_PARAM_LIST  NUM_ANALOG_PARAMS

#define  OUTPUT_NAME_LENGTH  16  // In the case of the ethernet control,
                                 //   outputs and sensors can be assigned
#define  SENSOR_NAME_LENGTH  16  //   char strings that "name", or describe
                                 //   them. This information only appears
                                 //   on web pages.

#define  MAX_SENSOR_NAMES     4  // There are only 4 sensors that can be
                                 //   named; Sn-1, Sn-2, Sn-3, and Sn-d

//
//  PARAM_LIST - The param list is used to buffer the parameters that will 
//               be presented to a user as they navigate the UI. Its contents
//               depend on whether the user is setting up a Relay or an
//               Analog output.
//
typedef struct
{
    uint8_t  list_length;            // Indicates the number of items in param[]
    uint8_t  param[MAX_PARAM_LIST];  // Parameters available to edit.

}    PARAM_LIST;

//
//  SYSTEM_STRUCT - This data structure is used to hold general information
//                  about the control and expansion modules.
//
typedef struct
{
    uint8_t  model_type;    // Type of control module; C450CEN or C450CRN

    int     num_outputs;   // Indicates the number of outputs that 
                           //   have been identified as part of the 
                           //   expansion module polling process.

    // The User and Admin passwords are encoded as 16 bit unsigned values.
    // Since they must be entered on the local UI, capable of displaying
    // 4 digits, the range in value from 0000 -> 9999.
    //
    uint16_t user_password;
    uint16_t admin_password;

}    SYSTEM_STRUCT;

#define  LOCAL_PASSWORD_DISABLED    0 // If the User PW = 0, passwords disabled

#define  DEFAULT_USER_PASSWORD   LOCAL_PASSWORD_DISABLED
#define  DEFAULT_ADMIN_PASSWORD  1234

#define  MIN_LOCAL_PASSWORD         0
#define  MAX_LOCAL_PASSWORD      9999

#define  MIN_MODBUS_ADDRESS       1   // Minimum valid Modbus address
#define  MAX_MODBUS_ADDRESS     247   // Maximum valid Modbus address

enum{  BAUD_9600,           //  9,600 bps
       BAUD_19200,          // 19,200 bps  
       BAUD_38400 };        // 38,400 bps

#define  MIN_MODBUS_BAUD   BAUD_9600
#define  MAX_MODBUS_BAUD   BAUD_38400

enum{  PARITY_NONE,         // No parity
       PARITY_ODD,          // Odd parity
       PARITY_EVEN };       // Even parity

#define  MIN_MODBUS_PARITY   PARITY_NONE
#define  MAX_MODBUS_PARITY   PARITY_EVEN

enum{  STOP_BITS_ONE = 1,   // 1 stop bit
       STOP_BITS_TWO = 2};  // 2 stop bits, no parity only

#define  MIN_STOP_BIT        STOP_BITS_ONE
#define  MAX_STOP_BIT        STOP_BITS_TWO

#define  DEFAULT_MODBUS_ADDRESS   50              // Default Modbus address
#define  DEFAULT_MODBUS_BAUD      BAUD_19200      // Default Baud Rate
#define  DEFAULT_MODBUS_PARITY    PARITY_EVEN     // Default Parity
#define  DEFAULT_MODBUS_STOP_BIT  STOP_BITS_ONE   // Default Stop Bits

//
//  MODBUS_SETUP - This data structure is used to hold the setup information
//                 for the Modbus network.
//
typedef struct
{
    uint8_t  address;    // Indicates the node address of this device, 1-247

    uint8_t  baud;       // Baud rate, encoded using an enum
    uint8_t  parity;     // None, even, or odd parity
    uint8_t  stop_bits;  // 1 or 2 stop bits. 2 stop bits can only be used
                        //    when parity = None.
}    MODBUS_SETUP;

#define SIZE_MAC_ID   6

//
//  MAC_ID - This data structure is used to hold the 6 byte MAC ID,
//           or "physical address" for the Ethernet network.
//
typedef struct
{
    uint8_t  addr[SIZE_MAC_ID]; // 6 byte MAC ID. This is a globally unique
                               //   address that is loaded during manufacture.
}    MAC_ID;

#define SIZE_HTTP_USER_NAME   16
#define SIZE_HTTP_PASSWORD    16
#define SIZE_HTTP_SITE_NAME   16

enum{  ENET_ADDR_MODE_DIRECT,    // C450 acts as the DHCP server (direct connect)
       ENET_ADDR_MODE_STATIC_IP, // C450 uses a static IP address
       ENET_ADDR_MODE_AUTO };    // C450 gets its IP from a DHCP server

enum{  REMOTE_LOCKOUT_OFF,    // Remote config changes allowed
       REMOTE_LOCKOUT_ON  };  // Remote config changes prohibited

//
//  ENET_SETUP - This data structure is used to hold the setup information
//               for the Ethernet network.
//
typedef struct
{
    uint32_t  static_ip_addr;
    uint32_t  default_gateway;
    uint32_t  subnet_mask;

    uint16_t  http_port_number;
    uint8_t   dyn_dns_provider;

    // This field defines how the C450 gets its IP address, and whether
    // or not it acts as a DHCP server
    uint8_t   addr_mode;   // 0 = C450 acts as the DHCP server (direct connect)
                          // 1 = C450 uses a static IP address
                          // 2 = C450 gets its IP from a DHCP server

    uint8_t   remote_lockout;  //  0 = Remote setpoint changes are allowed
                              // >0 = Changes via web pages are NOT allowed

    uint32_t  dns_server_ip_addr;

    char     user_name[ SIZE_HTTP_USER_NAME ];
    char     password[ SIZE_HTTP_PASSWORD ];
    char     site_name[ SIZE_HTTP_SITE_NAME ];

}    ENET_SETUP;

#define SIZE_DDNS_HOST_NAME   48
#define SIZE_DDNS_USER_NAME   16
#define SIZE_DDNS_PASSWORD    16

//
//  DYNAMIC_DNS_SETUP - This data structure is used to hold the setup 
//                      information for the Dynamic DNS provider.
//
typedef struct
{
    char      host_name[ SIZE_DDNS_HOST_NAME ];
    char      user_name[ SIZE_DDNS_USER_NAME ];
    char      password[ SIZE_DDNS_PASSWORD ];

    uint32_t   external_ip_addr;

}    DYNAMIC_DNS_SETUP;


#define MIN_IP_1_ADDR    0   // Minimum IP-1 address
#define MAX_IP_1_ADDR  255   // Maximum IP-1 address

#define MIN_IP_2_ADDR    0   // Minimum IP-2 address
#define MAX_IP_2_ADDR  255   // Maximum IP-2 address

#define MIN_IP_3_ADDR    0   // Minimum IP-3 address
#define MAX_IP_3_ADDR  255   // Maximum IP-3 address

#define MIN_IP_4_ADDR    0   // Minimum IP-4 address
#define MAX_IP_4_ADDR  255   // Maximum IP-4 address

enum{  DYNDNS_PROVIDER_NONE,    // No Dynamic DNS Provider is used
       DYNDNS_PROVIDER_DYNDNS,  // dyndns.com is the provider
       DYNDNS_PROVIDER_JCI };   // jci.com is the provider

#define MIN_REMOTE_LOCKOUT  0   // 0 = False, not locked out
#define MAX_REMOTE_LOCKOUT  1   // 1 = True, remote changes disabled

// Default Static IP Ethernet Address = 169.254.13.1
//
#define DEFAULT_STATIC_IP_ADDR       IPADDR( 169, 254,   1, 1 )
#define DEFAULT_GATEWAY              IPADDR( 169, 254,   1, 2 )
#define DEFAULT_DHCP_BASE_IP_ADDR    IPADDR( 169, 254,   1, 3 )
#define DEFAULT_TFTP_IP_ADDR         IPADDR( 169, 254,   1, 4 )
#define DEFAULT_ROUTER               IPADDR( 169, 254,   1, 5 )
#define DEFAULT_DHCP_ADDR_POOL       IPADDR( 169, 254,   1, 50 )

#define DEFAULT_SUBNET_MASK          IPADDR( 255, 255,   0, 0 )

#define NUM_DHCP_CLIENTS             4 // DHCP server supports 4 clients

#define DEFAULT_HTTP_PORT            80
#define DEFAULT_DYN_DNS_PROVIDER     DYNDNS_PROVIDER_NONE
#define DEFAULT_ENET_ADDR_MODE       ENET_ADDR_MODE_DIRECT
#define DEFAULT_REMOTE_LOCKOUT       REMOTE_LOCKOUT_OFF
#define DEFAULT_HTTP_USER_NAME       "System450User1"
#define DEFAULT_HTTP_PASSWORD        "Wx9jc3"

// Default Dynamic DNS settings
//
#define DEFAULT_DYN_DNS_HOST_NAME   ""
#define DEFAULT_DYN_DNS_USER_NAME   ""
#define DEFAULT_DYN_DNS_PASSWORD    ""
#define DEFAULT_DYN_DNS_EXT_IP      IPADDR( 0, 0, 0, 0 )

//
//  ENET_STATUS - This data structure is used to hold the status information
//                for the Ethernet network.
//
typedef struct
{
    uint32_t  ip_addr;
    uint32_t  mask;
    uint32_t  gateway;

}    ENET_STATUS;


//
//  RELAY_SETUP - The relay setup contains fields for all of the parameters
//                that govern control of Relay outputs. This information is
//                saved in EEPROM.
//
typedef struct 
{
    uint8_t  sensor_id;         // Sensor 1, 2, or 3 is referenced
	                       //   by this module for control
    uint8_t  sensor_fail_mode;
    int16_t  cut_on;            // Point at which relay turns On
    int16_t  cut_off;           // Point at which relay turns Off
    int16_t  min_on_time;       // Once turned On, this is the minimum
                               //    amount of time that a relay 
                               //    will remain On. Units = Seconds
    int16_t  min_off_time;      // Once turned Off, this is the minimum
                               //    amount of time that a relay
                               //    will remain Off. Units = Seconds
    int16_t  on_delay;          // Once the conditions call for the output
                               //    to turn On, this time period must
                               //    transpire before actually turning On.
    int16_t  off_delay;         // Once the conditions call for the output
                               //    to turn Off, this time period must
                               //    transpire before actually turning Off.
}     RELAY_SETUP;


//
// ANALOG_SETUP - The analog setup contains fields for all of the parameters
//                that govern control of Analog outputs. This information is
//                saved in EEPROM.
//
typedef struct 
{ 
    uint8_t  sensor_id;        // Sensor 1, 2, or 3 is referenced
	                      //   by this module for control
    uint8_t  sensor_fail_mode;
    int16_t  sp;               // Setpoint, target process variable
    int16_t  ep;               // End Point, point of full output 
    uint8_t  sp_output;        // Analog output, 0-100%, at SP
    uint8_t  ep_output;        // Analog output, 0-100%, at EP
    uint8_t  int_constant;     // Integration Constant
    uint8_t  update_rate;      // Output Update Rate (Period, in seconds)
    uint8_t  output_band;      // Output Band (0-50%)
    uint8_t  reserved;         // Force size of ANALOG_SETUP = RELAY_SETUP

}     ANALOG_SETUP;


typedef union                 // Union of output setup data structures
{
    RELAY_SETUP   relay;      // Used to buffer addr for gateway.
    ANALOG_SETUP  analog;     // Used to buffer measured data.

}    OUTPUT_UNION;


typedef struct                  // Output Setup data structure
{
    uint8_t        output_type;  // Analog or Relay output

    OUTPUT_UNION  output;       // Contains output setup data

}    OUTPUT_SETUP;


//
//  CALIBRATION - This data structure is used to hold calibration data
//                for resistive and voltage type sensor inputs. The gain
//                and offset are applied to the ADC count value before
//                the count value is converted to engineering units.
//
typedef struct
{
    float   five_volt_external;   // Voltage supplied at the wiring terminal
                                  //   by the C450

    uint16_t volt_adc_ground_1;    // These 3 values are the observed ADC
    uint16_t volt_adc_ground_2;    //   values when the sensor input was
    uint16_t volt_adc_ground_3;    //   connected to "C" (digital ground).

    uint16_t volt_adc_5Vext_1;     // These 3 values are the observed ADC
    uint16_t volt_adc_5Vext_2;     //   values when the sensor input was
    uint16_t volt_adc_5Vext_3;     //   connected to the 5V reference.

    int16_t  resistive_offset_1;   // These 3 values are the offset that
    int16_t  resistive_offset_2;   //   was determined by calculating the
    int16_t  resistive_offset_3;   //   difference between expected and 
                                  //   observed ADC values when a 1k ohm
                                  //   resistor was connected to the input.
}     CALIBRATION;

// Default calibration values
//
#define DEFAULT_CAL_5_VOLT_EXTERNAL   5.0
#define DEFAULT_CAL_VIN_GROUND          0
#define DEFAULT_CAL_VIN_5_VOLT      28254
#define DEFAULT_CAL_RIN_OFFSET          0

// Range limits on the calibration values
//
#define CAL_MIN_5_VOLT_EXTERNAL       4.0
#define CAL_MAX_5_VOLT_EXTERNAL       6.0
#define CAL_MIN_5_VIN_GROUND            0
#define CAL_MAX_5_VIN_GROUND         1000
#define CAL_MIN_5_VIN_5_VOLT        10000
#define CAL_MAX_5_VIN_5_VOLT        65000
#define CAL_MIN_5_RIN_OFFSET        -1000
#define CAL_MAX_5_RIN_OFFSET         1000

typedef struct                // Sensor Setup identifies the type
{                             //    of sensor that is being used.
    uint8_t  sensor_type;      //    In the case of temperature 
    int8_t   offset;           //    sensors, an Offset is supported.

}     SENSOR_SETUP;


//
//  SENSOR - The sensor data struct contains fields for both setup and
//           status information. The status information (ie. value in
//           engineering units) is routinely calculated. To perform
//           those calculations, the setup info is required. 
//
typedef struct
{
    SENSOR_SETUP  setup;        // Sensor type and offset
    
    int16_t        value_int;    // Engineering units, as an integer
    float         value_float;  // Engineernig units, floating point
    
    uint8_t        fail   : 1;   // 1 = sensor has failed
                                // 0 = sensor is Ok
    uint8_t        unused : 7;
    
    double        signal;       // Either a resistance (ohms) or voltage (vdc),
                                //   depending on the sensor type.
                                // The signal value is used in a subsequent
                                //   equation to calculate temperature,
                                //   pressure, or humidity.
 
    char          name[SENSOR_NAME_LENGTH+1];

}     SENSOR;


//
//  OUTPUT - The output data struct contains everything related to a 
//           specific output; I2C addresses, control status, setup
//           parameters, and a reference to a controlling sensor.
//
typedef struct
{
    uint8_t        module_addr  : 4;  // I2C Addr of slave module, 0 = this module

    uint8_t        point_addr   : 1;  // Point address within a module. In the
                                      //   case of a 2 relay module, the point
                                      //   address identifies relay 1 or 2.
                                      
    uint8_t        sensor_valid : 1;  // 0 : No sensor selected, or the selected
                                      //     sensor is of type SENSOR_TYPE_NONE
                                      // 1 : A configured sensor is referenced.
                                      //     by this Output. This DOES NOT
                                      //     imply that the data from the sensor
                                      //     is within an acceptable range. 
                                      //     The Sensor Fail flag determines
                                      //     if the data is good.

    uint8_t        unused       : 2;  // remaining 2 bits of the byte.
    
    uint8_t        desired_state;     // Desired state, set by control algorithm
    uint8_t        output_state;      // Output state, actual state of output

    float          i_term;            // Integral term, when supporting an
                                      //   analog module w/ PI control.   

                                      // When TRUE the On/Off Delay condition
                                      //   is being timed (On/Off condition
                                      //   must remain TRUE the entire time).
    bool           on_delay_timer_running;
    bool           off_delay_timer_running;

    int16_t        on_delay_timer;    // Countdown timers to maintain On Delay
    int16_t        off_delay_timer;   // Countdown timers to maintain Off Delay
    int16_t        min_on_timer;      // and minimum On\Off time requirements.
    int16_t        min_off_timer;  

    bool           force_update;      // When TRUE this causes an analog
                                      //   output to be updated regardless
                                      //   of the update timer and/or the
                                      //   output band.

    int16_t        update_timer;      // Countdown timer used to maintain
                                      //   the output update rate of an
                                      //   analog output.

    int16_t        i2c_error_count;   // Count of the number of I2C message
                                      //   failures to an output.

    char           name[OUTPUT_NAME_LENGTH+1];
    OUTPUT_SETUP   setup;             // Output Setup parameters
    SENSOR       * sensor;            // Reference to the controlling sensor

}   OUTPUT;


//
//  DATABASE - The "database" structure contains all of the Sensor
//             and Output status and setup information. This typedef
//             is used by various tasks that create and use a copy of 
//             the "core" database as they perform their specific
//             functions. The core database is synchronized by the 
//             copies used by the various tasks by a Mutex. The
//             multiple copies and syncrhonization are necessitated
//             by the nature of this device and the operating system.
//             There are multiple mechanisms by which data can be
//             written or read (local UI, RS485, ethernet), and there
//             are rules which govern the validity of the core data
//             as it relates to sensor and output setup values.
//
typedef struct
{  
    OUTPUT            output[ MAX_OUTPUTS ];  // Output Status and Setup params
    SENSOR            sensor[ MAX_SENSORS ];  // Sensor Status and Setup params

    MODBUS_SETUP      modbus_setup;           // Modbus setup params
    ENET_SETUP        enet_setup;             // Ethernet setup params
    DYNAMIC_DNS_SETUP dyndns;                 // Dynamic DNS setup params
    MAC_ID            mac;   

    float             cpu_temp;               // CPU Temp, in degrees F
    float             five_volt_ext;          // 5 volt external
    float             ten_volt_ref;           // 10 volt reference

}   DATABASE;


//
//  SCREEN_TYPE.. - A screen type describes the basic funcitionality of
//                  a given display screen. The enums defined here are
//                  used in the SCREEN_TEMPLATE field "screen_type"
//
enum{  SCREEN_TYPE_STATUS,       // Status display, Up/Down keys have no function
       SCREEN_TYPE_STATUS_LIST,
       SCREEN_TYPE_STATUS_NUMERIC,
       SCREEN_TYPE_EDIT_NUMERIC, // Display for editing a numeric value
       SCREEN_TYPE_SELECT_LIST,  // Display for selecting one item from a list
       SCREEN_TYPE_SUBMENU,      // Menu prompt, blinking dashes
       SCREEN_TYPE_PASSWORD,     // Display for entering a password
       SCREEN_TYPE_MESSAGE,      // Display msg, and maintain timeout timer
       SCREEN_TYPE_ERROR };      // Error prompt, static text lines 1 & 2


//
//  ITEM_NUM.. - Various display screens will include a number to identify
//               such things as; Output Number, Sensor Number, or they
//               may contain no identifying number at all. These enums are
//               used in the SCREEN_TEMPLATE field "item_num_flag". The
//               value of the flag indicates if an identifying number should 
//               be used, the source of that number, and where it should
//               be displayed.
//
enum{  ITEM_NUM_NONE,           // Do not display any identifying number
       ITEM_NUM_OUTPUT_ALPHA,   // Display Output number  line 2 of LCD
       ITEM_NUM_OUTPUT_RIGHT,   // Display Output number, right of line 2
       ITEM_NUM_SENSOR_ALPHA,   // Display Sensor number  line 2 of LCD
       ITEM_NUM_SENSOR_RIGHT,   // Display Sensor number, right of line 2
       ITEM_NUM_1,              // Display digit "1", item num field
       ITEM_NUM_2 };            // Display digit "2", item num field

//
//  LIST_.. - Some of the setup parameters use the concept of a list, 
//            whereby a user selects one of the available options. This
//            enum is used to identify the individual lists. It is
//            used in the SCREEN_TEMPLATE field "list_id". 
//
enum{  LIST_NONE,               // Selection list is not provided
       LIST_SENSOR_TYPE,        // List of Sensor Types; F, C, P100, ...
       LIST_SENSOR,             // List of Sensors; Sn-1, Sn-2, ...
       LIST_SENSOR_FAILURE,     // List of Fail Modes; On, Off
       LIST_REMOTE_LOCKOUT,     // List of Remote Lockout; On, Off
       LIST_ENET_ADDR_MODE,     // List of Enet Addr Modes: 0, 1, 2
       LIST_ENET_RESET,         // List used to provide a prompt: "rES"
       LIST_BAUD_RATE  };       // List of Baud Rates; 1200, 9600, 38.4 ...


//
//  PATH_.. - These identifiers are used to indicate what path of the 
//            UI navigation tree we are currently on. They are 
//            used in the SCREEN field "path_id". 
//
enum{  PATH_NONE,            // No menu, or menu branch is active, which means
                             //    this identifies the main status display screen
       PATH_STATUS,          // Sensor and Output status branch is active
       PATH_PASSWORD,        // Password entry prior to setup menu access
       PATH_SENSOR,          // Sensor setup branch is active
       PATH_OUTPUT_SETUP,    // Output setup branch is active
       PATH_MODBUS_SETUP,    // Modbus setup branch is active
       PATH_ENET_SETUP,      // Ethernet setup branch is active
       PATH_USER_SETUP,      // User password setup
       PATH_ADMIN_SETUP,     // Admin password setup
       PATH_ERROR_SCREEN };  // Special case, Error display

//
//  TEMPLATE_.. - These enumerations identify a specific templates. They are
//                used when loading screen. A template includes things such
//                as a prompt, indicating what the user is looking at.
//
//                The enumerations can be used as indices into the
//                global Template[] array.                             
//
enum{  TEMPLATE_STATUS,               // Main status screen
       TEMPLATE_OUTPUT_STATUS_MENU,   // Output Status Menu Prompt
       TEMPLATE_SENSOR_MENU,          // Sensor Setup Menu Prompt
       TEMPLATE_OUTPUT_SETUP_R_MENU,  // Output Setup Menu Prompt - Relay
       TEMPLATE_OUTPUT_SETUP_A_MENU,  // Output Setup Menu Prompt - Analog
       TEMPLATE_SYSTEM_MENU,          // System Setup Menu Prompt
       TEMPLATE_USER_MENU,            // User Password Setup Menu Prompt
       TEMPLATE_ADMIN_MENU,           // Admin Password Setup Menu Prompt
       TEMPLATE_MODBUS_MENU,          // Modbus Network Setup Menu Prompt
       TEMPLATE_ENET_MENU,            // Ethernet Setup Menu Prompt
       TEMPLATE_SENSOR_STATUS,        // Sensor Status Display
       TEMPLATE_DIFF_SENSOR_STATUS,   // Differential Sensor Status Display
       TEMPLATE_OUTPUT_STATUS,        // Output Status Display
       TEMPLATE_SENSOR_TYPE,          // Edit Sensor Type
       TEMPLATE_CUT_ON,               // Edit Cut On - Relay output
       TEMPLATE_CUT_OFF,              // Edit Cut Off - Relay output
       TEMPLATE_DIFF_CUT_ON,          // Edit Differential Cut On - Relay output
       TEMPLATE_DIFF_CUT_OFF,         // Edit Differential Cut Off - Relay output
       TEMPLATE_ON_DELAY,             // Edit On Delay - Relay output
       TEMPLATE_OFF_DELAY,            // Edit Off Delay - Relay output
       TEMPLATE_ON_TIME,              // Edit Minimum On Time - Relay output
       TEMPLATE_OFF_TIME,             // Edit Minimum Off Time - Relay output
       TEMPLATE_SEL_SENSOR,           // Edit Select Reference Sensor - Relay and Analog
       TEMPLATE_SP,                   // Edit SP - Analog output setpoint
       TEMPLATE_EP,                   // Edit EP - Analog output endpoint
       TEMPLATE_DIFF_SP,              // Edit Differential SP - Analog output setpoint
       TEMPLATE_DIFF_EP,              // Edit Differential EP - Analog output endpoint
       TEMPLATE_INT_CONSTANT,         // Edit Integration Constant - Analog output
       TEMPLATE_OUTPUT_SP,            // Edit Output @ SP - Analog output
       TEMPLATE_OUTPUT_EP,            // Edit Output @ EP - Analog output
       TEMPLATE_UPDATE_RATE,          // Edit Output Update Rate - Analog output
       TEMPLATE_OUTPUT_BAND,          // Edit Output Band - Analog output
       TEMPLATE_SENSOR_FAILURE,       // Edit Sensor Fail Mode - Analog output
       TEMPLATE_SENSOR_OFFSET,        // Edit Sensor Offset - deg F and deg C Sensors
       TEMPLATE_NODE_ADDRESS,         // Edit Node Address (RS485 comm)
       TEMPLATE_BAUD_RATE,            // Edit Baud Rate (RS485 comm)
       TEMPLATE_PARITY,               // Edit Parity (RS485 comm)
       TEMPLATE_STOP_BITS,            // Edit Stop Bits (RS485 comm)
       TEMPLATE_REMOTE_LOCKOUT,       // Edit Remote Lockout (ethernet)
       TEMPLATE_ENET_IP_1,            // Edit Enet IP Addr (ie. IP1.<>.<>.<>)
       TEMPLATE_ENET_IP_2,            //                   (ie. <>.IP2.<>.<>)
       TEMPLATE_ENET_IP_3,            //                   (ie. <>.<>.IP3.<>)
       TEMPLATE_ENET_IP_4,            //                   (ie. <>.<>.<>.IP4)
       TEMPLATE_ENET_ADDR_MODE,       // Static IP, DHCP Client, Direct Connect
       TEMPLATE_ENET_IP_RESET,        // Prompt to reset Ethernet settings
       TEMPLATE_PASSWORD,             // Prompt to enter Password
       TEMPLATE_ADMIN_PASSWORD,       // Prompt to enter Admin Password
       TEMPLATE_EDIT_USER,            // Prompt to edit User Password
       TEMPLATE_EDIT_ADMIN_1,         // Prompt to edit Admin Password 1
       TEMPLATE_EDIT_ADMIN_2,         // Prompt to edit Admin Password 2
       TEMPLATE_MESSAGE_FAIL,         // Indicate input Failure (edit admin pw)
       TEMPLATE_MESSAGE_OK,           // Indicate input Ok (edit admin pw)
       TEMPLATE_ERROR };              // Error msg, 2 lines of static text

//
//  SCREEN_TEMPLATE - This data struct contains fields that define the 
//                    content and behavior of a particular screen. It
//                    provides the basic framework, or template.
//
typedef struct
{
    char    prompt[4];      // Contains indices into the alpha char array
    uint8_t screen_type;    // Indicates is screen is used to;
	                    //    Display Status, Edit Numeric Value, 
	                    //    Select List Item, or is a Submenu
 
                            // This flag indicates if the "item number"
    uint8_t item_num_flag;  //   should be displayed as part of the
                            //   screen template. An item number may be 
                            //   a module number, or a sensor number.
                            //   It differentiates one module or sensor
                            //   from the other.
                            // The flag also indicates if the item number 
                            //   should be displayed as part of the 4 
                            //   character alphanumeric area, or to the
                            //   right of the alphanumeric area,
                            //   ... or not at all.
    
    uint8_t list_id;        // In the case where a list selection is offered,
	                    //    this field identifies which list to use.
}    SCREEN_TEMPLATE;


//
//  PASSWORD_ENTRY -  This data struct contains fields that define the 
//                    content and behavior of screens that are used to
//                    enter a password. Password entry follows a different
//                    convention compared to all other screens. 
//
//                    Passwords consists of 4 digits, numbered 1-4,
//                    from left to right. The digits are entered one at
//                    a time, and only the digit that is being altered will
//                    blink. 
//
//                    The range of all passwords is 0 - 9999
//
//                    When changing the Admin password, it must be
//                    entered twice, identically. Hence the two buffers
//                    for this value.
//
typedef struct
{
    uint16_t digit_1;       // Contains the 1000's digit
    uint16_t digit_2;       // Contains the 100's digit
    uint16_t digit_3;       // Contains the 10's digit
    uint16_t digit_4;       // Contains the 1's digit

    uint8_t  active_digit;  // Indicates the digit currently being 
                            //   edited. The value will be 1-4.

    uint16_t access;        // Buffer of the password that was entered
                            //   in the attempt to access password
                            //   protected features.

                            // The following buffers are used when
                            //   editing the User/Admin passwords as
                            //   part of the System Setup.
    uint16_t user;          // Buffer of the User password edit
    uint16_t admin_1;       // Buffer of the Admin 1 password edit
    uint16_t admin_2;       // Buffer of the Admin 2 password edit

}    PASSWORD_ENTRY;


#define MSG_TIME  10        // 10 = 5 seconds, used with a 1/2 second
                            //    timer.
//
//  SCREEN - This data struct contains is used extensively to manage
//           the User Interface. It contains all of the information 
//           needed to draw the screen, and respond to any user input.
//
typedef struct
{
    const SCREEN_TEMPLATE * template;  // Points to the template which provides
                                 //    general screen layout information.
                                    
    OUTPUT *  Output;            // Points to a module containing Output
                                 //    setup information. The first two 
                                 //    modules are part of this control 
                                 //    (the hardware running this application).
                                 //    Additional modules are expansion 
                                 //    modules accessed via the IIC bus.
                                    
    uint8_t   output_id;         // Identifies currently selected Output. 
                                 //    Output 1 & 2 are part of the this control
                                 //    Outputs 3 - n are expansion modules.    
                                    
    uint8_t   param_id;          // Identifies currently selected parameter
    
    uint8_t   path_id;           // Identifies current menu path. If this value
                                 //    is MENU_NONE, it can be assumed that the
                                 //    main status display is active.
                                    
    uint8_t   sensor_id;         // Identifies Sensor 1, 2, or 3
    
    uint8_t   param_index;       // This is an index into one of the "lists".
                                 //    A list may contain a number of params
                                 // associated with a menu, or a number of 
                                 // user selectable options.
                                 //    Setting up a Relay Output uses a list
                                 // of parameters associated with relay outputs.
                                 //    Setting up a sensor type uses a list
                                 // of sensor types.
                                 //    The list_index keeps track of where
                                 // a user is as the navigate, or make a 
                                 // selection from these lists.
                                    
    int16_t   param_value;       // Value of the field currently displayed   

    uint8_t   check_sens_id : 1; // This flag is set when editing the 
                                 //   reference sensor of an output, when
                                 //   there is no currently selected 
                                 //   sensor. This is necessary because it
                                 //   is a special case that alters the
                                 //   normal flow of the User Interface.
                                       
    uint8_t   decimal_pt    : 7; // Position of the decimal point

    int16_t   min_value;         // Minimum value permitted for parameter
    int16_t   max_value;         // Maximim value permitted for parameter
    int16_t   inc_value;         // Value to add or subtract to the parameter
                                 //    that is being edited

    PASSWORD_ENTRY  password;

    // There are a couple of instances where a message is displayed for
    //   a period of time. After the message times out, the next_template
    //   and next_param_id indicate what screen should be loaded.
    uint16_t                timeout;
    const SCREEN_TEMPLATE * next_template; 
    uint8_t                 next_param_id;

}    SCREEN;


// The following two threshold values are the minimum supply power and
//   sensor power measurements that must be observed before it is determined
//   that there is a problem. The Supply Power is measured by looking
//   at the 10v reference.
//
#define SUPPLY_POWER_THRESHOLD  (float) 8  
#define SENSOR_POWER_THRESHOLD  (float) 4.75

#define SUPPLY_POWER_DELAY_TIME   10    // 10 second delay before a supply
                                        //    power failure is declared.	

#define SENSOR_POWER_DELAY_TIME   10    // 10 second delay before a sensor
                                        //    power failure is declared.	

//
// ERROR_CONDITIONS - This data struct is used to indicate two possible
//                    errors;  
//
//    Supply Power Failed : True when the 10V ref is low
//
//    Sensor Power Failed : True when the 5V ext is low
//
typedef struct
{
    uint8_t supply_power_fail    : 1;  // Supply power failed
    uint8_t supply_power_pending : 1;  // Supply voltage dipped,
                                       //    start delay before calling
                                       //    out a supply power failure.
    uint8_t sensor_power_fail    : 1;  // Sensor power failed
    uint8_t sensor_power_pending : 1;  // Sensor voltage dipped,
                                       //    start delay before calling
                                       //    out a supply power failure.
    uint8_t reserved             : 4;

    uint8_t supply_power_delay;        // Countdown timer, loaded when
                                       //    low supply power detected,
                                       //    and decremented once per
                                       //    second while condition exists.
                                       //    When it counts down to zero,
                                       //    a supply power failure will
                                       //    be called out.

    uint8_t sensor_power_delay;        // Countdown timer, loaded when
                                       //    low sensor power detected,
                                       //    and decremented once per
                                       //    second while condition exists.
                                       //    When it counts down to zero,
                                       //    a sensor power failure will
                                       //    be called out.
}    ERROR_CONDITIONS;


//
// DISPLAY_CONTENT - This data structure is used to hold the desired
//                   content of the LCD. It includes all fields that
//                   are available on the LCD.
//
//                   Most of those fields are expressed as ascii 
//                   characters. These include the four 7-segment
//                   characters on the top row of the LCD. They
//                   are numbered, left to right, as 1-4 (C1-C4).
//
//                   A second row of character form the bottom 
//                   row of the LCD, numbered left to right as
//                   5-9 (C5-C9).
//          
//                   In addition to these 9 characters, bit
//                   fields are used to define special segments
//                   including; 
//                         decimal points for C1, C2, C3.
//                         ":" symbol between C2 and C3
//                         "M" symbol following C9
//                         5 segments forming the "ramp" icon
//                         commands for Xmit and Recv LEDs
//
typedef struct
{
    uint8_t cmd;

    uint8_t c1;   // Ascii char for the 7-segment character,  C1
    uint8_t c2;   // Ascii char for the 7-segment character,  C2
    uint8_t c3;   // Ascii char for the 7-segment character,  C3
    uint8_t c4;   // Ascii char for the 7-segment character,  C4
    uint8_t c5;   // Ascii char for the 13-segment character, C5
    uint8_t c6;   // Ascii char for the 16-segment character, C6
    uint8_t c7;   // Ascii char for the 13-segment character, C7
    uint8_t c8;   // Ascii char for the 13-segment character, C8
    uint8_t c9;   // Ascii char for the 7-segment character,  C9

    uint8_t colon     : 1;  // The ":" symbol separates C2 : C3
    uint8_t M         : 1;  // The "M" is used in AM \ PM displays
    uint8_t dp1       : 1;  // DP3-1 is a decimal point following
    uint8_t dp2       : 1;  //    C3, C2, C1
    uint8_t dp3       : 1;
    uint8_t reserved1 : 3;

    uint8_t ee        : 1;  // The 5 fields define the ramp icon.
    uint8_t dd        : 1;
    uint8_t cc        : 1;
    uint8_t bb        : 1;
    uint8_t aa        : 1;
    uint8_t reserved2 : 3;

    uint8_t xmit_led  : 4;
    uint8_t recv_led  : 4;

}    DISPLAY_CONTENT;


#define PC_CONTROL_TIME      15   // Number of seconds that a PC (I2C)
                                  //    command will be allowed to 
                                  //    control an output, after which
                                  //    the standard control algorithm
                                  //    is used.	

#define PC_LCD_CONTENT_TIME  15   // Number of seconds that a PC (I2C)
                                  //    will be allowed to determine
                                  //    the content of the LCD, after which
                                  //    standard LCD updating is used.

#define PC_KEYPAD_TEST_TIME  15   // Number of seconds that the Keypad
                                  //    Test mode will remain active, once
                                  //    it is started. During the keypad test
                                  //    the LCD will reflect the last key
                                  //    that was pressed.
//
//  PC_CONNECT - This data struct is used to manage the I2C interface as
//               it is used during factory test. In this case, a "PC",
//               or some type of functional test equipment, will 
//               interact with the Device Under Test (DUT), commanding
//               the outputs, calibrating sensor inputs, testing the LCD, etc.
//
//               This data structure is used to maintain the state of
//               this connection, and support the tests that are 
//               being conducted.
//
typedef struct
{
    uint8_t request;          // Request received from functional
                              //    test fixture, the "PC"

    uint8_t  last_key;
    uint8_t output_id;        // Output identifier of the PC commands
                              //   used to read/write the output setup.

    uint8_t cmd_output;       // Actual output command
    
    uint8_t cmd_output_id;    // Output identifier of the PC commands
                              //   used to force an output On\Off.
    
    uint8_t control_active;   // Count Down Timer, indicating that
                              //   an output command is active.
                              //   Control of an output will revert
                              //   to the standard control algorithm
                              //   after this timer counts down
                              //   to zero.
                                       
    uint8_t lcd_active;       // Count Down Timer, indicating that
                              //   an LCD command is active. Control
                              //   of the LCD will revert to normal
                              //   after this timer counts down to zero.

    uint8_t keypad_test_active;  // Count Down Timer, indicating that
                                 //   a keypad test is active. Response to
                                 //   keystrokes and updating of the LCD 
                                 //   will revert to normal after
                                 //   this timer counts down to zero.

    DISPLAY_CONTENT  display;    // Buffer to hold the entire LCD display
                                 //   content as commanded by the PC.
}    PC_CONNECT;

#endif
