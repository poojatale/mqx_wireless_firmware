/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : periodic_events.h

PURPOSE   : This header file contains definitions for a number of period
            events that are used to trigger different tasks, and functions
            within those tasks.

            Much of what the firmware does is in response to periodic 
            time intervals. These periodic events are largely controlled
            by the Periodic Interval Timer 0 (PIT0).

            When the intervals occur, tasks are triggered by using the 
            MQX component; lightweight event. Events cause tasks to
            wake up, or become active.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __periodic_events_inc
#define  __periodic_events_inc

#define  UI_KEYSTROKE_EVENT              0x0001
#define  UI_LCD_UPDATE_EVENT             0x0002

#define  CTRL_ALGORITHM_EVENT            0x0001
#define  CTRL_CORE_UPDATE_EVENT          0x0002

#define  MODBUS_MSG_COMPLETE_EVENT       0x0001
#define  MODBUS_SETUP_CHANGE_EVENT       0x0002
#define  MODBUS_ONE_SECOND_EVENT         0x0004

#define  ENET_RESET_EVENT                0x0001

#define  ADC_SAMPLE_CYCLE_COMPLETE_MASK  0x0001
#define  ADC_START_SAMPLE_CYCLE_MASK     0x0002

extern LWEVENT_STRUCT   eventControlTask;
extern LWEVENT_STRUCT   eventSensorTask;
extern LWEVENT_STRUCT   eventUiTask;
#ifdef RS485_HARDWARE
extern LWEVENT_STRUCT   eventModbusTask;
#else
extern LWEVENT_STRUCT   eventEnetHelpTask;
#endif

void init_event_handlers( void );

void init_task_schedule_timer( void );

#endif