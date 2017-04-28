/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : system450.h

PURPOSE   : This header file contains definitions that are shared by the
            System 450 controller and the expansion modules. The definitions
            are shared because they are related to data that is communicated
            over the I2C bus.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __system450_inc
#define  __system450_inc
                                        // I2C messages address a specific
                                        //    point within a module. 
#define  ID_RELAY_1                0    // Relay address within a module
#define  ID_RELAY_2                1    // Relay address within a module
#define  ID_ANALOG_1               0    // Analog output address within a module
#define  ID_ANALOG_2               1    // Analog output address within a module

                                        // Expansion modules identify their
                                        //    type with the following definitions.
#define  MODULE_TYPE_C450SB     0x01    // Slave Module - Single Relay Output
#define  MODULE_TYPE_C450SC     0x02    // Slave Module - Dual Relay Output
#define  MODULE_TYPE_C450SQ     0x03    // Slave Module - Dual Analog Output
#define  MODULE_TYPE_C450SP     0x04    // Slave Module - Single Analog Output
                                        //   This module type does not exist
                                        //   as of Control Module FW 1.03. 
                                        //   Support for a single analog output
                                        //   module is retained in the code, 
                                        //   since that was the original requirement.
                                        //   Support is retained in case such a
                                        //   module is ever developed.

#define  MODULE_TYPE_C450CB     0x11    // Control Module - Single Relay Output
#define  MODULE_TYPE_C450CC     0x12    // Control Module - Dual Relay Output
#define  MODULE_TYPE_C450CP     0x13    // Control Module - Single Analog Output
#define  MODULE_TYPE_C450CQ     0x14    // Control Module - Single Analog Output
#define  MODULE_TYPE_C450RB     0x15    // Reset Control Module - Single Relay Output
#define  MODULE_TYPE_C450RC     0x16    // Reset Control Module - Dual Relay Output
#define  MODULE_TYPE_C450CPW    0x17    // Control Module - Single Analog Output,
                                        //    Pulse Output to control EC Motor Speed

#define  MODULE_TYPE_C450CRN    0x18    // Modbus Control Module
#define  MODULE_TYPE_C450CEN    0x19    // Ethernet Control Module

#define  MODULE_INVALID_ADDRESS 0x00    // Default address of expansion modules,
                                        //    prior to being assigned an address.

#define  MODULE_BASE_ADDRESS    0x20    // Address of the "first" expansion module.
                                        //    This is the expansion  module 
                                        //    connected to the control module.

#define  MAX_MODULES              10    // Maximum number of expansion modules that
                                        //    the control module will attempt to
                                        //    communicate with.

#define  MAX_OUTPUTS              10    // Maximum number of outputs that
                                        //    the control module will support.

#define  MIN_ADDR_PULSES           5    // Minimum number of serial address pulses
                                        //    that an expansion module should see.
                                        //    If a lesser value is observed, they
                                        //    are ignored.

#endif
