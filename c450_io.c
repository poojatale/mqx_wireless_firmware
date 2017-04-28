#include "defines.h"
#include <io_gpio.h>
#include "c450_io.h"
//#include "i2cTerminal.h"
//#include "i2cExpModules.h"
#include "global.h"


//
// Initialize GPIO for LED's and push buttons
// This is also where you can set up more external GPIO
//
void initialize_IO(void)
{

/*
    //  Configure GPIO OUTPUT ports, Mux = Alt 1
    //  
    //  Port B9  = Expansion Module Reset Line
    //  Port B10 = Serial Address to Expansion Modules
    //  Port B11 = Phy Reset
    //  Port B16 = Ethernet Speed LED
    //  Port C5  = Hearbeat LED
    //  Port C13 = Select SN-1, Resistive or Voltage
    //  Port C14 = Select SN-2, Resistive or Voltage
    //  Port C15 = Select SN-3, Resistive or Voltage
    //  Port E2  = Power Control - Terminal Board
    //  Port E3  = EEPROM Write Control
    //

    //  PORT_PCR_MUX(1)   = Configure IO as GPIO (general purpose IO)
    //  PORT_PCR_ODE_MASK = Open Drain Enable (outputs only)
    //  PORT_PCR_SRE_MASK = Slew Rate Enable (outputs only)
    //

    // Reset - Exp Modules
    PORTB_PCR9  = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK | PORT_PCR_ODE_MASK; 
    // Address - Exp Modules
    PORTB_PCR10 = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK | PORT_PCR_ODE_MASK;
    //  Phy Reset
    PORTB_PCR11 = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK;
    // Link LED 
    PORTB_PCR16 = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK | PORT_PCR_ODE_MASK | PORT_PCR_DSE_MASK;

    PORTC_PCR5  = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK | PORT_PCR_ODE_MASK;  // Heartbeat LED
    PORTC_PCR13 = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK;  // Select SN-1, Res or Volt
    PORTC_PCR14 = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK;  // Select SN-2, Res or Volt
    PORTC_PCR15 = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK;  // Select SN-3, Res or Volt

    PORTE_PCR2  = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK;  // Power On\Off - Term Board
    PORTE_PCR3  = PORT_PCR_MUX(1) | PORT_PCR_SRE_MASK;  // EEPROM Write Control


    // The following pins are used for analog input. The Port Control
    //    registers did not appear to be loaded with their default
    //    value of "0", as indicated in the Freescale reference manual.
    //
    PORTD_PCR1  = 0;     // Port D1  = AI1_R (resistive input, Sn-1)
    PORTD_PCR5  = 0;     // Port D5  = AI2_R (resistive input, Sn-2)
    PORTD_PCR6  = 0;     // Port D6  = AI3_R (resistive input, Sn-3)
    PORTC_PCR9  = 0;     // Port C9  = AI1_V (voltage input, Sn-1)
    PORTC_PCR10 = 0;     // Port C10 = AI2_V (voltage input, Sn-2)
    PORTC_PCR11 = 0;     // Port C11 = AI3_V (voltage input, Sn-3)

    //   Port B, pins 9, 10, 11 & 16  are set to output.   
    //
    GPIOB_PDDR |= 0x00010E00;

    //   Port C, pins 5, 13, 14 & 15 are set to output.
    //
    GPIOC_PDDR |= 0x0000E020;

    //   Port E, pins 2 and 3 are set to output.
    //
    GPIOE_PDDR |= 0x0000000C;

    SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK; // Gate clock to Port A
    SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK; // Gate clock to Port B
    SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK; // Gate clock to Port C
    SIM_SCGC5 |= SIM_SCGC5_PORTD_MASK; // Gate clock to Port D
    SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK; // Gate clock to Port E
	
    EXP_RESET_HIGH;       // Expansion bus Not in reset
    PHY_RESET_INACTIVE;   // PHY chip Not in reset
    SPEED_LED_OFF;        // Set Ethernet Speed LED = 10 Mhz, LED off
    TERMINAL_PWR_ON;      // Supply power to terminal board
    EEPROM_WR_OFF;        // Protect EEPROM from writes
    HEARTBEAT_LED_OFF;    // Turn heartbeat LED off
    SN1_VOLTAGE_INPUT;    // Set all analog inputs to voltage inputs
    SN2_VOLTAGE_INPUT;
    SN3_VOLTAGE_INPUT;

    SERIAL_ADDR_LOW;      // The serial address line should be LOW when not
                          //   in use, making it less susceptible to noise.



    //  Configure GPIO INPUT ports, Mux = Alt 1
    //  
    //  Port C12 = Ethernet Reset Push Button
    //  Port D4  = Power Good Signal
    //  Port D7  = Keypress Signal
    //


    //  Port C12 = Ethernet Reset Button
    //
    PORTC_PCR12 = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

    // Clear bit 12 of the data direction register, Port C,
    //    making C12 an input.
    //
    GPIOC_PDDR &= 0xFFFFEFFF;   // Clear bit 12


    //  Port D4 = 3.3v Power Good signal
    //
    PORTD_PCR4 = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;

    //  Port D7 = UI Keypress Interrupt
    //
    //   MUX = 1 GPIO,   Pull Enable, Pull Select = Up
    //
    PORTD_PCR7 = PORT_PCR_MUX(1) | PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; 

    // Clear bits 4 and 7 of the data direction register, Port D,
    //    making D4 and D7 inputs.
    //
    GPIOD_PDDR &= 0xFFFFFF6F;   // Clear bits 4 and 7

    // Setup the interrupt settings for D7, the keypress signal
    //
    PORTD_PCR7 |= 0x000A0000;   // Bits 19-16 = Interrupt Configuration
                                //  1010, "A" = Interrupt on falling edge

    // Digital Filter Clock Register,
    //   1 = Digital Filter is clocked by the 1 kHz LPO clock
    //
    PORTD_DFCR |= PORT_DFCR_CS_MASK;

    // Digital Filter Width Register
    //  Bits 0-4 = filter length, number of clock cycles that
    //             a digital input must be stable before
    //             it is accepted (passes filter).
    //
    PORTD_DFWR = 0x00000001;   // 1 clock cycles = 1 mSec @ 1 kHhz LPO Clock

    // Digital Filter Enable Register - Port D7 (keypress signal)
    //   Set the corresponding bit to enable the digital
    //   filter for a specific IO pin. All pins in a given port
    //   share the digital filter clock source and width.
    //
    PORTD_DFER |= 0x00000080;

    // Clear interrupt status flags for Port D7 (keypress signal)
    //
    PORTD_ISFR |= 0x00000080;

*/

    // Initialize I2C-0, used to communicate with the expansion modules
    //
//    init_i2c_exp_modules();

    // Initialize I2C-1, used to communicate with the terminal board
    //    and the EEPROM.
    //
//    init_i2c_terminal();
}
