/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : pit_defines.h

PURPOSE   : This header file contains definitions that are used in                  
            conjunction with the Periodic Interval Timer (PIT).

            There are different definitions for various timer values
            because the different hardware (RS485 vs Ethernet designs)
            run at different clock rates.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __pit_defines_inc
#define  __pit_defines_inc

#include "defines.h"


// The PIT timer runs off of the bus clock (aka peripheral bus clock). 
//   The two versions of C450 hardware use different clocks. In the
//   case of the ethernet version a 50 MHz bus clock is used. In the
//   case of the RS485 version, a 24 MHz bus clock is used.
//
// Values are defined for various time intervals that will be 
//   maintained by the PIT timer. Based on the definition of the
//   hardware used (Ethernet or RS485), the appropriate time definitions
//   will be used. Defining the hardware is part of the build process,
//   it does not occur at run time.


//  With the Bus Clock is running at 24 MHz, this corresponds to a clock
//  period of 41.667 nanoSeconds. Each "tick" in the PIT is the equivalent
//  of 41.667 nSec.
//
#define BUS_24MHZ_PIT_TICK_TIME    42       // Each PIT tick is 42 nSec
                                            // ...actually, 41.66666

// This value is used to set the time interval between sampling the
//  various ADC inputs. The interval is 2.0833 mSec
//
#define BUS_24MHZ_PIT_ADC_SAMPLE_INTERVAL  50000   // 2.0833 mSec

// Calculate the number of ticks equivalent to 0.1 second.
//  0.1 Second = 100,000,000 nSec / (41.67 nSec / tick) = TICKS equiv of 0.1 Sec
//
#define BUS_24MHZ_PIT_100_mSEC   2400000    // 100000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 0.1 second.
//  0.15 Second = 150,000,000 nSec / (41.67 nSec / tick) = TICKS equiv of 0.15 Sec
//
#define BUS_24MHZ_PIT_150_mSEC   3600000    // 150000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 0.25 second.
//  0.25 Second = 250,000,000 nSec / (41.67 nSec / tick) = TICKS equiv of 0.25 Sec
//
#define BUS_24MHZ_PIT_250_mSEC   6000000    // 250000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 0.5 second.
//  0.5 Second = 500,000,000 nSec / (41.67 nSec / tick) = TICKS equiv of 0.5 Sec
//
#define BUS_24MHZ_PIT_500_mSEC  12000000    // 500000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 1 second.
//  1 Second = 1,000,000,000 nSec / (41.67 nSec / tick) = TICKS equiv of 1 Sec
//
#define BUS_24MHZ_PIT_1_SECOND  24000000    // 1000000000 / PIT_TICK_TIME  


//  With the Bus Clock is running at 50 MHz, this corresponds to a clock
//  period of 20 nanoSeconds. Each "tick" in the PIT is the equivalent
//  of 20 nSec.
//
#define BUS_50MHZ_PIT_TICK_TIME    20       // Each PIT tick is 20 nSec

// This value is used to set the time interval between sampling the
//  various ADC inputs. The interval is 2.0833 mSec
//
#define BUS_50MHZ_PIT_ADC_SAMPLE_INTERVAL  104166   // 2.0833 mSec

// Calculate the number of ticks equivalent to 0.1 second.
//  0.1 Second = 100,000,000 nSec / (20 nSec / tick) = TICKS equiv of 0.1 Sec
//
#define BUS_50MHZ_PIT_100_mSEC  5000000     // 100000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 0.1 second.
//  0.15 Second = 150,000,000 nSec / (20 nSec / tick) = TICKS equiv of 0.15 Sec
//
#define BUS_50MHZ_PIT_150_mSEC  7500000     // 150000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 0.25 second.
//  0.25 Second = 250,000,000 nSec / (20 nSec / tick) = TICKS equiv of 0.25 Sec
//
#define BUS_50MHZ_PIT_250_mSEC  12500000    // 250000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 0.5 second.
//  0.5 Second = 500,000,000 nSec / (20 nSec / tick) = TICKS equiv of 0.5 Sec
//
#define BUS_50MHZ_PIT_500_mSEC  25000000    // 500000000 / PIT_TICK_TIME

// Calculate the number of ticks equivalent to 1 second.
//  1 Second = 50,000,000 nSec / (20 nSec / tick) = TICKS equiv of 1 Sec
//
#define BUS_50MHZ_PIT_1_SECOND  50000000    // 1000000000 / PIT_TICK_TIME  

// The various PIT times used by this application are defined based
//   on the hardware used.
//
#ifdef RS485_HARDWARE
    #define PIT_ADC_SAMPLE_INTERVAL  BUS_24MHZ_PIT_ADC_SAMPLE_INTERVAL
    #define PIT_100_mSEC             BUS_24MHZ_PIT_100_mSEC
    #define PIT_150_mSEC             BUS_24MHZ_PIT_150_mSEC
    #define PIT_250_mSEC             BUS_24MHZ_PIT_250_mSEC
    #define PIT_500_mSEC             BUS_24MHZ_PIT_500_mSEC
    #define PIT_1_SEC                BUS_24MHZ_PIT_1_SECOND
#else
    #define PIT_ADC_SAMPLE_INTERVAL  BUS_50MHZ_PIT_ADC_SAMPLE_INTERVAL
    #define PIT_100_mSEC             BUS_50MHZ_PIT_100_mSEC
    #define PIT_150_mSEC             BUS_50MHZ_PIT_150_mSEC
    #define PIT_250_mSEC             BUS_50MHZ_PIT_250_mSEC
    #define PIT_500_mSEC             BUS_50MHZ_PIT_500_mSEC
    #define PIT_1_SEC                BUS_50MHZ_PIT_1_SECOND
#endif


#endif
