/**************************************************************************
*
* FileName: Init_Task.c
*
* Comments: Initialization Task to kick start all the other tasks and also
*           start the webserver.
*
*           This is the only task with MQX_AUTO_START_TASK attribute set
*           in the MQX_template_list[] (defined below). 
*
***************************************************************************/

#include <string.h>
#include <stdlib.h>
#include "defines.h"
#include "system450.h"
#include "global.h"
#include "c450_io.h"


#include "HVAC.h"
#include "atheros_driver_includes.h"

//#include "watchdog_func.h"
#include "Sensor_Task.h"
//#include "Control_Task.h"
//#include "UI_Task.h"

//    #include "rtcs_func.h"
//    #include "web_func.h"
//    #include "EnetHelp_Task.h"
//    #include "DynDNS_Task.h"

//#include "Watchdog_Task.h"
//#include "pcconnect.h"
//#include "i2cTerminal.h"
//#include "i2cExpModules.h"
//#include "sensors.h"
//#include "eeprom.h"
//#include "func.h"
//#include "periodic_events.h"






// Prototype for Init_Task, which will be used to start all other tasks
//
void Init_Task(uint_32);

extern boolean   get_number_string( char * number_str );

//
//  MQX initialization information
//
const TASK_TEMPLATE_STRUCT MQX_template_list[] =
{
  // Task Index,     Function,        Stack,  Priority, Name,        Attributes,         Param,  Time Slice
  { INIT_TASK,       Init_Task,        2000,   13,     "Init",       MQX_AUTO_START_TASK, 0,      0 },
  { WMICONFIG_TASK1, wmiconfig_Task1,  3000,    9,     "WMICONFIG1", 0,                   0,      0 },   
  { WMICONFIG_TASK2, wmiconfig_Task2,  3000,    9,     "WMICONFIG2", 0,                   0,      0 },
  { HVAC_TASK,       HVAC_Task,        1400,   10,     "HVAC",       0,                   0,      0 },

#if DEMOCFG_ENABLE_SERIAL_SHELL
  { SHELL_TASK,      Shell_Task,       2500,   15,     "Shell",      0,                   0,      0 },
#endif

//  { HEARTBEAT_TASK,  HeartBeat_Task,   1500,   14,     "HeartBeat",  0,                   0,      0 },
  { SENSOR_TASK,     Sensor_Task,      1000,    9,     "Sensor",     0,                   0,      0 },
  //added
  { BUTTON_TASK,     Button_Task,      1500,    9,      "Button",    0,                   0,      0 },
  { RCONTROL_TASK,   RControl_Task,    1500,    9,      "Control",   0,                   0,      0 },   

  {0}
};


void init_globals( void );
void init_gpio( void );

//
//   Init_Task() - Kick-start all the other tasks used in this application
//
void Init_Task(uint_32 data)
{
    // Initialize the mutex that will be used to protect / synchronize the
    //    core data that is shared by the tasks. Any task wanting to read
    //    or write to this core data must first "lock" the mutex, perform
    //    the memory read/write, and then "unlock" the mutex. Included in
    //    the core data are the Output[] structures, and Sensor[] 
    //    structures.
    //
    if( _mutex_init( &mutexCore, NULL ) != MQX_OK )
    {
        _mqx_exit( 0 );   // "0" is the error code
    }

    TickPerSecond = _time_get_ticks_per_sec();
    HwTickPerTick = _time_get_hwticks_per_tick();


    // Calculate a timeout value to be used in the I2C communication
    //    routines that is the equivalent of 10 mSec. The units
    //    of this value should be "hwticks".
    //
    HwTickI2cTimeout = (TickPerSecond * HwTickPerTick) / 100;

    _time_get_ticks( &TickStruct );

    
    init_gpio();

    init_globals();

    
// DEBUG VARIABLES - can be accessed via a Watch window
ptrPA = PORTA_BASE_PTR;
ptrPB = PORTB_BASE_PTR;
ptrPC = PORTC_BASE_PTR;
ptrPD = PORTD_BASE_PTR;
ptrPE = PORTE_BASE_PTR;


PtrAdc0 = (ADC_MemMapPtr) ADC0_BASE_PTR;  // Ptrs to ADC-0 and ADC-1 registers
PtrAdc1 = (ADC_MemMapPtr) ADC1_BASE_PTR;

ptrSim  = SIM_BASE_PTR;

ptrI2c0 = I2C0_BASE_PTR;
ptrI2c1 = I2C1_BASE_PTR;

ptrPmc    = PMC_BASE_PTR; // Power Management Register, doesn't work with watch
PmcLvdsc1 = PMC_LVDSC1;
PmcLvdsc2 = PMC_LVDSC2;
PmcRegsc  = PMC_REGSC;
// END DEBUG VARIABLES


//    pet_watchdog();

    BaseInfo.fw_version = FW_VERSION;
    BaseInfo.ssm        = SSM_NUMBER;
    BaseInfo.sim_sdid   = SIM_SDID;    // Read the Device ID Register

    // Start the PIT0 timer which will schedule running of
    //    the Control and UI tasks.
    //
    init_event_handlers();

    //  Install the unexpected ISR handler.
    _int_install_unexpected_isr();
//DES Installs the MQX-provided _int_exception_isr() as the default ISR for unhandled interrupts and exceptions.
    _int_install_exception_isr();                   //DES added



    _task_create( 0, WMICONFIG_TASK1, 0 );
    _task_create( 0, WMICONFIG_TASK2, 0 );
    _task_create( 0, HVAC_TASK,       0 );
    _task_create( 0, SHELL_TASK,      0 );
    _task_create( 0, HEARTBEAT_TASK,  0 );
    
    _task_create( 0, BUTTON_TASK,     0);
    _task_create( 0, RCONTROL_TASK,    0);

    // Create Sensor Task to routinely sample the sensor inputs
    _task_create( 0, SENSOR_TASK, 0 );

    _task_block();
}

void
init_gpio( void )
{
    //set leds as outputs
    PORTA_PCR1  = PORT_PCR_MUX(1); 
    PORTA_PCR2  = PORT_PCR_MUX(1);
    
    //set ports to gpio 
    PORTD_PCR2 = PORT_PCR_MUX(1);
    
    //set C4 as output
    PORTC_PCR4  = PORT_PCR_MUX(1); //io used for fan
    
    GPIOA_PDDR |= 0x00000006;   // Set bits 1&2, make pin an output
    GPIOD_PDDR |= 0x00000004; //set bit 2
    GPIOC_PDDR |= 0x00000010;  //set bit 5
    
    //SW3 button as input
    PORTB_PCR17 = PORT_PCR_MUX(1); 
    GPIOB_PDDR &= 0xFFFDFFFF; 
     
    //SW2 button as input
    PORTC_PCR1  = PORT_PCR_MUX(1);
    GPIOC_PDDR &= 0xFFFFFFFD; 
}


//
//   init_globals()
//
//   1. Initialize all of the globa data
//   2. Read EEPROM, loading defaults if corrupted
//   3. Poll expansion bus to determine number / type of outputs
//
void
init_globals( void )
{
//    EXP_POLL_RESPONSE   poll;
    int                 temp, k;

    System.model_type = MODULE_TYPE_C450CEN;

    
    //  Clear all Error flags, intialize countdown timers
    Error.supply_power_fail    = FALSE;
    Error.supply_power_pending = FALSE;
    Error.sensor_power_fail    = FALSE;
    Error.sensor_power_pending = FALSE;
    Error.supply_power_delay   = SUPPLY_POWER_DELAY_TIME;
    Error.sensor_power_delay   = SENSOR_POWER_DELAY_TIME;

    CommLedRequest.update_xmit_led = FALSE;
    CommLedRequest.update_recv_led = FALSE;
    CommLedRequest.xmit_led        = COMM_LED_OFF;
    CommLedRequest.recv_led        = COMM_LED_OFF;

    // If EEPROM fails, load default calibration data to EEPROM
    //    and to the global variable CalData.
    //
//    if( ee_read_calibration( &CalData ) == FALSE )
//    {
        CalData.five_volt_external = DEFAULT_CAL_5_VOLT_EXTERNAL;

        CalData.volt_adc_ground_1  = DEFAULT_CAL_VIN_GROUND;
        CalData.volt_adc_ground_2  = DEFAULT_CAL_VIN_GROUND;
        CalData.volt_adc_ground_3  = DEFAULT_CAL_VIN_GROUND;

        CalData.volt_adc_5Vext_1   = DEFAULT_CAL_VIN_5_VOLT;
        CalData.volt_adc_5Vext_2   = DEFAULT_CAL_VIN_5_VOLT;
        CalData.volt_adc_5Vext_3   = DEFAULT_CAL_VIN_5_VOLT;

        CalData.resistive_offset_1 = DEFAULT_CAL_RIN_OFFSET;
        CalData.resistive_offset_2 = DEFAULT_CAL_RIN_OFFSET;
        CalData.resistive_offset_3 = DEFAULT_CAL_RIN_OFFSET;

//        ee_write_calibration( &CalData );
//    }

    // If EEPROM fails, load default calibration data to EEPROM
    //    and to the global variable CalData.
    //
//    if( ee_read_local_password( &System ) == FALSE )
//    {
        System.user_password  = DEFAULT_USER_PASSWORD;
        System.admin_password = DEFAULT_ADMIN_PASSWORD;

//        ee_write_local_password( &System );
//    }
 
    // Range check passwords. This should never happen, but the 
    //   functional test fixture has the ability to change these
    //   via the I2C bus.
    //
//    if( (System.user_password > MAX_LOCAL_PASSWORD) ||
//        (System.admin_password > MAX_LOCAL_PASSWORD)  )
//    {
        System.user_password  = DEFAULT_USER_PASSWORD;
        System.admin_password = DEFAULT_ADMIN_PASSWORD;

//        ee_write_local_password( &System );
//    }

//    if( ee_read_mac_id( &coreDB.mac ) == FALSE )
//    {
        memset( &coreDB.mac, 0, SIZE_MAC_ID );
//        ee_write_mac_id( &coreDB.mac );
//    }

//    if( ee_read_enet_setup( &coreDB.enet_setup ) == FALSE )
//    {
        load_default_enet_setup( &coreDB.enet_setup );
//        ee_write_enet_setup( &coreDB.enet_setup );
//    }

//    if( ee_read_ddns_setup( &coreDB.dyndns ) == FALSE )
//    {
        strcpy( coreDB.dyndns.host_name, DEFAULT_DYN_DNS_HOST_NAME );
        strcpy( coreDB.dyndns.user_name, DEFAULT_DYN_DNS_USER_NAME );
        strcpy( coreDB.dyndns.password,  DEFAULT_DYN_DNS_PASSWORD );

//        ee_write_ddns_setup( &coreDB.dyndns );
//    }

//    if( ee_read_external_ip( &coreDB.dyndns.external_ip_addr ) == FALSE )
//    {
        coreDB.dyndns.external_ip_addr = DEFAULT_DYN_DNS_EXT_IP;

//        ee_write_external_ip( &coreDB.dyndns.external_ip_addr );
//    }


    // If EEPROM fails, load default sensor data to EEPROM
    //    and to the global variable coreDB.sensor[].
    //
//    if( ee_read_sensor_setup( &coreDB.sensor[SENSOR_ID_ONE] ) == FALSE )
//    {
        for( k=0; k<MAX_SENSORS; k++ )
        {
            coreDB.sensor[k].setup.sensor_type  = SENSOR_TYPE_NONE;
            coreDB.sensor[k].setup.offset       = 0;
        }
            
        // Write defaults to EEPROM. Configurations are
        //    stored in EEPROM for sensor IDs 1, 2, & 3 only.
        //    It is assumed that Sensor IDs 1,2,3 are contiguous.
//        ee_write_sensor_setup( &coreDB.sensor[SENSOR_ID_ONE] );
//    }        

//    if( !ee_read_sensor_name( coreDB.sensor[SENSOR_ID_ONE].name, SENSOR_ID_ONE ) )
//    {
        memset( coreDB.sensor[SENSOR_ID_ONE].name, SENSOR_NAME_LENGTH, 0 );
//        ee_write_sensor_name( coreDB.sensor[SENSOR_ID_ONE].name, SENSOR_ID_ONE );
//    }

//    if( !ee_read_sensor_name( coreDB.sensor[SENSOR_ID_TWO].name, SENSOR_ID_TWO ) )
//    {
        memset( coreDB.sensor[SENSOR_ID_TWO].name, SENSOR_NAME_LENGTH, 0 );
//        ee_write_sensor_name( coreDB.sensor[SENSOR_ID_TWO].name, SENSOR_ID_TWO );
//    }

//    if( !ee_read_sensor_name( coreDB.sensor[SENSOR_ID_THREE].name, SENSOR_ID_THREE ) )
//    {
        memset( coreDB.sensor[SENSOR_ID_THREE].name, SENSOR_NAME_LENGTH, 0 );
//        ee_write_sensor_name( coreDB.sensor[SENSOR_ID_THREE].name, SENSOR_ID_THREE );
//    }

//    if( !ee_read_sensor_name( coreDB.sensor[SENSOR_ID_DIFF].name, SENSOR_ID_DIFF ) )
//    {
        memset( coreDB.sensor[SENSOR_ID_DIFF].name, SENSOR_NAME_LENGTH, 0 );
//        ee_write_sensor_name( coreDB.sensor[SENSOR_ID_DIFF].name, SENSOR_ID_DIFF );
//    }


    // This element [SENSOR_ID_NONE] of the array is a special case.
    //    It represents the "None" sensor. As part of their setup,
    //    each output may reference one of the three sensors, or none.
    //    When an output reference sensor "none" it is Off, disabled.
    //
    coreDB.sensor[SENSOR_ID_NONE].setup.sensor_type = SENSOR_TYPE_NONE;
    coreDB.sensor[SENSOR_ID_NONE].setup.offset      = 0;
    coreDB.sensor[SENSOR_ID_NONE].value_int         = 0;
    coreDB.sensor[SENSOR_ID_NONE].value_float       = 0.0;
    coreDB.sensor[SENSOR_ID_NONE].fail              = 0;

    update_differential_sensor( &coreDB.sensor[0] );
    update_high_signal_sensor( &coreDB.sensor[0] );

    SecCounter = 0;

    // Prior to polling the expansion modules to determine what outputs
    //    exist in the system, read all of the output setup structures
    //    from EEPROM and initialize any problem data with defaults.
    //
    for( k=0; k<MAX_OUTPUTS; k++ )
    {
        coreDB.output[k].force_update = TRUE;
        coreDB.output[k].module_addr  = 0;
        coreDB.output[k].point_addr   = 0;

//        if( !ee_read_output_setup( &coreDB.output[k].setup, k ) )
//        {
            coreDB.output[k].setup.output_type = OUTPUT_TYPE_RELAY;
            load_default_setup( &coreDB.output[k], SENSOR_TYPE_NONE );        
//            ee_write_output_setup( &coreDB.output[k].setup, k );
//        }

//        if( !ee_read_output_name( coreDB.output[k].name, k ) )
//        {
            memset( coreDB.output[k].name, OUTPUT_NAME_LENGTH, 0 );
//            ee_write_output_name( coreDB.output[k].name, k );
//        }
    }

    System.num_outputs = 0;

    // Initialize the EeImage to match the core Database. Following this
    //    initialization this image will be managed by the UI_Task, which
    //    will be responsible for all EEPROM writes from this point forward.
    //
//    for( k=0; k<MAX_SENSORS; k++ )
//        EeImage.sens_setup[k] = coreDB.sensor[k].setup;

    // Only 4 of the sensors have "names" that are stored in EEPROM
    //    These include; Sn-1, Sn-2, Sn-3, Sn-d
    //
//    strcpy( &EeImage.sens_name[0][0], &coreDB.sensor[SENSOR_ID_ONE].name[0] );
//    strcpy( &EeImage.sens_name[1][0], &coreDB.sensor[SENSOR_ID_TWO].name[0] );
//    strcpy( &EeImage.sens_name[2][0], &coreDB.sensor[SENSOR_ID_THREE].name[0] );
//    strcpy( &EeImage.sens_name[3][0], &coreDB.sensor[SENSOR_ID_DIFF].name[0] );

//    for( k=0; k<MAX_OUTPUTS; k++ )
//    {
//        EeImage.out_setup[k] = coreDB.output[k].setup;
//        strcpy( &EeImage.out_name[k][0], &coreDB.output[k].name[0] );
//    }

    // Update the EE Image of the Ethernet Setup struct
//    EeImage.enet_setup = coreDB.enet_setup;
//    EeImage.dyndns     = coreDB.dyndns;

    // Update all copies of the data as used by the various tasks so
    //   that they are synchronized with the Core Database.
    //
    sensorDB  = coreDB;     // sensorDB is used by the Sensor Task
    controlDB = coreDB;     // controlDB is used by the Control Task
    uiDB      = coreDB;     // uiDB is used by the UI Task
    modbusDB  = coreDB;     // modbusDB is used by the Modbus Task
    enetDB    = coreDB;     // enetDB is used by the web server task
}
