/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : sensors.h

PURPOSE   : Function prototypes for "sensors.c" module.                  

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#ifndef  __sensors_inc
#define  __sensors_inc

// The CONVERT_FACTORS data structure holds values that are used during
//    the conversion process to generate a sensor value in engineering 
//    units, and to identify sensor error conditions (out of range).
//
// The Conversion Factors are not used with passive sensors (temperature).
//    They only apply to voltage inputs (pressure and humidity).
//  
// For all of the voltage type sensors there is a linear relationship
//    between the voltage at the sensor wiring terminal and the sensed
//    value expressed in engineering units. To use these conversion  
//    factors it is first necessary to determine the voltage at the
//    wiring terminal.
//
// Ratiometric Sensors: these sensor types are powered by the 5V ext
//    terminal. The engineering units of a ratiometric sensors is a
//    function of not only the voltage at the wiring terminal, but
//    also the voltage supplied by the C450 to the sensor. The voltage
//    supplied by the C450 is measured at the time of manufacture and
//    is part of the calibration data;
//    
//              CalData.five_volt_external
//    
//    A typical ratiometric sensor will generate a voltage in the range
//    of 0.5 to 4.5 when it is supplied with 5vdc.
//
//  fail_low and fail_high: These values represent the limits that if 
//    exceeded, result in a sensor failure. They are expressed in terms
//    of the "signal" which is an intermediary value between ADC counts
//    and engineering units. As part of the sensor conversion routines,
//    the ADC count is converted to either a resistance (ohms) or a 
//    voltage (vdc) depending on the sensor type. This is the "signal".
//
typedef struct
{
    double  min_eng_units;      // Min sensor value in engineering units
    double  max_eng_units;      // Max sensor value in engineering units
    double  signal_fail_low;    // Signal fail low point
    double  signal_fail_high;   // Signal fail high point
    bool    ratiometric;        // Ratiometric - T/F

}  CONVERT_FACTORS;

//
//    Function Prototypes
//
void    sensor_eng_units( SENSOR * sensor, CALIBRATION * cal, uint16_t raw_adc, int sensor_id ); 
float   calc_cpu_temp( uint16_t raw_adc );
float   calc_reference_voltage( uint16_t raw_adc );
void    update_differential_sensor( SENSOR * sensor );                   
void    update_high_signal_sensor( SENSOR * sensor );
double  adc_to_resistance( uint16_t adc_count );
double  adc_to_voltage( uint16_t adc_count );
double  a99_resistance_to_temp( double resistance );
double  nickel_resistance_to_temp( double resistance );
void    percent_rh(  SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    inwc_p_0pt25( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    inwc_p_0pt5( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    inwc_p_2pt5( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    inwc_p_5(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    bar_p_8(     SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    inwc_p_10(   SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    bar_p_15(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    bar_p_30(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    bar_p_50(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    psi_p100(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    psi_p110(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    psi_p500(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    psi_p750(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
void    psi_p200(    SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );

float   signal_to_eng_units( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply );
int     psi_to_hg(   SENSOR * sensor );

uint8_t get_sensor_type( SENSOR * sensor, uint8_t sensor_id );

void    sensor_int_to_float( int16_t  sens_int,   
                             uint8_t  sens_type, 
                             float *  sens_float );

void    sensor_float_to_int( float     sens_float,   
                             uint8_t   sens_type, 
                             int16_t * sens_int );
                           
void    limit_sensor_range( SENSOR * sensor );

uint8_t resistive_input( uint8_t sensor_type );
int     round_to_two( int num );
int     round_to_five( int num );

#endif