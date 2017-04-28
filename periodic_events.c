/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : periodic_events.c

PURPOSE   : This file contains code that;

            1) initializes the MQX events used to trigger tasks.

            2) initializes the Periodic Interval Timer 0 (PIT0)
               and responds to interrupts in order to determine when
               the events should be fired.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#include "defines.h"
#include "global.h"
#include "pit_defines.h"
#include "periodic_events.h"

#define NUM_PIT_EVENTS   5


enum{  EVENT_START_SAMPLE,  // These values are used to identify
       EVENT_UI_1,          //   which event the PIT0 timer should
       EVENT_COMM,          //   trigger next.
       EVENT_UI_2,          //
       EVENT_CONTROL };     //

unsigned int PitEventTime[NUM_PIT_EVENTS];
unsigned int ActiveEvent;


// Function Prototypes 
//
void   init_pit_0( void );
void   pit_0_isr( uintptr_t /* pointer */ isr );


//
//  init_event_handlers() - The purpose of this function is to
//
void
init_event_handlers( void )
{
    // Create an "event" structure that will be used by the
    //   Periodic Interval Timer (PIT) to signal that
    //   this control task should run. The PIT causes the
    //   control task to run once per second.
    //
    if( _lwevent_create(&eventControlTask, 0) != MQX_OK ) { _mqx_exit( 1L ); }
    if( _lwevent_create(&eventSensorTask, 0)  != MQX_OK ) { _mqx_exit( 1L ); }
    if( _lwevent_create(&eventUiTask, 0)      != MQX_OK ) { _mqx_exit( 1L ); }

    init_task_schedule_timer();
}

//
//  init_task_schedule_timer() - The purpose of this function is to
//     initialze on of the Period Interrupt Timers (PIT0), which
//     will be used to accurately schedule execution of two tasks;
//     Control_Task and UI_Task.
//
//     The Control_Task runs once per second. The UI_Task runs 
//     once every 1/2 second. Upon interrupt, this single PIT0 timer will
//     set event flags that causes these tasks to execute. Although
//     the OS can manage task execution and context switching, these
//     interrupts are scheduled such that the Control and UI Tasks
//     do not attempt to run concurrently.
//
//     Use of the PIT0 timer improves timing accuracy, and minimizes
//     CPU time spent performing context switching.
//
void
init_task_schedule_timer( void )
{
    ActiveEvent = EVENT_START_SAMPLE;

    PitEventTime[ EVENT_START_SAMPLE ] = PIT_150_mSEC;  // Time until next event -> UI_1
    PitEventTime[ EVENT_UI_1 ]         = PIT_250_mSEC;  // Time until next event -> COMM
    PitEventTime[ EVENT_COMM ]         = PIT_250_mSEC;  // Time until next event -> UI_2
    PitEventTime[ EVENT_UI_2 ]         = PIT_250_mSEC;  // Time until next event -> CONTROL
    PitEventTime[ EVENT_CONTROL ]      = PIT_100_mSEC;  // Time until next event -> START SAMPLE


    // Install PIT0 Interrupt handler
    //
    _int_install_isr( INT_PIT0, (INT_ISR_FPTR) pit_0_isr, NULL );

    // param 1 = interrupt number, INT_PIT0
    // param 2 = interrupt priority, 5
    // param 3 = enable interrupt now?, yes, TRUE
    //
    _nvic_int_init( INT_PIT0, 5, TRUE );

    init_pit_0();   // Initialize the Periodic Interrupt Timer
}


//
//  pit_0_isr() - Interrupt service handler for Periodic Interval Timer 0
//
void  pit_0_isr( uintptr_t /* pointer */ isr )
{
    PIT_MemMapPtr  pit;

    // Get a pointer to the PIT registers
    pit = (PIT_MemMapPtr) PIT_BASE_PTR;

    pit->CHANNEL[0].TFLG = 0x01;  // Clear PIT0 Interrupt Flag
                                  //   by writing a '1' to bit 0 (TIF)

    // There is a glitch in the PIT in that in order for repeated,
    // periodic interrupts to occur, either the LDVAL or CVAL 
    // register must be read.
    //

    pit->CHANNEL[0].LDVAL;

    switch( ActiveEvent )
    {
        case EVENT_START_SAMPLE:
            _lwevent_set( &eventSensorTask, (_mqx_uint) ADC_START_SAMPLE_CYCLE_MASK );
            ActiveEvent = EVENT_UI_1;
            pit->CHANNEL[0].LDVAL = PitEventTime[ ActiveEvent ];
        break;

        case EVENT_UI_1:
            _lwevent_set( &eventUiTask, (_mqx_uint) UI_LCD_UPDATE_EVENT );
            ActiveEvent = EVENT_COMM;
            pit->CHANNEL[0].LDVAL = PitEventTime[ ActiveEvent ];
        break;

        case EVENT_COMM:
#ifdef RS485_HARDWARE
            _lwevent_set( &eventModbusTask, (_mqx_uint) MODBUS_ONE_SECOND_EVENT );
#endif
            ActiveEvent = EVENT_UI_2;
            pit->CHANNEL[0].LDVAL = PitEventTime[ ActiveEvent ];
        break;

        case EVENT_UI_2:
            _lwevent_set( &eventUiTask, (_mqx_uint) UI_LCD_UPDATE_EVENT );
            ActiveEvent = EVENT_CONTROL;
            pit->CHANNEL[0].LDVAL = PitEventTime[ ActiveEvent ];
        break;

        case EVENT_CONTROL:
                            // SecCounter is a global variable that can be
            SecCounter++;   //   accessed by any task that needs to 
                            //   measure time durations in seconds.

            _lwevent_set( &eventControlTask, (_mqx_uint) CTRL_ALGORITHM_EVENT );
            ActiveEvent = EVENT_START_SAMPLE;
            pit->CHANNEL[0].LDVAL = PitEventTime[ ActiveEvent ];
        break;

        default: 
            ActiveEvent = EVENT_CONTROL;
            pit->CHANNEL[0].LDVAL = PitEventTime[ ActiveEvent ];
        break;
    }
}


//
//  init_pit_0() - PIT = Periodic Interval Timer. It is a countdown
//                 timer that generates an interrupt when the count
//                 reaches zero, at which point it reloads and counts
//                 down again.
//
//                 There are four channels to this timer, numbered
//                 0 - 3. This application uses channel 0 (PIT0),
//                 to signal that the Control Task should run.
//
void
init_pit_0( void )
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
    pit->CHANNEL[0].LDVAL = PitEventTime[ EVENT_START_SAMPLE ]; 

    // TFLG = Timer Flag Register, bit 0 holds the PIT timer interrupt flag.
    //
    pit->CHANNEL[0].TFLG = 0x01;   // Clear Timer Interrupt Flag
                                   // bit 0 = TIF, Timer Interrupt Flag,
                                   //         It is cleared by writing
                                   //         a '1' to it.

    // TCTRL = Timer Control Register, bits 1 and 0 holds the PIT timer 
    //         interrupt enable, and timer enable control bits.
    //
    pit->CHANNEL[0].TCTRL = 0x03;  // bits 31-2 = reserved
                                   // bit     1, TIE, 1 = Interrupt Enabled
                                   // bit     0, TEN, 1 = Timer Enabled
}
