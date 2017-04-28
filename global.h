/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : global.h

PURPOSE   : "extern" declarations of the global variables defined in 
            global.c. Source files that reference the global variables
            need to include this header file.                

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __global_inc
#define  __global_inc

#include <mqx.h>
#include <mutex.h>
#include <i2c.h>
#include "system450.h"
#include "defines.h"
//#include "lcd.h"
#include "Sensor_Task.h"
//#include "i2c_k60.h"


extern LWEVENT_STRUCT     eventControlTask;
extern LWEVENT_STRUCT     eventSensorTask;
extern LWEVENT_STRUCT     eventUiTask;
#ifdef RS485_HARDWARE
extern LWEVENT_STRUCT     eventModbusTask;
#endif

extern MUTEX_STRUCT       mutexCore;

extern int gRelayState;



// DEBUG VARIABLES
extern  PORT_MemMapPtr ptrPA;
extern  PORT_MemMapPtr ptrPB;
extern  PORT_MemMapPtr ptrPC;
extern  PORT_MemMapPtr ptrPD;
extern  PORT_MemMapPtr ptrPE;

extern  uint32_t       PA_PDDR;
extern  uint32_t       PB_PDDR;
extern  uint32_t       PC_PDDR;
extern  uint32_t       PD_PDDR;
extern  uint32_t       PE_PDDR;

extern  ADC_MemMapPtr  PtrAdc0;
extern  ADC_MemMapPtr  PtrAdc1;

extern  SIM_MemMapPtr  ptrSim;

extern  PMC_MemMapPtr  ptrPmc;    // Power Management Register, doesn't work with watch
extern  unsigned char  PmcLvdsc1;
extern  unsigned char  PmcLvdsc2;
extern  unsigned char  PmcRegsc;

extern  unsigned char  ResetStatusRegHigh;
extern  unsigned char  ResetStatusRegLow;
extern  uint32_t        ResetCount;

extern  I2C_MemMapPtr  ptrI2c0;
extern  I2C_MemMapPtr  ptrI2c1;
// END DEBUG VARIABLES


extern       SAMPLE_STRUCT     Sample[MAX_ANA_INPUTS];

extern const char              SensorUnits[NUM_SENSOR_TYPES][8];

extern       LED_REQUEST       CommLedRequest;
extern       PLATFORM_INFO     TerminalInfo;  // Terminal board fw, device info
extern       PLATFORM_INFO     BaseInfo;      // Base board fw, device info

extern       uint64_t          TickPerSecond;
extern       uint64_t          HwTickPerTick;
extern       uint64_t          HwTickI2cTimeout;
extern       MQX_TICK_STRUCT   TickStruct;

//extern       I2C_INFO_STRUCT   I2cInfoStruct0;
//extern       I2C_INFO_STRUCT   I2cInfoStruct1;


extern const uint8_t         CRC_8_Table[];

extern const RELAY_SETUP     DefaultRelaySetup[];      // Default Relay setup
extern const RELAY_SETUP     DefaultDiffRelaySetup[];  // Default Diff-Relay setup
extern const ANALOG_SETUP    DefaultAnalogSetup[];     // Default Analog setup
extern const ANALOG_SETUP    DefaultDiffAnalogSetup[]; // Default Diff-Analog setup
extern const int16_t         MinimumSetpoint[];        // Min Setpt, per sensor type
extern const int16_t         MinDiffSetpoint[];        // Min Differential Setpt, per sensor type
extern const int16_t         MaximumSetpoint[];        // Max Setpt, per sensor type
extern const int16_t         MinimumDifferential[];    // Min Diff, per sensor type

// This global variable is used solely by LCD routines, so it is declared
//     here. It is used to blink parameters available for edit.
//
extern       uint8_t         BlinkFlag;

extern       DISPLAY_CONTENT Display;

extern       SYSTEM_STRUCT   System;            // General System Information

extern       ENET_STATUS     EthernetStatus;

extern       MODBUS_SETUP    ModbusSetup;       // Modbus setup data

// The Core Database; "coreDB" contains all of the setup and status data that
//   needs to be protected from corruption as might occur with shared
//   memory. Most of the tasks have a copy of the core database that they
//   use to synchronize with the core. The Core Database is protected by a
//   mutex. In cases where the Core Database is not immediately accessible,
//   the tasks will use their copy until such time as they can syncrhonize
//   with the core.
//
extern       DATABASE        coreDB;
extern       DATABASE        sensorDB;
extern       DATABASE        controlDB;
extern       DATABASE        uiDB;
extern       DATABASE        modbusDB;
extern       DATABASE        enetDB;

extern const uint8_t          SensorTypeList[];  // List of available sensor types
extern       uint8_t          SensorList[];      // List of actual sensors
extern const uint8_t          CtrlModeList[];    
extern const uint8_t          SensorFailList[];  // List of sensor failure modes
extern const uint8_t          EnetAddrModeList[];// List of Enet address modes
extern const uint8_t          ModbusBaudList[];  // List of Modbus Baud Rates

extern const uint8_t          RelayParamList[];  // List of Relay setup params
extern const uint8_t          BinaryParamList[];
extern const uint8_t          DiffRelayParamList[];  // List of Differential Relay params
extern const uint8_t          AnalogParamList[];     // List of Analog setup params
extern const uint8_t          DiffAnalogParamList[]; // List of Differential Analog params
extern       PARAM_LIST      ParamList;         // Current parameter list

extern       SCREEN          Screen;            // Screen content, UI information
extern const SCREEN_TEMPLATE Template[];        // Screen outline, basic behavior

extern       uint32_t         SecCounter;        // The running count of seconds

extern       uint8_t          ModelType;         // Indicates the model type of this
                                                //     control module.
extern       CALIBRATION     CalData;

extern       PC_CONNECT      PC;

extern       ERROR_CONDITIONS Error;


#endif
