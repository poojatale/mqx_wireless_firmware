/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : global.c

PURPOSE   : Contains the declaration of global variables.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#include "global.h"

//  The Lightweight Events for the Control and UI task are declared globally.
//  They are accessed by an interrupt service routine of the Periodic
//  interrupt timer (PIT0), which triggers execution of the respective
//  tasks. The hardware ISR causes the event masks to be set, MQX causes
//  the tasks to run it detects this event flag.
//
LWEVENT_STRUCT     eventControlTask;
LWEVENT_STRUCT     eventSensorTask;
LWEVENT_STRUCT     eventUiTask;
LWEVENT_STRUCT     eventModbusTask;

MUTEX_STRUCT       mutexCore;



int gRelayState; //1 = on, 0 = off




//
// DEBUG VARIABLES
//
PORT_MemMapPtr      ptrPA;
PORT_MemMapPtr      ptrPB;
PORT_MemMapPtr      ptrPC;
PORT_MemMapPtr      ptrPD;
PORT_MemMapPtr      ptrPE;
      
uint32_t            PA_PDDR;
uint32_t            PB_PDDR;
uint32_t            PC_PDDR;
uint32_t            PD_PDDR;
uint32_t            PE_PDDR;

ADC_MemMapPtr       PtrAdc0;
ADC_MemMapPtr       PtrAdc1;

SIM_MemMapPtr       ptrSim;

PMC_MemMapPtr       ptrPmc;
unsigned char       PmcLvdsc1;
unsigned char       PmcLvdsc2;
unsigned char       PmcRegsc;

unsigned char       ResetStatusRegHigh; // Reason for last reset, High Reg
unsigned char       ResetStatusRegLow;  //                        Low Reg
__no_init uint32_t  ResetCount;         // Count of the microcontroller resets

I2C_MemMapPtr       ptrI2c0;
I2C_MemMapPtr       ptrI2c1;
//
// END DEBUG VARIABLES
//


LED_REQUEST      CommLedRequest;

PLATFORM_INFO    TerminalInfo;   // Terminal board firmware and device info
PLATFORM_INFO    BaseInfo;       // Base board firmware and device info


uint64_t         TickPerSecond;
uint64_t         HwTickPerTick;
uint64_t         HwTickI2cTimeout;
MQX_TICK_STRUCT  TickStruct;

// I2C_INFO_STRUCT  I2cInfoStruct0;
// I2C_INFO_STRUCT  I2cInfoStruct1;

uint8_t           BlinkFlag;

DISPLAY_CONTENT  Display;

SYSTEM_STRUCT    System;
ENET_STATUS      EthernetStatus;

MODBUS_SETUP     ModbusSetup;  // Modbus Communication settings

// The Core Database; "coreDB" contains all of the setup and status data that
//   needs to be protected from corruption as might occur with shared
//   memory. Most of the tasks have a copy of the core database that they
//   use to synchronize with the core. The Core Database is protected by a
//   mutex. In cases where the Core Database is not immediately accessible,
//   the tasks will use their copy until such time as they can syncrhonize
//   with the core.
//
DATABASE         coreDB;        // Core Database - the gold standard
DATABASE         sensorDB;      // Copy maintained by the Sensor Task
DATABASE         controlDB;     // Copy maintained by the Control Task
DATABASE         uiDB;          // Copy maintained by the UI Task
DATABASE         modbusDB;      // Copy maintained by the Modbus Task
DATABASE         enetDB;        // Copy maintained by the Web Server


// The SensorTypeList is an array of the available sensors
//    a user can choose from when setting up the three
//    input sensors.
//
const char  SensorUnits[NUM_SENSOR_TYPES][8] = 
{
      " n/a",     // Sensor type is UNCONFIGURED, unused
      " F",       // A99 sensor,      -46 to 255,   units = degrees F
      " C",       // A99 sensor,    -43.0 to 124.0, units = degrees C
      " %rH",     // Rel. humidity,     1 to 100,   units = %RH
      " inwc",    // Pressure,      0.000 to 0.500, units = INWC
      " bar",     // Pressure,      -1.00 to 8.00,  units = bAR
      " inwc",    // Pressure,       0.00 to 10.00, units = INWC
      " bar",     // Pressure,       -1.0 to 15.0,  units = bAR
      " bar",     // Pressure,        0.0 to 30.0,  units = bAR
      " bar",     // Pressure,        0.0 to 50.0,  units = bAR
      " psi",     // Pressure,        0.0 to 100.0, units = PSI
      " psi",     // Pressure,          0 to 500,   units = PSI
      " psi",     // Pressure,          0 to 750,   units = PSI
      " psi",     // Pressure,          0 to 200,   units = PSI
      " inwc",    // Pressure,          0 to 2.5,   units = INWC
      " inwc",    // Pressure,          0 to 5.0,   units = INWC
      " F",       // Temperature,      70 to 330,   units = degrees F
      " C",       // Temperature,    21.0 to 165.0, units = degrees C
      " psi"      // Pressure,      -10.0 to 100.0, units = PSI
      "    "      // No units, binary input
};


// The SensorTypeList is an array of the available sensors
//    a user can choose from when setting up the three
//    input sensors.
//
const uint8_t  SensorTypeList[] = 
{
    SENSOR_TYPE_BINARY,    // Binary input, Open or Closed, no units
    SENSOR_TYPE_TEMP_HI_C, // High Temp sensor, -40.0 to 165, deg C
    SENSOR_TYPE_TEMP_HI_F, // High Temp sensor, -40   to 330, deg F
    SENSOR_TYPE_P750,      // Pressure,     0 to 750,   units = PSI
    SENSOR_TYPE_P500,      // Pressure,     0 to 500,   units = PSI
    SENSOR_TYPE_P200,      // Pressure,     0 to 200,   units = PSI
    SENSOR_TYPE_P110,      // Pressure, -10.0 to 100.0, units = PSI
    SENSOR_TYPE_P100,      // Pressure,   0.0 to 100.0, units = PSI
    SENSOR_TYPE_P_50,      // Pressure,   0.0 to 50.0,  units = bAR
    SENSOR_TYPE_P_30,      // Pressure,   0.0 to 30.0,  units = bAR
    SENSOR_TYPE_P_15,      // Pressure,  -1.0 to 15.0,  units = bAR
    SENSOR_TYPE_P_10,      // Pressure,  0.00 to 10.00, units = INWC
    SENSOR_TYPE_P__8,      // Pressure, -1.00 to 8.00,  units = bAR
    SENSOR_TYPE_P_5,       // Pressure,  0.00 to 5.00,  units = INWC
    SENSOR_TYPE_P_2pt5,    // Pressure,  0.00 to 2.50,  units = INWC
    SENSOR_TYPE_P_0pt5,    // Pressure, 0.000 to 0.500, units = INWC
    SENSOR_TYPE_P_0pt25,   // Pressure, -0.250 to 0.250, units = INWC
    SENSOR_TYPE_RH,        // Relative humidity, 1 to 100, units = %RH
    SENSOR_TYPE_TEMP_C,    // A99 sensor, -43.0 to 124, units = degrees C
    SENSOR_TYPE_TEMP_F,    // A99 sensor, -46   to 255, units = degrees F
    SENSOR_TYPE_NONE       // Sensor type is UNCONFIGURED, unused
};


// The SensorFailList is an array of the available 
//    choices a user can choose from when setting up the
//    sensor failure action for an output.
//
const uint8_t  SensorFailList[] =
{
    SENSOR_FAIL_ON,         // Turn output ON upon sensor failure
    SENSOR_FAIL_OFF         // Turn output OFF upon sensor failure
};

// The EnetAddrModeList is an array of the available status
//    indicators of the C450's DHCP server.
//
const uint8_t  EnetAddrModeList[] =
{
    ENET_ADDR_MODE_DIRECT,     // C450 acts as the DHCP server (direct connect)
    ENET_ADDR_MODE_STATIC_IP,  // C450 uses a static IP address
    ENET_ADDR_MODE_AUTO        // C450 gets its IP from a DHCP server
};

// The ModbusBaudList is an array of the available choices
//    a user can choose from when setting up the baud rate
//    for the Modbus protocol.
//
const uint8_t  ModbusBaudList[] =
{
    BAUD_9600,       // Baud Rate = 1200 bps
    BAUD_19200,      // Baud Rate = 19.2 kbps
    BAUD_38400       // Baud Rate = 38.4 kbps
};


// The RelayParamList is an array of the parameters 
//    associated with setting up a Relay type output.
//
const uint8_t  RelayParamList[] =
{
    PARAM_ID_CUTON,          // Cut-On point of a relay Output displayed
    PARAM_ID_CUTOFF,         // Cut-Off point of a relay Output displayed
    PARAM_ID_ON_DELAY,       // On Delay time of a relay Output displayed
    PARAM_ID_OFF_DELAY,      // Off Delay time of a relay Output displayed
    PARAM_ID_MINON,          // Minimum On time of a relay Output displayed
    PARAM_ID_MINOFF,         // Minimum Off time of a relay Output displayed
    PARAM_ID_SENSOR_FAILURE, // Action to take upon Sensor Failure
    PARAM_ID_SEL_SENSOR      // Sensor used by a Output is displayed
};

// The BinaryParamList is an array of the parameters used when setting
//    setting up a Relay type output that references a Binary sensor.
//
const uint8_t  BinaryParamList[] =
{
    PARAM_ID_ON_DELAY,       // On Delay time of a relay Output displayed
    PARAM_ID_OFF_DELAY,      // Off Delay time of a relay Output displayed
    PARAM_ID_MINON,          // Minimum On time of a relay Output displayed
    PARAM_ID_MINOFF,         // Minimum Off time of a relay Output displayed
    PARAM_ID_SEL_SENSOR      // Sensor used by a Output is displayed
};

// The DiffRelayParamList is an array of the parameters associated
//     with a Relay type output that uses the differential sensor.
//
const unsigned char  DiffRelayParamList[] =
{
    PARAM_ID_DIFF_CUTON,     // Differential Cut-On point of a relay Output
    PARAM_ID_DIFF_CUTOFF,    // Differential Cut-Off point of a relay Output 
    PARAM_ID_ON_DELAY,       // On Delay time of a relay Output displayed
    PARAM_ID_OFF_DELAY,      // Off Delay time of a relay Output displayed
    PARAM_ID_MINON,          // Minimum On time of a relay Output 
    PARAM_ID_MINOFF,         // Minimum Off time of a relay Output 
    PARAM_ID_SENSOR_FAILURE, // Action to take upon Sensor Failure
    PARAM_ID_SEL_SENSOR      // Sensor used by a Output
};

// The AnalogParamList is an array of the parameters 
//    associated with setting up an Analog type output.
//
const uint8_t  AnalogParamList[] =
{
    PARAM_ID_SP,             // Setpoint of analog output
    PARAM_ID_EP,             // Endpoint of analog output
    PARAM_ID_OUTPUT_SP,      // Control output (0-100%) when sensor = SP
    PARAM_ID_OUTPUT_EP,      // Control output (0-100%) when sensor = EP
    PARAM_ID_INT_CONSTANT,   // Integration Constant
    PARAM_ID_UPDATE_RATE,    // Output Update Rate - analog module
    PARAM_ID_OUTPUT_BAND,    // Output Band (deadband) - analog module
    PARAM_ID_SENSOR_FAILURE, // Action to take upon Sensor Failure
    PARAM_ID_SEL_SENSOR      // Sensor used by a Output is displayed
};

// The DiffAnalogParamList is an array of the parameters associated with
//     setting up an Analog type output that uses the differential sensor.
//
const unsigned char  DiffAnalogParamList[] =
{
    PARAM_ID_DIFF_SP,        // Setpoint of analog output
    PARAM_ID_DIFF_EP,        // Endpoint of analog output
    PARAM_ID_OUTPUT_SP,      // Control output (0-100%) when sensor = SP
    PARAM_ID_OUTPUT_EP,      // Control output (0-100%) when sensor = EP
    PARAM_ID_INT_CONSTANT,   // Integration Constant
    PARAM_ID_UPDATE_RATE,    // Output Update Rate - analog module
    PARAM_ID_OUTPUT_BAND,    // Output Band (deadband) - analog module
    PARAM_ID_SENSOR_FAILURE, // Action to take upon Sensor Failure
    PARAM_ID_SEL_SENSOR      // Sensor used by a Output is displayed
};
// DefaultRelaySetup[] is an array of default setup 
//    values for a Relay type output. There are unique
//    default values for each of the sensor types. Each
//    element in the array corresponds to a specific
//    sensor type.
//
const RELAY_SETUP  DefaultRelaySetup[] =
{ // Sensor ID,    Sensor Fail Mode, On, Off, On Delay, Off Delay, Min On, Min Off
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  110,  100, 0, 0, 0, 0},  // SENSOR_TYPE_NONE
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  110,  100, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  450,  400, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   70,   50, 0, 0, 0, 0},  // SENSOR_TYPE_RH
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  150,  125, 0, 0, 0, 0},  // SENSOR_TYPE_P_0pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  470,  450, 0, 0, 0, 0},  // SENSOR_TYPE_P__8
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  520,  500, 0, 0, 0, 0},  // SENSOR_TYPE_P_10
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   90,   70, 0, 0, 0, 0},  // SENSOR_TYPE_P_15
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  190,  150, 0, 0, 0, 0},  // SENSOR_TYPE_P_30
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  290,  250, 0, 0, 0, 0},  // SENSOR_TYPE_P_50
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  700,  500, 0, 0, 0, 0},  // SENSOR_TYPE_P100
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  350,  300, 0, 0, 0, 0},  // SENSOR_TYPE_P500
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  500,  450, 0, 0, 0, 0},  // SENSOR_TYPE_P750
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   95,   65, 0, 0, 0, 0},  // SENSOR_TYPE_P200
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  120,  100, 0, 0, 0, 0},  // SENSOR_TYPE_P_2pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  250,  200, 0, 0, 0, 0},  // SENSOR_TYPE_P_5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  220,  200, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_HI_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF, 1100, 1000, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_HI_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  700,  500, 0, 0, 0, 0},  // SENSOR_TYPE_P110
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   10,    0, 0, 0, 0, 0},  // SENSOR_TYPE_BINARY
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  150,  125, 0, 0, 0, 0}   // SENSOR_TYPE_P_0pt25
};

// DefaultDiffRelaySetup[] is an array of default setup values for a
//    Relay type output. There are unique default values for each of
//    the sensor types. Each element in the array corresponds to
//    a specific sensor type.
//
const RELAY_SETUP  DefaultDiffRelaySetup[] =
{ // Sensor ID,    Sensor Fail Mode, On, Off, On Delay, Off Delay, Min On, Min Off
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  10, 0, 0, 0, 0},  // SENSOR_TYPE_NONE
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  10, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  50, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  10, 0, 0, 0, 0},  // SENSOR_TYPE_RH
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  25, 0, 0, 0, 0},  // SENSOR_TYPE_P_0pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  40, 0, 0, 0, 0},  // SENSOR_TYPE_P__8
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  50, 0, 0, 0, 0},  // SENSOR_TYPE_P_10
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,   7, 0, 0, 0, 0},  // SENSOR_TYPE_P_15
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  15, 0, 0, 0, 0},  // SENSOR_TYPE_P_30
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  24, 0, 0, 0, 0},  // SENSOR_TYPE_P_50
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  50, 0, 0, 0, 0},  // SENSOR_TYPE_P100
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  25, 0, 0, 0, 0},  // SENSOR_TYPE_P500
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  50, 0, 0, 0, 0},  // SENSOR_TYPE_P750
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  10, 0, 0, 0, 0},  // SENSOR_TYPE_P200
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  24, 0, 0, 0, 0},  // SENSOR_TYPE_P_2pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  50, 0, 0, 0, 0},  // SENSOR_TYPE_P_5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  10, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_HI_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  50, 0, 0, 0, 0},  // SENSOR_TYPE_TEMP_HI_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  50, 0, 0, 0, 0},  // SENSOR_TYPE_P110
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  10, 0, 0, 0, 0},  // SENSOR_TYPE_BINARY
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   0,  25, 0, 0, 0, 0}   // SENSOR_TYPE_P_0pt25
};


// DefaultAnalogSetup[] is an array of default setup 
//    values for an Analog type output. There are unique
//    default values for each of the sensor types. Each
//    element in the array corresponds to a specific
//    sensor type.
//
const ANALOG_SETUP  DefaultAnalogSetup[] =
{ // Sensor ID, Sensor Fail Mode, sp, ep, sp_output, ep_output, int_constant, period, output band, unused
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  100,  110, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_NONE
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  100,  110, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  400,  450, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   50,   70, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_RH
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  125,  150, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_0pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  450,  470, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P__8
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  500,  520, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_10
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   70,   90, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_15
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  150,  190, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_30
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  250,  290, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_50
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  500,  700, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P100
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  300,  350, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P500
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  450,  500, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P750
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   65,   95, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P200
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  100,  120, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_2pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  200,  250, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  200,  220, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_HI_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF, 1000, 1100, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_HI_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  500,  700, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P110
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   10,    0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_BINARY
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  125,  150, 0, 100, 0, 1, 0, 0}   // SENSOR_TYPE_P_0pt25
};

// DefaultDiffAnalogSetup[] is an array of default setup values for an
//    Analog type output. There are unique default values for each
//    of the sensor types. Each element in the array corresponds to
//    a specific sensor type.
//
const ANALOG_SETUP  DefaultDiffAnalogSetup[] =
{ // Sensor ID, Sensor Fail Mode, sp, ep, sp_output, ep_output, int_constant, period, output band, unused
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  10,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_NONE
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  10,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  50,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  10,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_RH
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  25,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_0pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  40,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P__8
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  50,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_10
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,   7,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_15
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  15,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_30
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  24,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_50
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  50,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P100
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  25,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P500
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  50,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P750
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  10,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P200
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  24,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_2pt5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  50,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P_5
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  10,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_HI_F
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  50,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_TEMP_HI_C
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  50,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_P110
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  10,   0, 0, 100, 0, 1, 0, 0},  // SENSOR_TYPE_BINARY
  {SENSOR_ID_NONE, SENSOR_FAIL_OFF,  25,   0, 0, 100, 0, 1, 0, 0}   // SENSOR_TYPE_P_0pt25
};

// MinimumSetpoint[] is an array of setpoints, representing
//    the lowest value permitted. There are unique values
//    for each of the sensor types. Each element in the
//    array corresponds to a specific sensor type.
//
const int16_t MinimumSetpoint[] = {   0,   // SENSOR_TYPE_NONE
                                   -40,   // SENSOR_TYPE_TEMP_F,   -40    deg F
                                  -400,   // SENSOR_TYPE_TEMP_C,   -40.0  deg C
                                    10,   // SENSOR_TYPE_RH,        10%   rH
                                    25,   // SENSOR_TYPE_P_0pt5,   0.025  INWC
                                  -100,   // SENSOR_TYPE_P__8,     -1.00  BAR
                                    50,   // SENSOR_TYPE_P_10,      0.50  INWC
                                   -10,   // SENSOR_TYPE_P_15,     -1.0   BAR
                                     0,   // SENSOR_TYPE_P_30,      0.0   BAR
                                     0,   // SENSOR_TYPE_P_50,      0.0   BAR
                                     0,   // SENSOR_TYPE_P100,      0.0   PSI
                                    90,   // SENSOR_TYPE_P500,       90   PSI
                                   150,   // SENSOR_TYPE_P750,      150   PSI
                                     0,   // SENSOR_TYPE_P200,        0   PSI
                                    10,   // SENSOR_TYPE_P_2pt5,    0.10  INWC
                                    25,   // SENSOR_TYPE_P_5,       0.25  INWC
                                   -40,   // SENSOR_TYPE_TEMP_HI_F, -40   deg F
                                  -400,   // SENSOR_TYPE_TEMP_HI_C, -40.0 deg C
                                  -100,   // SENSOR_TYPE_P110,      -10.0 PSI
                                     0,   // SENSOR_TYPE_BINARY,      0.0 n/a
                                  -225 }; // SENSOR_TYPE_P_0pt25, -0.225  INWC

// MinDiffSetpoint[] is an array of setpoints, representing the lowest
//    value permitted. There are unique values for each of the sensor types.
//    Each element in the array corresponds to a specific sensor type.
//
const int16_t MinDiffSetpoint[] = {   0,   // SENSOR_TYPE_NONE
                                  -290,   // SENSOR_TYPE_TEMP_F,    -290   deg F
                                 -1610,   // SENSOR_TYPE_TEMP_C,    -161.0 deg C
                                   -95,   // SENSOR_TYPE_RH,        -95%   rH
                                  -500,   // SENSOR_TYPE_P_0pt5,    -0.500 INWC
                                  -900,   // SENSOR_TYPE_P__8,      -9.00  BAR
                                 -1000,   // SENSOR_TYPE_P_10,     -10.00  INWC
                                  -160,   // SENSOR_TYPE_P_15,      -16.0  BAR
                                  -300,   // SENSOR_TYPE_P_30,      -30.0  BAR
                                  -500,   // SENSOR_TYPE_P_50,      -50.0  BAR
                                 -1000,   // SENSOR_TYPE_P100,     -100.0  PSI
                                  -500,   // SENSOR_TYPE_P500,      -500   PSI
                                  -750,   // SENSOR_TYPE_P750,      -750   PSI
                                  -200,   // SENSOR_TYPE_P200,      -200   PSI
                                  -250,   // SENSOR_TYPE_P_2pt5,    -2.50  INWC
                                  -500,   // SENSOR_TYPE_P_5,       -5.00  INWC
                                  -380,   // SENSOR_TYPE_TEMP_HI_F, -380   deg F
                                 -2100,   // SENSOR_TYPE_TEMP_HI_C, -210.0 deg C
                                 -1100,   // SENSOR_TYPE_P110,      -110.0 PSI
                                     0,   // SENSOR_TYPE_BINARY,       0.0 n/a
                                  -500 }; // SENSOR_TYPE_P_0pt25,   -0.500 INWC

// MaximumSetpoint[] is an array of setpoints, representing
//    the largest value permitted. There are unique values
//    for each of the sensor types. Each element in the
//    array corresponds to a specific sensor type.
//
const int16_t MaximumSetpoint[] = {   0,   // SENSOR_TYPE_NONE
                                   250,   // SENSOR_TYPE_TEMP_F,    250   deg F
                                  1210,   // SENSOR_TYPE_TEMP_C,    121.0 deg C
                                    95,   // SENSOR_TYPE_RH,         95%  rH
                                   500,   // SENSOR_TYPE_P_0pt5,    0.500  INWC
                                   800,   // SENSOR_TYPE_P__8,      8.00  BAR
                                  1000,   // SENSOR_TYPE_P_10,     10.00  INWC
                                   150,   // SENSOR_TYPE_P_15,      15.0  BAR
                                   300,   // SENSOR_TYPE_P_30,      30.0  BAR
                                   500,   // SENSOR_TYPE_P_50,      50.0  BAR
                                  1000,   // SENSOR_TYPE_P100,      100.0 PSI
                                   500,   // SENSOR_TYPE_P500,      500   PSI
                                   750,   // SENSOR_TYPE_P750,      750   PSI
                                   200,   // SENSOR_TYPE_P200,      200   PSI
                                   250,   // SENSOR_TYPE_P_2pt5,    2.50  INWC
                                   500,   // SENSOR_TYPE_P_5,       5.00  INWC
                                   350,   // SENSOR_TYPE_TEMP_HI_F, 350   deg F
                                  1760,   // SENSOR_TYPE_TEMP_HI_C, 176.0 deg C
                                  1000,   // SENSOR_TYPE_P110,      100.0 PSI
                                     1,   // SENSOR_TYPE_BINARY,      1.0 n/a
                                   250 }; // SENSOR_TYPE_P_0pt25,   0.250  INWC

// MinimumDifferential[] is an array of values representing
//    the minimum span between setpoint (Cut-On\Cut-Off for
//    Relay outputs, SP\EP for Analog output). There are 
//    unique values for each of the sensor types. Each element
//    in the array corresponds to a specific sensor type.
//
const int16_t MinimumDifferential[] = { 0,   // SENSOR_TYPE_NONE,   0
                                       1,   // SENSOR_TYPE_TEMP_F, 1     deg F
                                       5,   // SENSOR_TYPE_TEMP_C, 0.5   deg C
                                       2,   // SENSOR_TYPE_RH,     2%    rH
                                      10,   // SENSOR_TYPE_P_0pt5, 0.010 INWC 
                                      10,   // SENSOR_TYPE_P__8,   0.1   BAR
                                      20,   // SENSOR_TYPE_P_10,   0.2   INWC
                                       2,   // SENSOR_TYPE_P_15,   0.2   BAR
                                       4,   // SENSOR_TYPE_P_30,   0.4   BAR
                                       4,   // SENSOR_TYPE_P_50,   0.4   BAR
                                      10,   // SENSOR_TYPE_P100,   1     PSI
                                       5,   // SENSOR_TYPE_P500,   5     PSI
                                       6,   // SENSOR_TYPE_P750,   6     PSI
                                       1,   // SENSOR_TYPE_P200,   1     PSI
                                      10,   // SENSOR_TYPE_P_2pt5, 0.10  INWC
                                      25,   // SENSOR_TYPE_P_5,    0.25  INWC
                                       1,   // SENSOR_TYPE_TEMP_HI_F, 1   deg F
                                       5,   // SENSOR_TYPE_TEMP_HI_C, 0.5 deg C
                                      10,   // SENSOR_TYPE_P110,      1   PSI
                                       1,   // SENSOR_TYPE_BINARY,    1.0 n/a
                                      10 }; // SENSOR_TYPE_P_0pt25, 0.010 INWC 


// Template[] is an array of values describe a "screen", a
//    specific display, on the LCD. Contained in the template
//    are the prompt, and an indication as to how some of the
//    display fields are used. A template provides a general
//    outline regarding control of a specific user interface screen.
//
// Each time a new screen is loaded, a new template is referenced.
//
const SCREEN_TEMPLATE Template[] = 
{ 
  // TEMPLATE_STATUS - Main Status Display
  {{' ', ' ', ' ', ' '}, SCREEN_TYPE_STATUS,       ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_OUTPUT_STATUS_MENU - Output Status Menu Prompt
  {{'O', 'U', 'T', 'P'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_SENSOR_MENU - Sensor Setup Menu Prompt
  {{'S', 'E', 'N', 'S'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_OUTPUT_SETUP_R_MENU - Relay Output Setup Menu Prompt
  {{'O', 'U', 'T', 'R'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_OUTPUT_SETUP_A_MENU - Analog Output Setup Menu Prompt
  {{'O', 'U', 'T', 'A'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_SYSTEM_MENU - System Setup Menu Prompt
  {{'S', 'Y', 'S', ' '}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_USER_MENU - User Password Setup Menu Prompt
  {{'U', 'S', 'E', 'R'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_ADMIN_MENU - Admin Password Setup Menu Prompt
  {{'A', 'd', 'M', 'N'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_MODBUS_MENU - Modbus Network Setup Menu Prompt
  {{'C', 'O', 'M', 'M'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_ENET_MENU - Ethernet Network Setup Menu Prompt
  {{'C', 'O', 'M', 'M'}, SCREEN_TYPE_SUBMENU,      ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_SENSOR_STATUS - Sensor Status Display
  {{' ', ' ', ' ', ' '}, SCREEN_TYPE_STATUS,       ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_DIFF_SENSOR_STATUS - Differential Sensor Status Display
  {{'d', 'I', 'F', 'F'}, SCREEN_TYPE_STATUS,       ITEM_NUM_NONE,         LIST_NONE},

  // TEMPLATE_OUTPUT_STATUS - Output Status Display
  {{' ', 'O', 'U', 'T'}, SCREEN_TYPE_STATUS,       ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_SENSOR_TYPE - Edit Sensor Type
  {{'S', 'n', '-', ' '}, SCREEN_TYPE_SELECT_LIST,  ITEM_NUM_SENSOR_ALPHA, LIST_SENSOR_TYPE},

  // TEMPLATE_CUT_ON - Edit On Point - relay control, Cut-On
  {{' ', ' ', 'O', 'N'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_CUT_OFF - Edit Off Point - relay control, Cut-Off
  {{' ', 'O', 'F', 'F'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_DIFF_CUT_ON - Edit Differential On Point - relay control, Diff Cut-On
  {{' ', 'd', 'O', 'N'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_DIFF_CUT_OFF - Edit Differential Off Point - relay control, Diff Cut-Off
  {{'d', 'O', 'F', 'F'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_ON_DELAY - Edit On Delay Time - relay control
  {{' ', 'O', 'N', 'd'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_OFF_DELAY - Edit Off Delay Time - relay control
  {{'O', 'F', 'F', 'd'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_ON_TIME - Edit Minimum On Time - relay control
  {{' ', 'O', 'N', 'T'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_OFF_TIME - Edit Minimum Off Time - relay control
  {{'O', 'F', 'F', 'T'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_SEL_SENSOR - Edit Sensor ID, controlling sensor - relay & analog control
  {{'S', 'E', 'N', 'S'}, SCREEN_TYPE_SELECT_LIST,  ITEM_NUM_OUTPUT_RIGHT, LIST_SENSOR},

  // TEMPLATE_SP - Edit Setpoint - analog control
  {{' ', ' ', 'S', 'P'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_EP - Edit End Point - analog control
  {{' ', ' ', 'E', 'P'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_DIFF_SP - Edit Differential Setpoint - analog control
  {{' ', 'd', 'S', 'P'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_DIFF_EP - Edit Differential End Point - analog control
  {{' ', 'd', 'E', 'P'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_INT_CONSTANT - Edit Integration Constant - analog control
  {{' ', 'I', '-', 'C'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_OUTPUT_SP - Edit Setpoint Output Level - analog control
  {{' ', 'O', 'S', 'P'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_OUTPUT_EP - Edit End Point Output Level - analog control
  {{' ', 'O', 'E', 'P'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_UPDATE_RATE - Edit Output Update Rate - Analog output
  {{'U', 'P', '-', 'R'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_OUTPUT_BAND - Edit Output Band - Analog output
  {{' ', 'b', 'N', 'd'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_OUTPUT_RIGHT, LIST_NONE},

  // TEMPLATE_SENSOR_FAILURE - Edit Sensor Fail Mode - relay & analog control
  {{' ', 'S', 'N', 'F'}, SCREEN_TYPE_SELECT_LIST,  ITEM_NUM_OUTPUT_RIGHT, LIST_SENSOR_FAILURE},

  // TEMPLATE_SENSOR_OFFSET - Edit Sensor Offset - Temperature Sensors Only
  {{'O', 'F', 'F', 'S'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_SENSOR_RIGHT, LIST_NONE},

  // TEMPLATE_NODE_ADDRESS - Edit Node Address - RS485 communication
  {{'A', 'd', 'd', 'r'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_BAUD_RATE - Edit Node Address - RS485 communication
  {{'b', 'A', 'U', 'd'}, SCREEN_TYPE_SELECT_LIST, ITEM_NUM_NONE, LIST_BAUD_RATE},

  // TEMPLATE_PARITY - Edit Parity - RS485 communication
  {{' ', 'P', 'A', 'R'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_STOP_BITS - Edit Stop Bits - RS485 communication
  {{'S', 'T', 'O', 'P'}, SCREEN_TYPE_EDIT_NUMERIC, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_REMOTE_LOCKOUT - Edit Remote Lockout, ethernet
  {{'R', 'L', 'C', 'K'}, SCREEN_TYPE_SELECT_LIST, ITEM_NUM_NONE, LIST_REMOTE_LOCKOUT},

  // TEMPLATE_ENET_IP_1 - Edit Ethernet IP Address 1 - ie. IP1.255.255.255
  {{'I', 'P', '-', '1'}, SCREEN_TYPE_STATUS_NUMERIC, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_ENET_IP_2 - Edit Ethernet IP Address 2 - ie. 255.IP2.255.255
  {{'I', 'P', '-', '2'}, SCREEN_TYPE_STATUS_NUMERIC, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_ENET_IP_3 - Edit Ethernet IP Address 3 - ie. 255.255.IP3.255
  {{'I', 'P', '-', '3'}, SCREEN_TYPE_STATUS_NUMERIC, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_ENET_IP_4 - Edit Ethernet IP Address 4 - ie. 255.255.255.IP4
  {{'I', 'P', '-', '4'}, SCREEN_TYPE_STATUS_NUMERIC, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_ENET_ADDR_MODE - Display Static IP, DHCP Client, Direct Connect
  {{'M', 'o', 'd', 'E'}, SCREEN_TYPE_STATUS_LIST, ITEM_NUM_NONE, LIST_ENET_ADDR_MODE},

  // TEMPLATE_ENET_IP_RESET - Display "Reset IP" Prompt
  {{' ', 'I', 'P', ' '}, SCREEN_TYPE_STATUS_LIST, ITEM_NUM_NONE, LIST_ENET_RESET},

  // TEMPLATE_PASSWORD - Display "PW" Prompt
  {{' ', 'P', 'W', ' '}, SCREEN_TYPE_PASSWORD, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_ADMIN_PASSWORD - Display "PW-A" Prompt
  {{'A', 'd', 'P', 'W'}, SCREEN_TYPE_PASSWORD, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_EDIT_USER - Display "USER" Prompt
  {{'U', 'S', 'E', 'R'}, SCREEN_TYPE_PASSWORD, ITEM_NUM_1, LIST_NONE},

  // TEMPLATE_EDIT_ADMIN_1 - Display "AdMN" Prompt and "1"
  {{'A', 'd', 'M', 'N'}, SCREEN_TYPE_PASSWORD, ITEM_NUM_1, LIST_NONE},

  // TEMPLATE_ADMIN_PASSWORD_2 - Display "AdMN" Prompt and "2"
  {{'A', 'd', 'M', 'N'}, SCREEN_TYPE_PASSWORD, ITEM_NUM_2, LIST_NONE},

  // TEMPLATE_MESSAGE_FAIL - Error message, empty prompt overwritten w/ error msg
  {{'F', 'A', 'I', 'L'}, SCREEN_TYPE_MESSAGE, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_MESSAGE_OK - Error message, empty prompt overwritten w/ error msg
  {{' ', 'O', 'K', ' '}, SCREEN_TYPE_MESSAGE, ITEM_NUM_NONE, LIST_NONE},

  // TEMPLATE_ERROR - Error message, empty prompt overwritten w/ error msg
  {{' ', ' ', ' ', ' '}, SCREEN_TYPE_STATUS, ITEM_NUM_NONE, LIST_NONE}
};


SCREEN      Screen;     // This data structure contains a lot of information
                        //    regarding the current display screen. 
                        //    Included is a pointer to a particular
                        //    output, since many of the screens provide
                        //    a user interface for setting up the outputs.

uint8_t      SensorList[ MAX_SENSORS ];  // The Sensor list contains the
                                        //    available sensors, used
                                        //    when setting up an output.

PARAM_LIST  ParamList;   // The ParamList is used to hold the list
                         //    of parameters that can be edited, including
                         //    parameters related to an output setup,
                         //    and sensor setup.

uint32_t     SecCounter;  // The SecCounter is a running count of seconds
                         //    that have transpired since reset. It can
                         //    be used as needed by the application code.
                         // NOTE! The SecCounter simply rolls over when
                         //       it reaches 0xFFFFFFFF.                          

uint8_t      ModelType;   // Indicates the model type of this
                         //     control module.

CALIBRATION  CalData;
                               
PC_CONNECT   PC;

ERROR_CONDITIONS Error;


// The following CRC table is used in the calculation of the 
//    8-bit CRC, contained in the I2C msgs between the control
//    and expansion modules.
//
const uint8_t CRC_8_Table[256] =
{
    0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53,  // CRC 00 - 07
    0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,  // CRC 08 - 0F
    0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E,  // CRC 10 - 17
    0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,  // CRC 18 - 1F
    0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4,  // CRC 20 - 27
    0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,  // CRC 28 - 2F
    0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19,  // CRC 30 - 37
    0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,  // CRC 38 - 3F
    0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40,  // CRC 40 - 47
    0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,  // CRC 48 - 4F
    0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D,  // CRC 50 - 57
    0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,  // CRC 58 - 5F
    0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7,  // CRC 60 - 67
    0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,  // CRC 68 - 6F
    0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A,  // CRC 70 - 77
    0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,  // CRC 78 - 7F
    0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75,  // CRC 80 - 87
    0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,  // CRC 88 - 8F
    0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8,  // CRC 90 - 97
    0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,  // CRC 98 - 9F
    0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2,  // CRC A0 - A7
    0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,  // CRC A8 - AF
    0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F,  // CRC B0 - B7
    0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,  // CRC B8 - BF
    0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66,  // CRC C0 - C7
    0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,  // CRC C8 - CF
    0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB,  // CRC D0 - D7
    0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,  // CRC D8 - DF
    0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1,  // CRC E0 - E7
    0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,  // CRC E8 - EF
    0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C,  // CRC F0 - F7
    0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4   // CRC F8 - FF
};
                        