/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : Sensor_Task.h

PURPOSE   : Function prototypes for "Sensor_Task.c" module.    

            There are a number of definitions that are used to buffer and
            control conversions of the analog inputs over the course of
            a single sample cycle.

            One thing to note is that a single sensor input (Sn-1, 2, or 3)
            may be configured as one of several sensor types. Depending
            on the type (temperature or pressure/humidity), a different
            analog input channel is sampled. The temp sensors are resistive,
            and the press/humidity sensors are voltage inputs.              

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __sensor_task_inc
#define  __sensor_task_inc


// These values are used to index into the global array "Sample[]" and
//   indirectly into AdcConfig[].
//
typedef enum {

    IDX_AI_SN1_V    = 0,   // Sn-1 Voltage input   (AI1V on schematic)
    IDX_AI_SN2_V    = 1,   // Sn-2 Voltage input   (AI2V on schematic)
    IDX_AI_SN3_V    = 2,   // Sn-3 Voltage input   (AI3V on schematic)
    IDX_AI_5_VOLT   = 3,   //  5 Volt input        (AI4V on schematic)
    IDX_AI_10_VOLT  = 4,   // 10 Volt input        (AI5V on schematic)
    IDX_AI_SN1_R    = 5,   // Sn-1 Resistive input (AI1R on schematic)
    IDX_AI_SN2_R    = 6,   // Sn-2 Resistive input (AI2R on schematic)
    IDX_AI_SN3_R    = 7,   // Sn-3 Resistive input (AI3R on schematic)
    IDX_AI_CPU_TEMP = 8    // CPU temperature

}  ANALOG_INPUT_INDEX;


#define  MAX_ADC_INDEX   IDX_CPU_TEMP      // Maximum index into the ADC
                                           //    Struct arrays.

#define  MAX_ADC_INPUTS  MAX_ADC_INDEX + 1 // Maximum number of ADC inputs,

#define INPUT_VOLTAGE       0
#define INPUT_RESISTIVE     1

#define ADC_ID_0            0
#define ADC_ID_1            1

#define CALIBRATE_TIME  14400  // Re-calibrate the ADCs every 14,400 sec (4 hr)

// For each of the analog inputs a definition for the ADCx_SC1A register is
//   defined. The "x" in the register name is 0 or 1, corresponding to
//   ADC0 or ADC1.
//
// The fields in the ADCx_SC1A register include;
// 
//   Field    Bits    Description
//
//   reserved 31-8    reserved
//
//   COCO     7       ADC Mux Select, selects ADC channel A or B
//                    1 = conversion has completed
//                  
//   AEIN     6       Interrupt Enabled (conversion complete)
//                    0 = Conversion complete interrupt disabled
//                    1 = Conversion complete interrupt enabled
//
//   DIFF     5       Differential Mode Enabled
//                    0 = Single-ended conversion and input channel selected
//  
//   ADCH     4-0     Input channel select, this 5 bit field selects
//                    the input channel (input pin) for the ADC. Refer
//                    to the Freescale Reference manual for a definition
//                    of the ADC connection/channel assignments, and
//                    how those channels are encoded in ADCH.
//
//   In all cases the AEIN   = 1 (conv. complete interrupt enabled)
//                    DIFF   = 0 (single ended)
//                    ADLSTS = 0 (longest sample time)
//
//   The only field that changes in between inputs is the ADCH, the
//   channel selection. The actual ADC (0 or 1) is indicated  elsewhere.
//
//                                                             Input Signal
#define ADC_SC1A_SN1_V    ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(5)  // ADC1_SE5a 
#define ADC_SC1A_SN2_V    ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(6)  // ADC1_SE6a 
#define ADC_SC1A_SN3_V    ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(7)  // ADC1_SE7a
#define ADC_SC1A_5_VOLT   ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(1)  // ADC0_DP1
#define ADC_SC1A_10_VOLT  ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(1)  // ADC1_DP1
#define ADC_SC1A_SN1_R    ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(5)  // ADC0_SE5b 
#define ADC_SC1A_SN2_R    ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(6)  // ADC0_SE6b
#define ADC_SC1A_SN3_R    ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(7)  // ADC0_SE7b 
#define ADC_SC1A_CPU_TEMP ADC_SC1_AIEN_MASK | ADC_SC1_ADCH(26) // Temp Sensor 


// For each of the analog inputs a definition for the ADCx_CFG2 register is
//   defined. The fields in the ADCx_CFG2 register include;
// 
//   Field    Bits    Description
//
//   reserved 31-5    reserved
//
//   MUXSEL   4       ADC Mux Select, selects ADC channel A or B
//                    0 = channel A, 1 = channel B
//                  
//   ADACKEN  3       Asynchronous clock output enable
//                    0 = Async clock output is disabled
//
//   ADHSC    2       High Speed Configuration
//                    0 = Normal conversion sequence selected
//  
//   ADLSTS   1-0     Long sample time select
//                    00 = default longest sample time
//
//   In all cases the ADACKEN = 0 (clock out disabled)
//                    ADHSC   = 0 (normal conversion sequence)
//                    ADLSTS  = 0 (longest sample time)
//
//  The only that changes between analog inputs is the ADC channel, A or B
//
#define ADC_CFG2_SN1_V     0x10  // ADC Channel B
#define ADC_CFG2_SN2_V     0x10  // ADC Channel B 
#define ADC_CFG2_SN3_V     0x10  // ADC Channel B
#define ADC_CFG2_5_VOLT    0x00  // ADC Channel A
#define ADC_CFG2_10_VOLT   0x00  // ADC Channel A 
#define ADC_CFG2_SN1_R     0x10  // ADC Channel B 
#define ADC_CFG2_SN2_R     0x10  // ADC Channel B
#define ADC_CFG2_SN3_R     0x10  // ADC Channel B 
#define ADC_CFG2_CPU_TEMP  0x00  // ADC Channel A 


typedef struct
{
    // The Status and control register 1 is used to intiate conversions.
    //    Its content must be initialized before the ADC is used. The
    //    values in "adc_sc1a" are loaded directly into the ADCx_SC1A
    //    register, and include; the channel select, diff/single ended
    //    mode enable, and the interrupt enable.
    //
    uint32_t adc_sc1a;         // SC1A - Status and control register 1 A. 

    // The configuration register 2 contains a field named "MUXSEL"
    //    which is a multiplexer selection option, for ADC channels
    //    A and B. Each sensor and other analog inputs contain a
    //    unique MUXSEL setting. The remaining  bits in this register
    //    are fixed for all analog inputs that are sampled.
    //
    uint32_t adc_cfg2;     

    uint32_t adc_id;          // 0 or 1, for ADC0 or ADC1 

}  ADC_CONFIG;


#define MAX_ADC_SAMPLE 128   // Sample Sn-1,2,3 128 times every 800 mSec

typedef struct
{
    uint32_t           sample_count; // Number of conversions completed
                                     //  for this analog input
    uint32_t           adc_sum;      // Sum of all converted raw values
    uint16_t           raw;          // Average of the sum raw values
    const ADC_CONFIG * adc_cfg;      // Pointer to analog input configuration
                                     //   parameters for this analog input
}  SAMPLE_STRUCT;

// These values are used to index into the conversion list (array)
//
typedef enum 
{
    STATE_SAMPLE_IDLE     = 0,   // Sampling is not active
    STATE_SAMPLE_SN1      = 1,   // Sample sensor 1
    STATE_SAMPLE_SN2      = 2,   // Sample sensor 2
    STATE_SAMPLE_SN3      = 3,   // Sample sensor 3
    STATE_SAMPLE_5V       = 4,   // Sample 5V input
    STATE_SAMPLE_10V      = 5,   // Sample 10V input
    STATE_SAMPLE_CPU_TEMP = 6    // Sample CPU temp

}  STATE_SAMPLE;


// These values are used to index into the conversion list (array)
//
typedef enum 
{
    IDX_ANA_SENSOR_1 = 0,   // Sn-1 input
    IDX_ANA_SENSOR_2 = 1,   // Sn-2 input
    IDX_ANA_SENSOR_3 = 2,   // Sn-3 input
    IDX_ANA_5_VOLT   = 3,   //  5 Volt input
    IDX_ANA_10_VOLT  = 4,   // 10 Volt input
    IDX_ANA_CPU_TEMP = 5    // CPU temperature

}  ANA_INPUT_INDEX;

#define  MAX_ANA_INPUTS   IDX_ANA_CPU_TEMP + 1  


void Sensor_Task( uint32_t data );


#endif
