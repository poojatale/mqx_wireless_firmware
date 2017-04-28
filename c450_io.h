#ifndef __c450_IO_h__
#define __c450_IO_h__


#define ON  TRUE
#define OFF FALSE


//  Port B9  = Expansion Module Reset
//  Port B10 = Serial Address to Expansion Modules
//  Port C1  = Transmit Enable, transmit active High
//  Port C5  = Heartbeat  LED
//  Port C13 = Sensor 1, Resistive vs Voltage Select
//  Port C14 = Sensor 2, Resistive vs Voltage Select
//  Port C15 = Sensor 3, Resistive vs Voltage Select
//  Port E2  = Power Control - Terminal Board
//  Port E3  = EEPROM Write Enable

#define EXP_RESET_HIGH     GPIOB_PCOR = 0x00000200  // Set   Port B9
#define EXP_RESET_LOW      GPIOB_PSOR = 0x00000200  // Clear Port B9

#define SERIAL_ADDR_HIGH   GPIOB_PCOR = 0x00000400  // Set   Port B10
#define SERIAL_ADDR_LOW    GPIOB_PSOR = 0x00000400  // Clear Port B10

#define XMIT_ENABLE        GPIOC_PSOR = 0x00000002  // Set   Port C1
#define XMIT_DISABLE       GPIOC_PCOR = 0x00000002  // Clear Port C1

#define HEARTBEAT_LED_ON   GPIOC_PCOR = 0x00000020  // Clear Port C5
#define HEARTBEAT_LED_OFF  GPIOC_PSOR = 0x00000020  // Set   Port C5
#define HEARTBEAT_LED_TGL  GPIOC_PTOR = 0x00000020  // Tgl   Port C5

#define SN1_RESISTIVE_INPUT  GPIOC_PSOR = 0x00002000  // Set   Port C13
#define SN1_VOLTAGE_INPUT    GPIOC_PCOR = 0x00002000  // Clear Port C13

#define SN2_RESISTIVE_INPUT  GPIOC_PSOR = 0x00004000  // Set   Port C14
#define SN2_VOLTAGE_INPUT    GPIOC_PCOR = 0x00004000  // Clear Port C14

#define SN3_RESISTIVE_INPUT  GPIOC_PSOR = 0x00008000  // Set   Port C15
#define SN3_VOLTAGE_INPUT    GPIOC_PCOR = 0x00008000  // Clear Port C15

#define TERMINAL_PWR_ON    GPIOE_PCOR = 0x00000004  // Clear Port E2 to power terminal board
#define TERMINAL_PWR_OFF   GPIOE_PSOR = 0x00000004  // Set   Port E2 to turn power off

#define EEPROM_WR_ON       GPIOE_PCOR = 0x00000008  // Set   Port E3
#define EEPROM_WR_OFF      GPIOE_PSOR = 0x00000008  // Clear Port E3

#define PHY_RESET_ACTIVE   GPIOB_PCOR = 0x00000800  // Set   Port B11
#define PHY_RESET_INACTIVE GPIOB_PSOR = 0x00000800  // Clear Port B11
#define PHY_RESET_TGL      GPIOB_PTOR = 0x00000800  // Tgl   Port B11

#define SPEED_LED_ON       GPIOB_PCOR = 0x00010000  // Clear Port B16
#define SPEED_LED_OFF      GPIOB_PSOR = 0x00010000  // Set   Port B16
#define SPEED_LED_TGL      GPIOB_PTOR = 0x00010000  // Tgl   Port B16



void    initialize_IO(void);

#endif