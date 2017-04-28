/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : Sensor_Task.c

PURPOSE   : This task initializes the ADC and provides for routine sampling
            of the analog inputs. The analog inputs include;
 
                1. Sensor 1 (resistive or voltage)
                2. Sensor 2 (resistive or voltage)
                3. Sensor 3 (resistive or voltage)
                4. 5V external (wiring terminal)
                5. 10V reference
                6. CPU temperature (internal to micro)

            These inputs are sampled over a period of 800 mSec, and upon
            conclusion of this period the raw ADC values are converted
            to engineering units. Each input is converted every 4.166 mSec,
            using 32 count hardware averaging. Each input is converted 32
            times over the 800 mSec period, and since hardware averaging is
            used, this results in a total of 1024 samples per input.

            SAMPLE CYCLE : This term is used to define the 800 mSec time
                           period over which all inputs are sampled and
                           the results are converted to engineering units.
           
            Interrupts :  ADC0 and ADC1 generate interrupts upon completion
                          of a conversion.

                          While a sample cycle is in progress, PIT1 
                          generates an interrupt every 4.166 mSec, and
                          is used to start a conversion. This causes the
                          conversions to be spaced in time at a specific
                          frequency.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "defines.h"
#include "pit_defines.h"
#include "c450_io.h"
#include "global.h"
#include "periodic_events.h"
#include "Sensor_Task.h"
#include "sensors.h"
#include "func.h"

// There are two events that may trigger this task to run;
//
//  ADC_START_SAMPLE_CYCLE_MASK - This event is set once per second as
//     part of the period event scheduling. It starts in motion the
//     sampling of all analog inputs. Each input is sampled 32 times.
//
//  ADC_SAMPLE_CYCLE_COMPLETE_MASK - This event is set when all inputs
//     have been fully sampled.
//

#define  SENSOR_TASK_EVENTS (_mqx_uint) (ADC_SAMPLE_CYCLE_COMPLETE_MASK | ADC_START_SAMPLE_CYCLE_MASK)


// AdcConfig[] is an array of values that are used to configure each
//             of the analog inputs prior to a conversion.
//
//   For each analog input there are 3 parameters;
//
//   Status & Control Register 1 : This value selects an input channel for a
//    (ADCx_SC1A)                  specific ADC and configures that channel's
//                                 interrupt enable and differential mode.
//
//  Config Register 2 : This value selects the ADC's channel A or B,
//   (ADCx_CFG2)        sets the high speed configuration, and the 
//                      long sample time selection.
//
//  ADC ID : This value indicates if the analog input is connected
//           to either ADC0 or ADC1.
//
const ADC_CONFIG AdcConfig[] = 
{ 
/*
  // ADCx_SC1A Reg    ADCx_CFG2 Reg       ADC ID (0 or 1)
  //
  // AdcInput Index = IDX_AI_SN1_V (AI1V on schematic)
  {ADC_SC1A_SN1_V,    ADC_CFG2_SN1_V,     ADC_ID_1},

  // AdcInput Index = IDX_AI_SN2_V (AI2V on schematic)
  {ADC_SC1A_SN2_V,    ADC_CFG2_SN2_V,     ADC_ID_1},

  // AdcInput Index = IDX_AI_SN3_V (AI3V on schematic)
  {ADC_SC1A_SN3_V,    ADC_CFG2_SN3_V,     ADC_ID_1},

  // AdcInput Index = IDX_AI_5_VOLT (AI4V on schematic)
  {ADC_SC1A_5_VOLT,   ADC_CFG2_5_VOLT,    ADC_ID_0},

  // AdcInput Index = IDX_AI_10_VOLT (AI5V on schematic)
  {ADC_SC1A_10_VOLT,  ADC_CFG2_10_VOLT,   ADC_ID_1},

  // AdcInput Index = IDX_AI_SN1_R (AI1R on schematic)
  {ADC_SC1A_SN1_R,    ADC_CFG2_SN1_R,     ADC_ID_0},

  // AdcInput Index = IDX_AI_SN2_R (AI2R on schematic)
  {ADC_SC1A_SN2_R,    ADC_CFG2_SN2_R,     ADC_ID_0},

  // AdcInput Index = IDX_AI_SN3_R (AI3R on schematic)
  {ADC_SC1A_SN3_R,    ADC_CFG2_SN3_R,     ADC_ID_0},

  // AdcInput Index = IDX_AI_CPU_TEMP
  {ADC_SC1A_CPU_TEMP, ADC_CFG2_CPU_TEMP,  ADC_ID_0}
*/

  // ADCx_SC1A Reg    ADCx_CFG2 Reg       ADC ID (0 or 1)
  //
  // AdcInput Index = IDX_AI_CPU_TEMP
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 },
  {ADC_SC1A_CPU_TEMP,  ADC_CFG2_CPU_TEMP,  ADC_ID_0 }

};

SAMPLE_STRUCT    Sample[MAX_ANA_INPUTS];
int              SampleState;

                 // The random number generator is seeded by this task,
                 //    using the sum of the ADC value of all of the analog 
                 //    inputs. This is done only once, after these inputs
                 //    are sampled the first time.
                 //
bool             SeedRandom = TRUE;



uint32_t     Spi1_mcr;
uint32_t     Spi1_ctar0;
uint32_t     Spi1_ctar1;
uint32_t     Spi1_sr;



// Function Prototypes - The functions defined here are used by this task only.
//
void    init_sample_struct( SENSOR * sensor );
void    select_conditioning_circuits( SENSOR * sensor );

void    start_sample_sequence( void );
void    stop_sample_sequence( void );

void    init_sensor_isr_handlers( void );
void    pit_1_isr( uintptr_t /* pointer */ isr );
void    adc0_isr( uintptr_t /* pointer */ isr );
void    adc1_isr( uintptr_t /* pointer */ isr );

void    initialize_adc();
void    adc_calibrate( void );
void    start_analog_conversion( const ADC_CONFIG * cfg );
void    update_sample_state( uint32_t raw_value );


//
//  Sensor_Task()
//
void Sensor_Task( uint32_t data )
{
    _mqx_uint  event_signal, result;
    int        calibrate_timer, k;
    uint32_t   seed_value;

    _lwevent_clear( &eventSensorTask, (_mqx_uint) SENSOR_TASK_EVENTS );

    init_sample_struct( &sensorDB.sensor[0] );

// THESE IO POINTS (Res v Volt mux) APPEAR TO CAUSE PROBLEMS w/ FREEDOM BOARD
//    select_conditioning_circuits( &sensorDB.sensor[0] );

    init_sensor_isr_handlers();

    initialize_adc();
    adc_calibrate();            // Calibrate ADC0 and ADC1
    initialize_adc();

    // Load the countdown timer that will be used to determine when the
    //   ADCs should be re-calibrated. This timer is decremented once
    //   per second at the conclusion of a sample cycle.
    //
    calibrate_timer = CALIBRATE_TIME;

    start_sample_sequence();    // Start the sampling sequence (start PIT 1)

    while( TRUE )
    { 
PA_PDDR = GPIOA_PDDR;
PB_PDDR = GPIOB_PDDR;
PC_PDDR = GPIOC_PDDR;
PD_PDDR = GPIOD_PDDR;
PE_PDDR = GPIOE_PDDR;


Spi1_mcr   = SPI1_MCR;
Spi1_ctar0 = SPI1_CTAR0;
Spi1_ctar1 = SPI1_CTAR1;
Spi1_sr    = SPI1_SR;

//SPI1_MCR |= 0x00010000;
//Spi1_mcr  = SPI1_MCR;


        // Tick Timeout is 200 ticks (2 sec). This device is configured such 
        //  that there are 100 ticks per second, see: _time_get_ticks_per_sec()
        //  DEBUG - change this so that the timeout is calculated upon
        //  task startup. . . maybe some globals shared by all tasks?
        //
        result = _lwevent_wait_ticks( &eventSensorTask, (_mqx_uint) SENSOR_TASK_EVENTS, FALSE, 200 );

        // The "event_signal" contains a bit mask that indicates which
        //     of the possible events has just occurred.
        //
        event_signal = _lwevent_get_signalled();

        // Clear all of the events that have been detected.
        _lwevent_clear( &eventSensorTask, event_signal );

        // If it is time to start a Sample Cycle;
        // 
        //   1. Build the list of inputs that will be sampled. This list
        //      always contains 5V ext, 10V ref, and CPU Temp. It also
        //      contains Sn-1,2,3, however, those sensor inputs could
        //      be either a voltage or resistive input. The list is updated
        //      prior to each sample cycle to allow for any changes to
        //      a sensor input type (voltage -> resitive or vice-versa).
        //
        //  2. Select the conditioning circuit, voltage or resistance 
        //     input, prior to starting the next sample cycle.
        //
        //  3. Start the sample sequence by starting PIT 1 which generates
        //     periodic interrupts used to start conversions.
        //
        if( event_signal & ADC_START_SAMPLE_CYCLE_MASK )
        {
            init_sample_struct( &sensorDB.sensor[0] );
            select_conditioning_circuits( &sensorDB.sensor[0] );
            start_sample_sequence();
        }
 
        // IF the Sample Cycle is complete, convert the inputs to engineering
        //    units and update the core database with the results.
        //
        if( event_signal & ADC_SAMPLE_CYCLE_COMPLETE_MASK )
        {
            // Seed the random number generator one time. It will be 
            //   seeded with the sum of all of the ADC value for each of
            //   the analog inputs. This seeding occurs the first time
            //   we have sampled all 6 inputs.
            //
            if( SeedRandom )
            {
                SeedRandom = FALSE;   // Prevent further seeding
                seed_value = 0;

                for( k=0; k<MAX_ANA_INPUTS; k++ )  // Sum all ADC inputs
                    seed_value += Sample[k].raw;
  
                // Seed the random number generator with this "random" value.
                srand( (unsigned int) seed_value );
            }

            // Convert Raw Sn-1 samples to engineering units
            sensor_eng_units( &sensorDB.sensor[ SENSOR_ID_ONE ], &CalData, Sample[ IDX_ANA_SENSOR_1 ].raw, SENSOR_ID_ONE );

            // Convert Raw Sn-2 samples to engineering units
            sensor_eng_units( &sensorDB.sensor[ SENSOR_ID_TWO ], &CalData, Sample[ IDX_ANA_SENSOR_2 ].raw, SENSOR_ID_TWO );

            // Convert Raw Sn-3 samples to engineering units
            sensor_eng_units( &sensorDB.sensor[ SENSOR_ID_THREE ], &CalData, Sample[ IDX_ANA_SENSOR_3 ].raw, SENSOR_ID_THREE );

            // Update the differential and high signal select sensors based
            //    on the update Sn-1, 2, and 3 sensors.
            update_differential_sensor( &sensorDB.sensor[0] );
            update_high_signal_sensor( &sensorDB.sensor[0] );

            // Convert Raw 5 vdc external samples to engineering units
            coreDB.five_volt_ext = calc_reference_voltage( Sample[ IDX_ANA_5_VOLT ].raw );

            // Convert Raw 10 vdc reference samples to engineering units
            coreDB.ten_volt_ref = calc_reference_voltage( Sample[ IDX_ANA_10_VOLT ].raw );

            // Convert Raw CPU Temp samples to engineering units
            coreDB.cpu_temp = calc_cpu_temp( Sample[ IDX_ANA_CPU_TEMP ].raw );

            // If the ext. 5 vdc is low, check for the error condition.
            // Else; clear the error condition.
            //
            if( coreDB.five_volt_ext < SENSOR_POWER_THRESHOLD )
            {
                if( Error.sensor_power_pending == FALSE )
                {
                    Error.sensor_power_pending = TRUE;
                    Error.sensor_power_delay   = SUPPLY_POWER_DELAY_TIME;
                }
                else
                {
                    if( Error.sensor_power_delay )
                        Error.sensor_power_delay--;

                    if( Error.sensor_power_delay == 0 )
                        Error.sensor_power_fail = TRUE;
                }
            }
            else
            {
                Error.sensor_power_fail    = FALSE;
                Error.sensor_power_pending = FALSE;
                Error.sensor_power_delay   = SUPPLY_POWER_DELAY_TIME;
            }

            // If the 10vdc reference is low, check for the error condition.
            // Else; clear the error condition.
            //
            if( coreDB.ten_volt_ref < SUPPLY_POWER_THRESHOLD )
            {
                if( Error.supply_power_pending == FALSE )
                {
                    Error.supply_power_pending = TRUE;
                    Error.supply_power_delay   = SUPPLY_POWER_DELAY_TIME;
                }
                else
                {
                    if( Error.supply_power_delay )
                        Error.supply_power_delay--;

                    if( Error.supply_power_delay == 0 )
                        Error.supply_power_fail = TRUE;
                }
            }
            else
            {
                Error.supply_power_fail    = FALSE;
                Error.supply_power_pending = FALSE;
                Error.supply_power_delay   = SUPPLY_POWER_DELAY_TIME;
            }

            // Routinely re-calibrate ADC0 and ADC1. The value "calibrate_timer"
            //   is counting seconds because this code runs in response to
            //   the Sample Cycle Complete event, which occurs once per second.
            //
            if( !Error.sensor_power_pending  && !Error.supply_power_pending  )
            {
                if( --calibrate_timer <= 0 )   // If count-down timer = 0, re-cal
                {
                    calibrate_timer = CALIBRATE_TIME;
                    adc_calibrate(); 
                    initialize_adc();
                }
            }

            // If the mutex is succesfully locked (access is granted), update
            //    the "sensorDB" with the core sensor setup data. Update 
            //    the "coreDB" with the current sensor status data.
            //
            if( _mutex_lock( &mutexCore ) == MQX_OK )
            {
                // Copy the sensor setup information for sensors 1, 2, and 3
                //    from the core DB.
                //
                for( k=SENSOR_ID_NONE; k<=SENSOR_ID_THREE; k++ )
                    sensorDB.sensor[k].setup = coreDB.sensor[k].setup;

                // Copy the sensor setup information for Sn-d, HI-2, and HI-3
                //    from the sensor DB -> the core DB. THESE ARE "virtual"
                //    sensors and their state is determined by the sensor 
                //    task.
                for( k=SENSOR_ID_DIFF; k<=SENSOR_ID_HIGH_SIGNAL_3; k++ )
                    coreDB.sensor[k].setup = sensorDB.sensor[k].setup;

                // Copy the current sensor status data TO the core DB
                //
                for( k=0; k<MAX_SENSORS; k++ )
                {
                    coreDB.sensor[k].value_int   = sensorDB.sensor[k].value_int;
                    coreDB.sensor[k].value_float = sensorDB.sensor[k].value_float;
                    coreDB.sensor[k].fail        = sensorDB.sensor[k].fail;
                    coreDB.sensor[k].signal      = sensorDB.sensor[k].signal;
                }

                _mutex_unlock( &mutexCore );    // Unlock the mutex
            }
        }      // End - if( event_signal & ADC_SAMPLE_CYCLE_COMPLETE_MASK )
    }
}

//
//  init_sensor_isr_handlers() - 
//
//   This routine establishes and enables the interrupt handler for both
//   of the A/D Converters (ADC0 and ADC1), and for Periodic Timer 1 (PIT1).
//
//   The ADC interrupts occur upon completion of a conversion.
//
//   The PIT1 interrupts occur after completion of a time interval, which is
//   used to trigger the start of a conversion. PIT1 is used to space the
//   conversions over time, relative to one another, at a specific frequency.
//
void    
init_sensor_isr_handlers( void )
{
    // Init ADC0 interrupts
    //
    _int_install_isr( INT_ADC0, (INT_ISR_FPTR) adc0_isr, NULL );
    _nvic_int_init( INT_ADC0, 4, TRUE );

    // Init ADC1 interrupts
    //
    _int_install_isr( INT_ADC1, (INT_ISR_FPTR) adc1_isr, NULL );
    _nvic_int_init( INT_ADC1, 4, TRUE );

    // Install PIT1 Interrupt handler
    //
    _int_install_isr( INT_PIT1, (INT_ISR_FPTR) pit_1_isr, NULL );
    _nvic_int_init( INT_PIT1, 5, TRUE );
}


//
//  pit_1_isr() - Interrupt service handler for Periodic Interval Timer 1
//
void  pit_1_isr( uintptr_t /* pointer */ isr )
{
    PIT_MemMapPtr  pit;

    // Get a pointer to the PIT registers
    pit = (PIT_MemMapPtr) PIT_BASE_PTR;

    pit->CHANNEL[1].TFLG = 0x01;  // Clear PIT1 Interrupt Flag
                                  //   by writing a '1' to bit 0 (TIF)

    // There is a glitch in the PIT in that in order for repeated,
    // periodic interrupts to occur, either the LDVAL or CVAL 
    // register must be read.
    //
    pit->CHANNEL[1].LDVAL = PIT_ADC_SAMPLE_INTERVAL;


    switch( SampleState )
    {
        case STATE_SAMPLE_SN1:
            start_analog_conversion( Sample[IDX_ANA_SENSOR_1].adc_cfg );
        break;

        case STATE_SAMPLE_SN2:
            start_analog_conversion( Sample[IDX_ANA_SENSOR_2].adc_cfg );
        break;

        case STATE_SAMPLE_SN3:
            start_analog_conversion( Sample[IDX_ANA_SENSOR_3].adc_cfg );
        break;

        case STATE_SAMPLE_5V:
            start_analog_conversion( Sample[IDX_ANA_5_VOLT].adc_cfg );
        break;

        case STATE_SAMPLE_10V:
            start_analog_conversion( Sample[IDX_ANA_10_VOLT].adc_cfg );
        break;

        case STATE_SAMPLE_CPU_TEMP:
            start_analog_conversion( Sample[IDX_ANA_CPU_TEMP].adc_cfg );
        break;
    }
}


//
//  adc0_isr() - Interrupt Service handler for ADC0. This interrupt
//               occurs upon completion of a conversion, or in 
//               the case of hardware averaging, completion of all
//               of the conversions comprising the average.
//
void adc0_isr( uintptr_t /* pointer */ isr )
{
    uint32_t  raw_value;

    raw_value = ADC0_RA;   // Always read the ADC result register

    update_sample_state( raw_value );
}


//
//  adc1_isr() - Interrupt Service handler for ADC1. This interrupt
//               occurs upon completion of a conversion, or in 
//               the case of hardware averaging, completion of all
//               of the conversions comprising the average.
//
void adc1_isr( uintptr_t /* pointer */ isr )
{
    uint32_t  raw_value;

    raw_value = ADC1_RA;   // Always read the ADC result register

    update_sample_state( raw_value );
}


//
//   update_sample_state()
//
void
update_sample_state( uint32_t raw_value )
{
    SAMPLE_STRUCT * ptr;

    switch( SampleState )
    {
        case STATE_SAMPLE_SN1:
            ptr = &Sample[ IDX_ANA_SENSOR_1 ];

            ptr->adc_sum += raw_value;
    
            ptr->sample_count++;

            if( ptr->sample_count >= MAX_ADC_SAMPLE )
                ptr->raw = (ptr->adc_sum + (MAX_ADC_SAMPLE/2)) / MAX_ADC_SAMPLE;

            SampleState = STATE_SAMPLE_SN2;
        break;

        case STATE_SAMPLE_SN2:
            ptr = &Sample[ IDX_ANA_SENSOR_2 ];

            ptr->adc_sum += raw_value;
    
            ptr->sample_count++;

            if( ptr->sample_count >= MAX_ADC_SAMPLE )
                ptr->raw = (ptr->adc_sum + (MAX_ADC_SAMPLE/2)) / MAX_ADC_SAMPLE;

            SampleState = STATE_SAMPLE_SN3;
        break;

        case STATE_SAMPLE_SN3:
            ptr = &Sample[ IDX_ANA_SENSOR_3 ];

            ptr->adc_sum += raw_value;
    
            ptr->sample_count++;

            if( ptr->sample_count >= MAX_ADC_SAMPLE )
            {
                ptr->raw = (ptr->adc_sum + (MAX_ADC_SAMPLE/2)) / MAX_ADC_SAMPLE;
                SampleState = STATE_SAMPLE_5V;
            }
            else
                SampleState = STATE_SAMPLE_SN1;
        break;

        case STATE_SAMPLE_5V:
            ptr = &Sample[ IDX_ANA_5_VOLT ];

            ptr->adc_sum = raw_value;
            ptr->raw     = raw_value;
            SampleState  = STATE_SAMPLE_10V;
        break;

        case STATE_SAMPLE_10V:
            ptr = &Sample[ IDX_ANA_10_VOLT ];

            ptr->adc_sum = raw_value;
            ptr->raw     = raw_value;
            SampleState  = STATE_SAMPLE_CPU_TEMP;
        break;

        case STATE_SAMPLE_CPU_TEMP:
            ptr = &Sample[ IDX_ANA_CPU_TEMP ];

            ptr->adc_sum = raw_value;
            ptr->raw     = raw_value;
            SampleState  = STATE_SAMPLE_IDLE;

            stop_sample_sequence();  // Disable PIT 1, stop routine conversions
            _lwevent_set( &eventSensorTask, (_mqx_uint) ADC_SAMPLE_CYCLE_COMPLETE_MASK );
        break;

        default:
        break;
    }  
}


//
//   select_conditioning_circuits() - 
//
//     The purpose of this function is to adjust the conditioning circuit
//     of one of the Sensor inputs; Sn-1, Sn-2, or Sn-3.
//
//     The sensor inputs Sn-1, 2, 3 may be configured to measure either a
//     temperature signal (resistive),  or a pressure or humidity signal 
//     (voltage).  
//
//     When measuring a resistive input, there is a 10v reference voltage
//     that is supplied to the conditioning circuit that results in a
//     signal input to the ADC. When measuring a voltage input, this
//     10v reference is switched out of the circuit. 
//
//     For each sensor input there is a digital output that is used
//     to include \ remove the 10v reference from the conditioning 
//     circuit. These control lines are accessed with the macros;
//             "SNn_RESISTIVE_INPUT" and "SNn_VOLTAGE_INPUT". 
//
void
select_conditioning_circuits( SENSOR * sensor )
{
    if( resistive_input( sensor[ SENSOR_ID_ONE ].setup.sensor_type ) )
        SN1_RESISTIVE_INPUT;
    else
        SN1_VOLTAGE_INPUT;

    if( resistive_input( sensor[ SENSOR_ID_TWO ].setup.sensor_type ) )
        SN2_RESISTIVE_INPUT;
    else
        SN2_VOLTAGE_INPUT;

    if( resistive_input( sensor[ SENSOR_ID_THREE ].setup.sensor_type ) )
        SN3_RESISTIVE_INPUT;
    else
        SN3_VOLTAGE_INPUT;
}


//
//   init_sample_struct()
//
void
init_sample_struct( SENSOR * sensor )
{
    int  k;

    SampleState = STATE_SAMPLE_SN1;

    for( k=0; k<MAX_ANA_INPUTS; k++ )
    {
        Sample[k].adc_sum      = 0;
        Sample[k].sample_count = 0;
    }

    if( resistive_input( sensor[ SENSOR_ID_ONE ].setup.sensor_type )  )
        Sample[ IDX_ANA_SENSOR_1 ].adc_cfg = &AdcConfig[ IDX_AI_SN1_R ];
    else
        Sample[ IDX_ANA_SENSOR_1 ].adc_cfg = &AdcConfig[ IDX_AI_SN1_V ];   

    if( resistive_input( sensor[ SENSOR_ID_TWO ].setup.sensor_type )  )
        Sample[ IDX_ANA_SENSOR_2 ].adc_cfg = &AdcConfig[ IDX_AI_SN2_R ];
    else
        Sample[ IDX_ANA_SENSOR_2 ].adc_cfg = &AdcConfig[ IDX_AI_SN2_V ];

    if( resistive_input( sensor[ SENSOR_ID_THREE ].setup.sensor_type )  )
        Sample[ IDX_ANA_SENSOR_3 ].adc_cfg = &AdcConfig[ IDX_AI_SN3_R ];
    else
        Sample[ IDX_ANA_SENSOR_3 ].adc_cfg = &AdcConfig[ IDX_AI_SN3_V ];

    Sample[IDX_ANA_5_VOLT].adc_cfg   = &AdcConfig[ IDX_AI_5_VOLT   ];
    Sample[IDX_ANA_10_VOLT].adc_cfg  = &AdcConfig[ IDX_AI_10_VOLT  ];
    Sample[IDX_ANA_CPU_TEMP].adc_cfg = &AdcConfig[ IDX_AI_CPU_TEMP ];
}


//
//  start_sample_sequence() - PIT = Periodic Interval Timer. It is a countdown
//                 timer that generates an interrupt when the count
//                 reaches zero, at which point it reloads and counts
//                 down again.
//
//                 There are four channels to this timer, numbered
//                 0 - 3. This application uses channel 1 (PIT1),
//                 to start an analog conversion.
//
void
start_sample_sequence( void )
{
    PIT_MemMapPtr  pit;

    SIM_SCGC6 |= SIM_SCGC6_PIT_MASK;    // Gate the clock to the PIT

    // Get a pointer to the PIT registers
    pit = (PIT_MemMapPtr) PIT_BASE_PTR;

    // MCR = PIT Module Control Register
    //       Clear the MDIS bit, bit 1, enabling the PIT
    //       Clear the FRZ bit, bit 0, timer continues to run in debug mode
    //
    pit->MCR = 0x01; 
                           
    // LDVAL = Timer Load Value Register, the PIT timer counts down from
    //         this value, generating an interrupt when it reaches zero,
    //         then reloads this value and begins counting down again.
    //
    pit->CHANNEL[1].LDVAL = PIT_ADC_SAMPLE_INTERVAL; 

    // TFLG = Timer Flag Register, bit 0 holds the PIT timer interrupt flag.
    //
    pit->CHANNEL[1].TFLG = 0x01;   // Clear Timer Interrupt Flag
                                   // bit 0 = TIF, Timer Interrupt Flag,
                                   //         It is cleared by writing
                                   //         a '1' to it.

    // TCTRL = Timer Control Register, bits 1 and 0 holds the PIT timer 
    //         interrupt enable, and timer enable control bits.
    //
    pit->CHANNEL[1].TCTRL = 0x03;  // bits 31-2 = reserved
                                   // bit     1, TIE, 1 = Interrupt Enabled
                                   // bit     0, TEN, 1 = Timer Enabled
}


//
//  stop_sample_sequence() - This routine halts routine ADC converions
//                           by stopping the PIT 1 timer. PIT 1 is to start
//                           ADC conversions on a routine time interval.
//                           Once a sample cycle completes, PIT 1 is stopped.
//
void  
stop_sample_sequence( void )
{
    ADC_MemMapPtr  adc0;
    ADC_MemMapPtr  adc1;
    PIT_MemMapPtr  pit;
   
    // Get a pointer to the PIT registers
    pit = (PIT_MemMapPtr) PIT_BASE_PTR;

    // TCTRL = Timer Control Register, bits 1 and 0 holds the PIT timer 1
    //         interrupt enable, and timer enable control bits.
    //
    pit->CHANNEL[1].TCTRL = 0x00;  // bits 31-2 = reserved
                                   // bit  1, TIE, 0 = Interrupt Disabled
                                   // bit  0, TEN, 0 = Timer Disabled

    // Get pointers to the ADC-0 and ADC-1 registers
    adc0 = (ADC_MemMapPtr) ADC0_BASE_PTR;
    adc1 = (ADC_MemMapPtr) ADC1_BASE_PTR;

    // Disable analog interrupts
    adc0->SC1[0] &= ~ADC_SC1_AIEN_MASK;  // Clear AIEN, interrupt enable
    adc0->SC1[1] &= ~ADC_SC1_AIEN_MASK;  // Clear AIEN, interrupt enable
    adc1->SC1[0] &= ~ADC_SC1_AIEN_MASK;  // Clear AIEN, interrupt enable
    adc1->SC1[1] &= ~ADC_SC1_AIEN_MASK;  // Clear AIEN, interrupt enable
}

//
//   start_analog_conversion()
//
void
start_analog_conversion( const ADC_CONFIG * cfg ) 
{
    ADC_MemMapPtr  ptrAdc;

    if( cfg->adc_id == ADC_ID_0 )
    {
        ptrAdc = (ADC_MemMapPtr) ADC0_BASE_PTR;
    }
    else
    {
        ptrAdc = (ADC_MemMapPtr) ADC1_BASE_PTR;
    }

    // Load the configuration 2 register (CFG2)
    ptrAdc->CFG2   = cfg->adc_cfg2;

    // Load the status / control 1A register (SC1A)
    // Writing to this reg selects an input channel and starts a conversion
    ptrAdc->SC1[0] = cfg->adc_sc1a;
}


void initialize_adc()
{
    ADC_MemMapPtr  adc0;
    ADC_MemMapPtr  adc1;

    // Get pointers to the ADC-0 and ADC-1 registers
    adc0 = (ADC_MemMapPtr) ADC0_BASE_PTR;
    adc1 = (ADC_MemMapPtr) ADC1_BASE_PTR;

    // Gate the ADC0 and ADC1 clocks
    SIM_SCGC6 |= (SIM_SCGC6_ADC0_MASK );
    SIM_SCGC6 |= (SIM_SCGC6_ADC1_MASK );


    //         Setup the initial configuration of ADC0
    //
    // ADC Configuration Register 1
    //
    adc0->CFG1 = 0x9F;  // Bit 7   = ADLPC 
                        //         = 1, Low power configurtion
                        //
                        // Bit 6-5 = ADIV, clock divide select
                        //         = 00 (binary), divide ratio is 1,
                        //              clock rate is clock input / 1
                        //
                        // Bit 4   = ADLSMP, sample time configuration
                        //         = 1, long sample time
                        //
                        // Bit 3-2 = MODE, conversion mode select
                        //         = 11 (binary), 16 bit conversion
                        //
                        // Bit 1-0 = ADICLK, input clock select
                        //         = 11 (binary), asynchronous clock


    // ADC Configuration Register 2
    //
    adc0->CFG2 = 0x00;  // Bit 4   = MUXSEL, ADC mux select
                        //         = 0, "A" channel selected
                        //
                        // Bit 3   = ADACKEN, asynchronous clock output
                        //         = 0, async clock output disabled
                        //
                        // Bit 2   = ADHSC, high speed configuration
                        //         = 0, normal conversion sequence
                        //
                        // Bit 1-0 = ADLSTS, long sample time select
                        //         = 00 (binary), longest sample time

    adc0->CV1 = 0x00;   // Compare Value register 1, not used 
    adc0->CV2 = 0x00;   // Compare Value register 2, not used

    // ADC Status and Control Register 2
    //
    adc0->SC2 = 0x00;   // Bit 6   = ADTRG, conversion trigger select
                        //         = 0, software trigger selected
                        //
                        // Bit 5   = ACFE, compare function enable
                        //         = 0, compare function disabled
                        //
                        // Bit 4   = ACFGT, compare function ">" enable
                        //         = 0, doesn't matter, disabled via ACFE
                        //
                        // Bit 3   = ACREN, compare function range enable
                        //         = 0, range function disabled
                        //
                        // Bit 2   = DMAEN, DMA enable
                        //         = 0, DMA disabled
                        //
                        // Bit 1-0 = REFSEL, voltage reference select
                        //         = 00 (binary), external VREFH and VREFL

    // ADC Status and Control Register 3
    //
    adc0->SC3 = 0x07;   // Bit 7   = CAL, calibration
                        //         = 0, software trigger selected
                        //
                        // Bit 6-4 = read only bits
                        //         = 0
                        //
                        // Bit 3   = ADCO, continuous conversion enable
                        //         = 0, not continuous
                        //
                        // Bit 2   = AVGE, hardware average enable
                        //         = 1, hardware average enabled
                        //
                        // Bit 1-0 = AVGS, hardware average select
                        //         = 11, 32 samples averaged

    // ADC PGA Register (programmable gain, differential inputs only)
    //
//    adc0->PGA = 0x00;   // Bit 23    = PGAEN, programmable gain enable
                        //           = 0, PGA disabled
                        //
                        // Bit 19-16 = PGAG, PGA gain setting
                        //           = 0, gain = 1 (not used here )

    // ADC Status and Control Register 1A
    //
    adc0->SC1[0] = 0x1F; // Bit 6   = AIEN, interrupt enable
                         //         = 0, interrupt disabled
                         //
                         // Bit 5   = DIFF, differential mode enable
                         //         = 0, single ended conversion
                         //
                         // Bit 4-0 = ADCH, input channel select
                         //         = 11111 (binary), module disabled

    // ADC Status and Control Register 1B
    //
    adc0->SC1[1] = 0x1F; // Bit 6   = AIEN, interrupt enable
                         //         = 0, interrupt disabled
                         //
                         // Bit 5   = DIFF, differential mode enable
                         //         = 0, single ended conversion
                         //
                         // Bit 4-0 = ADCH, input channel select
                         //         = 11111 (binary), module disabled

    // ADC1 is setup identically to ADC0. Copy the ADC0 registers to ADC1.
    //
    *adc1 = *adc0;
}


//
//  Function Name : AUTO CAL ROUTINE (from Inga Harris' Nucleus ADC0_ Validation)
//
//  Notes         : Calibrates ADC0 and ADC1, typically performed following a reset.
//
//		ADACKEN is deliberately set in this function.  This is to resolve
//		an issue with the calibration routine failing due to asynchronous
//		clock start up time.
//
//		The Calibration routine appears to resolve a 1%-2% error in the
//		ADC reading (when compared to the uncalibrated result).
//
void adc_calibrate( void )
{
    unsigned short cal_var;

    // Gate the ADC0 and ADC1 clocks
    SIM_SCGC6 |= (SIM_SCGC6_ADC0_MASK );
    SIM_SCGC6 |= (SIM_SCGC6_ADC1_MASK );

    ADC0_CFG2 |= ADC_CFG2_ADACKEN_MASK;   // Set ADACKEN bit - turns on asynchronous clock for calibration
    ADC1_CFG2 |= ADC_CFG2_ADACKEN_MASK;     
    
    delay_usec(5);       //  Asynchronous clock takes up to 5 usec to start
    
    // CALIBRATE - ADC0
    //
    // Enable Software Conversion Trigger for Calibration Process
    ADC0_SC2 &=  ~ADC_SC2_ADTRG_MASK; 

    // set single conversion, clear avgs bitfield for next writing
    ADC0_SC3 &= ( ~ADC_SC3_ADCO_MASK & ~ADC_SC3_AVGS_MASK ); 

    // Turn averaging ON and set at max value ( 32 )
    ADC0_SC3 |= ( ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3) );


    ADC0_SC3 |= ADC_SC3_CAL_MASK;   // Start CAL

    // Wait for completion complete
    while( (ADC0_SC1A & ADC_SC1_COCO_MASK ) == 0 )
        ;

    // If the Calibration Fail Flag is not set, store and use
    //   the calibration values just obtained.
    //
    if( (ADC0_SC3 & ADC_SC3_CALF_MASK) != ADC_SC3_CALF_MASK ) 
    {
        // Calculate plus-side calibration per Kinetis Ref Manual 21.4.7
        cal_var  = 0x00;

        cal_var  = ADC0_CLP0;
        cal_var += ADC0_CLP1;
        cal_var += ADC0_CLP2;
        cal_var += ADC0_CLP3;
        cal_var += ADC0_CLP4;
        cal_var += ADC0_CLPS;

        cal_var  = cal_var / 2;
        cal_var |= 0x8000;              // Set MSB

        ADC0_PG  = ADC_PG_PG(cal_var);

        // Calculate minus-side calibration per Kinetis Ref Manual 21.4.7
        cal_var  = 0x00;

        cal_var  = ADC0_CLM0;
        cal_var += ADC0_CLM1;
        cal_var += ADC0_CLM2;
        cal_var += ADC0_CLM3;
        cal_var += ADC0_CLM4;
        cal_var += ADC0_CLMS;

        cal_var = cal_var / 2;

        cal_var |= 0x8000;              // Set MSB

        ADC0_MG = ADC_MG_MG(cal_var);
    }

    ADC0_SC3 &= ~ADC_SC3_CAL_MASK;  // Clear CAL bit 


    // CALIBRATE - ADC1
    //
    // Enable Software Conversion Trigger for Calibration Process
    ADC1_SC2 &=  ~ADC_SC2_ADTRG_MASK; 

    // set single conversion, clear avgs bitfield for next writing
    ADC1_SC3 &= ( ~ADC_SC3_ADCO_MASK & ~ADC_SC3_AVGS_MASK ); 

    // Turn averaging ON and set at max value ( 32 )
    ADC1_SC3 |= ( ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3) );


    ADC1_SC3 |= ADC_SC3_CAL_MASK;   // Start CAL

    // Wait calibration end
    while( (ADC1_SC1A & ADC_SC1_COCO_MASK ) == 0 )
        ;

    // If the Calibration Fail Flag is not set, store and use
    //   the calibration values just obtained.
    //
    if( (ADC1_SC3 & ADC_SC3_CALF_MASK) != ADC_SC3_CALF_MASK ) 
    {
        // Calculate plus-side calibration as per 21.4.7
        cal_var  = 0x00;

        cal_var  = ADC1_CLP0;
        cal_var += ADC1_CLP1;
        cal_var += ADC1_CLP2;
        cal_var += ADC1_CLP3;
        cal_var += ADC1_CLP4;
        cal_var += ADC1_CLPS;

        cal_var  = cal_var / 2;
        cal_var |= 0x8000;              // Set MSB

        ADC1_PG  = ADC_PG_PG(cal_var);

        // Calculate minus-side calibration as per 21.4.7
        cal_var  = 0x00;

        cal_var  = ADC1_CLM0;
        cal_var += ADC1_CLM1;
        cal_var += ADC1_CLM2;
        cal_var += ADC1_CLM3;
        cal_var += ADC1_CLM4;
        cal_var += ADC1_CLMS;

        cal_var = cal_var / 2;

        cal_var |= 0x8000;              // Set MSB

        ADC1_MG = ADC_MG_MG(cal_var);
    }

    ADC1_SC3  &= ~ADC_SC3_CAL_MASK ;       // Clear CAL bit 


    ADC0_CFG2 &= ~ADC_CFG2_ADACKEN_MASK;   //  Clear ADACKEN bit - turns off freerunning clock, activates clock only when needed
    ADC1_CFG2 &= ~ADC_CFG2_ADACKEN_MASK;   //  Clear ADACKEN bit - turns off freerunning clock, activates clock only when needed
}

