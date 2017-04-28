/***************************************************************************
(C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
the document, except as permitted by the License Agreement, is prohibited.

FILENAME  : sensors.c

PURPOSE   : This modules contains a number of routines that are used to
            convert raw ADC sensor readings into engineering units. The
            System 450 supports multiple sensors, each with their own
            response characteristics, and required conversion routines.

            When converting raw data to engineering units, two versions of
            the converted value are generated. One is an integer, the
            other a floating point value.

History:
Date        Author     Rel      EC#    Prob#  Task# Reason for change
---------   --------- ------- ------- ------- ----- -------------------------
*****************************************************************************/

#include "defines.h"
#include "global.h" 
#include "sensors.h" 
#include <math.h>
#include <complex.h>

//   Each of the supported sensors has a range of values that it should
//   respond to. In most cases, we identify sensor readings outside of
//   those ranges as a "sensor failure condition". The sensor failure
//   condition is declared for the temperature and humidity sensors 
//   when the sensor value, expressed as engineering units, is oustide
//   of a given range. For the pressure sensors, the sensor failure
//   condition is declared when the raw ADC value is out of range.
//
//   The following set of declarations define the range of expected
//   sensor readings. Values outside of these ranges result in a 
//   sensor failure.
//
#define  DEG_F_FAIL_LOW      -46    // This value is engineering units, degrees F
#define  DEG_F_FAIL_HIGH     255    // This value is engineering units, degrees F

#define  DEG_C_FAIL_LOW      -44    // This value is engineering units, degrees F
#define  DEG_C_FAIL_HIGH     124    // This value is engineering units, degrees F

#define  DEG_HIGH_F_FAIL_LOW 1000   // Fail low point, ADC counts

#define  RH_FAIL_LOW           1    // This value is engineering units, 1% rH

#define  P_0pt5_FAIL_HIGH  28000    // Fail high point, ADC counts
#define  P_2pt5_FAIL_HIGH  28000    // Fail high point, ADC counts
#define  P_5_FAIL_HIGH     28000    // Fail high point, ADC counts

#define  P_8_FAIL_LOW       1000    // Fail low point, ADC counts
#define  P_8_FAIL_HIGH     28000    // Fail high point, ADC counts

#define  P_10_FAIL_HIGH    28000    // Fail high point, ADC counts

#define  P_15_FAIL_LOW      1000    // Fail low point, ADC counts
#define  P_15_FAIL_HIGH    28000    // Fail high point, ADC counts

#define  P_30_FAIL_LOW      1000    // Fail low point, ADC counts
#define  P_30_FAIL_HIGH    28000    // Fail high point, ADC counts

#define  P_50_FAIL_LOW      1000    // Fail low point, ADC counts
#define  P_50_FAIL_HIGH    28000    // Fail high point, ADC counts

#define  P_100_FAIL_LOW     1000    // Fail low point, ADC counts
#define  P_100_FAIL_HIGH   28000    // Fail high point, ADC counts

#define  P_110_FAIL_LOW     1000    // Fail low point, ADC counts
#define  P_110_FAIL_HIGH   28000    // Fail high point, ADC counts

#define  P_200_FAIL_LOW     1000    // Fail low point, ADC counts
#define  P_200_FAIL_HIGH   28000    // Fail high point, ADC counts

#define  P_500_FAIL_LOW     1000    // Fail low point, ADC counts
#define  P_500_FAIL_HIGH   28000    // Fail high point, ADC counts

#define  P_750_FAIL_LOW     1000    // Fail low point, ADC counts
#define  P_750_FAIL_HIGH   28000    // Fail high point, ADC counts


// The binary sensor type is implemented as a resistive input. It is
//   assumed that the binary input is the sensing of a contact closure.
// If the contact is Open, the resistance is large (about 4 kohm).
// If the contact is Closed, the resistance is small.
//
// The Binary sensor transitions between Open <=> Closed this way;
//
//  IF sensor is Open AND resistance <= 1500, sensor = Closed.
//
//  IF sensor is Closed AND resistance >= 2500, sensor = Open.
//
#define  OHMS_BINARY_OPEN   2500
#define  OHMS_BINARY_CLOSED 1500


// The Sensor Min\Max data structure is used as part of the conversion
//    process. One of the last steps of all conversions is to ensure
//    that an out-of-range value is never reported as the current
//    sensor status.
//
typedef struct 
{
    int16_t min_value_int;    // Minimum sensor value, eng. units, type = int
    int16_t max_value_int;    // Maximum sensor value, eng. units, type = int
    float   min_value_float;  // Minimum sensor value, eng. units, type = float
    float   max_value_float;  // Maximum sensor value, eng. units, type = float
}     SENSOR_MIN_MAX;

// The following table contains the minimum and maximum values that
//    are permitted for each of the sensor types.
//  
const SENSOR_MIN_MAX  SensorMinMax[] =
{
{    0,    0,   0.0,   0.0 },   // SENSOR_TYPE_NONE
{  -46,  255, -46.0, 255.0 },   // SENSOR_TYPE_TEMP_F
{ -430, 1240, -43.0, 124.0 },   // SENSOR_TYPE_TEMP_C, int value = float x 10
{    1,  100,   1.0, 100.0 },   // SENSOR_TYPE_RH
{    0,  500,   0.0,   0.5 },   // SENSOR_TYPE_P_0pt5, int value = float x 1000
{ -100,  800,  -1.0,   8.0 },   // SENSOR_TYPE_P__8,   int value = float x  100
{    0, 1000,   0.0,  10.0 },   // SENSOR_TYPE_P_10,   int value = float x  100
{  -10,  150,  -1.0,  15.0 },   // SENSOR_TYPE_P_15,   int value = float x   10
{    0,  300,   0.0,  30.0 },   // SENSOR_TYPE_P_30,   int value = float x   10
{    0,  500,   0.0,  50.0 },   // SENSOR_TYPE_P_50,   int value = float x   10
{    0, 1000,   0.0, 100.0 },   // SENSOR_TYPE_P100,   int value = float x   10
{    0,  500,   0.0, 500.0 },   // SENSOR_TYPE_P500
{    0,  750,   0.0, 750.0 },   // SENSOR_TYPE_P750
{    0,  200,   0.0, 200.0 },   // SENSOR_TYPE_P200
{    0,  250,   0.0,   2.5 },   // SENSOR_TYPE_P_2pt5,    int value = float x 100
{    0,  500,   0.0,   5.0 },   // SENSOR_TYPE_P_5,       int value = float x 100
{  -50,  360, -50.0, 360.0 },   // SENSOR_TYPE_TEMP_HI_F,
{ -455, 1820, -45.5, 182.0 },   // SENSOR_TYPE_TEMP_HI_C, int value = float x 10
{ -100, 1000, -10.0, 100.0 },   // SENSOR_TYPE_P110,      int value = float x 10
{    0,    1,   0.0,   1.0 },   // SENSOR_TYPE_BINARY,    int value = float x 1
{ -250,  250, -0.25,  0.25 }    // SENSOR_TYPE_P_0pt25, int value = float x 1000
};


const CONVERT_FACTORS 
 Convert[] = {
//                   signal    signal
// min eng  max eng  fail low  fail high  ratiometric
{    0.0,      0.0,    0.00,      0.00,   FALSE },  // SENSOR_TYPE_NONE
{  -46.0,    255.0,  550.00,   2100.00,   FALSE },  // SENSOR_TYPE_TEMP_F
{  -43.0,    124.0,  550.00,   2100.00,   FALSE },  // SENSOR_TYPE_TEMP_C
{    0.0,    100.0,    0.05,      5.25,   FALSE },  // SENSOR_TYPE_RH
{    0.0,      0.5,  -10.00,      5.25,   FALSE },  // SENSOR_TYPE_P_0pt5
{   -1.0,      8.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P__8
{    0.0,     10.0,  -10.00,      5.25,   FALSE },  // SENSOR_TYPE_P_10
{   -1.0,     15.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P_15
{    0.0,     30.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P_30
{    0.0,     50.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P_50
{    0.0,    100.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P100
{    0.0,    500.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P500
{    0.0,    750.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P750
{    0.0,    200.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P200
{    0.0,      2.5,  -10.00,      5.25,   FALSE },  // SENSOR_TYPE_P_2pt5
{    0.0,      5.0,  -10.00,      5.25,   FALSE },  // SENSOR_TYPE_P_5
{  -50.0,    360.0,  625.00,   2200.00,   FALSE },  // SENSOR_TYPE_TEMP_HI_F
{  -45.5,    182.0,  625.00,   2200.00,   FALSE },  // SENSOR_TYPE_TEMP_HI_C
{  -10.0,    100.0,    0.25,      4.75,   TRUE  },  // SENSOR_TYPE_P110
{    0.0,      1.0,  -10.00,  10000.00,   TRUE  },  // SENSOR_TYPE_BINARY
{  -0.25,     0.25,  -10.00,      5.25,   FALSE }   // SENSOR_TYPE_P_0pt25
};


//
//  sensor_eng_units() - This routine converts the raw ADC reading
//                       into engineering units.
//                     
//                       For each sensor type two values are 
//                       generated. One is an integer value, the
//                       other is floating point.
//
//                       The Integer value is used for display,
//                       as an input to relay control algorithm,
//                       and an equivalent value is used to enter
//                       and store setpoint values. Different
//                       sensors will scale the Integer value by
//                       various amounts. For instance, the 
//                       DPT-2005 pressure sensor (SENSOR_TYPE_P_05)
//                       records the pressure of 0.025 INWC as 25.
//
//                       The Floating Point value used for
//                       generating the Integer value, and as
//                       an input to the analog control algorithm.
//
//                       The temperature sensors support a user
//                       supplied offset. This offset is used
//                       here as part of the conversion.
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - pointer to a sensor structure.
//                                  This structure identifies the type
//                                  of sensor and is used to select
//                                  the appropriate conversion routine.
//                                  This structure also contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//                        raw_adc - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
sensor_eng_units( SENSOR * sensor, CALIBRATION * cal, uint16_t raw_adc, int sensor_id )
{
    const CONVERT_FACTORS * cnvt;
    int                     cal_adc, offset;   
    uint16_t                adc_low, adc_high;
    uint8_t                 res_input;
    float                   ftemp;

    res_input = resistive_input( sensor->setup.sensor_type );

    if( res_input )
    {
        switch( sensor_id )
        {
            case SENSOR_ID_ONE:   offset = cal->resistive_offset_1;  break;
            case SENSOR_ID_TWO:   offset = cal->resistive_offset_2;  break;
            case SENSOR_ID_THREE: offset = cal->resistive_offset_3;  break;
            default:              offset = 0;                           break;
        }

        cal_adc = (int) raw_adc;      // Convert unsigned 16 to signed 32 bit
        cal_adc = cal_adc + offset;   // Apply the Calibration Offset 

        if( cal_adc < 0 )             // Disallow negative ADC values.
            cal_adc = 0;  

        if( cal_adc > 65535 )
            cal_adc = 65535;

        sensor->signal = adc_to_resistance( (uint16_t) cal_adc );
    }
    else
    {
        switch( sensor_id )
        {
            case SENSOR_ID_ONE:   
                adc_low  = cal->volt_adc_ground_1;  
                adc_high = cal->volt_adc_5Vext_1;  
            break;

            case SENSOR_ID_TWO:
                adc_low  = cal->volt_adc_ground_2;  
                adc_high = cal->volt_adc_5Vext_2;  
            break;

            case SENSOR_ID_THREE:
                adc_low  = cal->volt_adc_ground_3;  
                adc_high = cal->volt_adc_5Vext_3;  
            break;

            default:
                adc_low  = 0;
                adc_high = 28254;
            break;
        }

        if( raw_adc < adc_low )
            sensor->signal = 0.0;
        else
        {
            // Calculate the voltage (signal) at the sensor wiring terminal
            if( adc_high != adc_low )  // Prevent a divide by zero error
            {
                ftemp = (float) (adc_high - adc_low);

                ftemp = (float) (raw_adc - adc_low) / ftemp;

                ftemp = ftemp * cal->five_volt_external;
            }
            else
                ftemp = 0.0;

            sensor->signal = ftemp;
        }
    }

    switch( sensor->setup.sensor_type )   // Select conversion routine 
    {                                     //    based on sensor type.
        case SENSOR_TYPE_TEMP_F:          // A99 sensor, units = degrees F
            cnvt                = &Convert[ SENSOR_TYPE_TEMP_F ];
            sensor->value_float = a99_resistance_to_temp( sensor->signal );    

                                          // Convert from deg C to deg F
            sensor->value_float = (sensor->value_float * 1.8) + 32.0;
            
            if( sensor->setup.offset != 0 )  // Apply offset, if non-zero
            {
                sensor->value_float += (float) sensor->setup.offset;
            }
        break;

        case SENSOR_TYPE_TEMP_HI_F:       // High Temp sensor, units = degrees F
            cnvt                = &Convert[ SENSOR_TYPE_TEMP_HI_F ];
            sensor->value_float = nickel_resistance_to_temp( sensor->signal );    

                                          // Convert from deg C to deg F
            sensor->value_float = (sensor->value_float * 1.8) + 32.0;
            
            if( sensor->setup.offset != 0 )  // Apply offset, if non-zero
            {
                sensor->value_float += (float) sensor->setup.offset;
            }
        break;

        
        case SENSOR_TYPE_TEMP_C:          // A99 sensor, units = degrees C
            cnvt                = &Convert[ SENSOR_TYPE_TEMP_C ];
            sensor->value_float = a99_resistance_to_temp( sensor->signal );    
            
            if( sensor->setup.offset != 0 )  // Apply offset, if non-zero
            {
                sensor->value_float += ((float) sensor->setup.offset) / 10.0;
            }
        break;

        case SENSOR_TYPE_TEMP_HI_C:       // High Temp sensor, units = degrees C
            cnvt                = &Convert[ SENSOR_TYPE_TEMP_HI_C ];
            sensor->value_float = nickel_resistance_to_temp( sensor->signal );   
 
            if( sensor->setup.offset != 0 )  // Apply offset, if non-zero
            {
                sensor->value_float += ((float) sensor->setup.offset) / 10.0;
            }
        break;
        
        case SENSOR_TYPE_RH:                // Relative humidity, units = %RH
            cnvt                = &Convert[ SENSOR_TYPE_RH ];
            percent_rh(  sensor, cnvt, cal->five_volt_external );
        break;

        case SENSOR_TYPE_P_0pt25:            // Pressure, units = INWC
            cnvt  = &Convert[ SENSOR_TYPE_P_0pt25 ];
            inwc_p_0pt25( sensor, cnvt, cal->five_volt_external );
        break;

        case SENSOR_TYPE_P_0pt5:            // Pressure, units = INWC
            cnvt  = &Convert[ SENSOR_TYPE_P_0pt5 ];
            inwc_p_0pt5( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P_2pt5:            // Pressure, units = INWC
            cnvt  = &Convert[ SENSOR_TYPE_P_2pt5 ];
            inwc_p_2pt5( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P_5:               // Pressure, units = INWC
            cnvt  = &Convert[ SENSOR_TYPE_P_5 ];
            inwc_p_5( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P__8:              // Pressure, units = bAR
            cnvt  = &Convert[ SENSOR_TYPE_P__8 ];
            bar_p_8( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P_10:              // Pressure, units = INWC
            cnvt  = &Convert[ SENSOR_TYPE_P_10 ];
            inwc_p_10( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P_15:              // Pressure, units = bAR
            cnvt  = &Convert[ SENSOR_TYPE_P_15 ];
            bar_p_15( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P_30:              // Pressure, units = bAR
            cnvt  = &Convert[ SENSOR_TYPE_P_30 ];
            bar_p_30( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P_50:              // Pressure, units = bAR
            cnvt  = &Convert[ SENSOR_TYPE_P_50 ];
            bar_p_50( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P100:              // Pressure, units = PSI
            cnvt  = &Convert[ SENSOR_TYPE_P100 ];
            psi_p100( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P110:              // Pressure, units = PSI
            cnvt  = &Convert[ SENSOR_TYPE_P110 ];
            psi_p110( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P200:              // Pressure, units = PSI
            cnvt  = &Convert[ SENSOR_TYPE_P200 ];
            psi_p200( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P500:              // Pressure, units = PSI
            cnvt  = &Convert[ SENSOR_TYPE_P500 ];
            psi_p500( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_P750:              // Pressure, units = PSI
            cnvt  = &Convert[ SENSOR_TYPE_P750 ];
            psi_p750( sensor, cnvt, cal->five_volt_external );
        break;
        
        case SENSOR_TYPE_BINARY:            // Open or Closed, units = n/a
            // Binary inputs never generate a sensor failure.
            //
            //  IF sensor is Open AND resistance <= 1500, sensor = Closed.
            //
            //  IF sensor is Closed AND resistance >= 2500, sensor = Open.
            //
            if( sensor->value_int == BIN_SENSOR_OPEN )
            {
                if( sensor->signal <= OHMS_BINARY_CLOSED )
                {
                    sensor->value_int   = BIN_SENSOR_CLOSED;
                    sensor->value_float = (float) BIN_SENSOR_CLOSED;
                }
                else
                {
                    sensor->value_int   = BIN_SENSOR_OPEN;
                    sensor->value_float = (float) BIN_SENSOR_OPEN;
                }
            }
            else   // Else sensor is currently "Closed"
            {
                if( sensor->signal >= OHMS_BINARY_OPEN )
                {
                    sensor->value_int   = BIN_SENSOR_OPEN;
                    sensor->value_float = (float) BIN_SENSOR_OPEN;
                }
                else
                {
                    sensor->value_int   = BIN_SENSOR_CLOSED;
                    sensor->value_float = (float) BIN_SENSOR_CLOSED;
                }
            }

            sensor->fail = FALSE;
        break;

        default:
            cnvt  = &Convert[ SENSOR_TYPE_NONE ];
            sensor->value_int   = 0;
            sensor->value_float = 0.0;
        break;
    }

    if( sensor->setup.sensor_type != SENSOR_TYPE_BINARY )
    {
        if( sensor->signal < cnvt->signal_fail_low )
            sensor->fail = TRUE;
        else if( sensor->signal > cnvt->signal_fail_high )
            sensor->fail = TRUE;
        else
            sensor->fail = FALSE;

        if( sensor->fail == TRUE )
        {
            sensor->value_float = 0;
            sensor->value_int   = 0;
        }
        else
        {
            sensor_float_to_int( sensor->value_float, 
                                 sensor->setup.sensor_type, 
                                 &sensor->value_int );
        }

        // Make sure that the calculated value remains within a pre-determined
        // range of sensed values (different range for each sensor type).
        //
        limit_sensor_range( sensor );
    }
}                        


//
//  calc_cpu_temp() - This routine converts the raw ADC reading from the
//                    internal CPU temperature channel, and converts it
//                    to degrees F. The equation for this conversion is
//                    from the Kinetis K60 reference manual. Chapt 34 (ADC).
//
// Global Vars Affected:  None
//
//           Parameters:  raw_adc - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  CPU Temperature
//
float
calc_cpu_temp( uint16_t raw_adc )
{
    float  cpu_temp, vtemp;

    // First, calculate the voltage, in millivolts, that corresponds
    //    to the ADC count. The voltage reference is 3.3V (3,300 mV)
    //
    vtemp = ((float) raw_adc * 3300) / 65535;

    // Second, use the equation in the Freescale databook to convert
    //    the voltage to a temperature.
    //
    //  Temp (C) = 25 - ( ( Vtemp - Vtemp25) / m )
    //
    //  Where : Vtemp25 = 716 mV             (from sub-family data sheet)
    //                m = 1.62 mv / deg C    (from sub-family data sheet)
    //            Vtemp = adc reading converted to millivolts
    //
    cpu_temp = 25 - ((vtemp - 716) / 1.62);

    // Convert to degrees Fahrenheit
    cpu_temp = (cpu_temp * 1.8) + 32;

    return( cpu_temp );
}


//
//  calc_reference_voltage() - This routine converts the raw ADC reading of
//                             the of 5V ext or 10V reference to a float value.
//                             The units of the result are vdc.
//  Conversion 
//     Routine: The 5V ext and 10v reference go through a voltage divider that
//              has a gain of 0.2587 (10v -> 2.587v, 5v -> 1.29v). The ADC 
//              compares this voltage to the 3v analog reference voltage, 
//              using a 16 bit conversion (3v -> 65,535). The net result  
//              is that;
//
//              ADC = [(Vin)(0.2587)(65,535)] / 3
//
//              Solving for Vin;
//
//              Vin = (ADC)(1.769618e-4)
//
// Global Vars Affected:  None
//
//           Parameters:  raw_adc - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  reference voltage
//
float   
calc_reference_voltage( uint16_t raw_adc )
{
    float reply;

    reply = ((float) raw_adc) * 1.769618e-4;

    return( reply );
}


//
//  update_differential_sensor() - This routine updates the content of
//            the virtual sensor; "Sn-d", based on the content and setup of
//            Sn-1 and Sn-2.
//
//            If Sn-1 and Sn-2 are of the same type, then
//
//                 Sn-d  =  (Sn-1)  -  (Sn-2)
//
void  
update_differential_sensor( SENSOR * sensor )
{
    // Determine the sensor type for sensor Sn-d
    if(    (sensor[SENSOR_ID_ONE].setup.sensor_type == sensor[SENSOR_ID_TWO].setup.sensor_type)
        && (sensor[SENSOR_ID_ONE].setup.sensor_type != SENSOR_TYPE_BINARY) )
    {
	sensor[SENSOR_ID_DIFF].setup.sensor_type = sensor[SENSOR_ID_ONE].setup.sensor_type;
    }
    else
    {
	sensor[SENSOR_ID_DIFF].setup.sensor_type = SENSOR_TYPE_NONE;
    }

    // Determine the status for sensor Sn-d
    if( sensor[ SENSOR_ID_DIFF ].setup.sensor_type != SENSOR_TYPE_NONE )
    {
	sensor[SENSOR_ID_DIFF].value_float = sensor[SENSOR_ID_ONE].value_float - sensor[SENSOR_ID_TWO].value_float;
	sensor[SENSOR_ID_DIFF].value_int   = sensor[SENSOR_ID_ONE].value_int   - sensor[SENSOR_ID_TWO].value_int;

        if( sensor[SENSOR_ID_ONE].fail || sensor[SENSOR_ID_TWO].fail )
            sensor[SENSOR_ID_DIFF].fail	= TRUE;
	else
            sensor[SENSOR_ID_DIFF].fail	= FALSE;

        if( sensor[SENSOR_ID_DIFF].fail )
        {
	    sensor[SENSOR_ID_DIFF].value_float = 0;
     	    sensor[SENSOR_ID_DIFF].value_int   = 0; 
        }
    }
    else
    {
	sensor[SENSOR_ID_DIFF].value_float = 0;
     	sensor[SENSOR_ID_DIFF].value_int   = 0; 
        sensor[SENSOR_ID_DIFF].fail	   = FALSE;
    }
}


//
//  update_high_signal_sensor() - 
//
//     This function supports two special cases; "High Signal Select 2"
//     and "High Signal Select 3".
//
//     All outputs reference a sensor. Two of the options supported
//     by this firmware are to reference the higher value of sensors
//     Sn-1 and Sn-2, or the higher of Sn-1, 2, and 3. This is
//     permitted if all of these sensors are of the same type. This
//     special case is supported by including elements in the sensor 
//     array that will contain this higher value. This function 
//     must be routinely  called in order to maintain the high value. It 
//     should be called shortly before calling the relay and analog output
//     control functions.
//
// Global Vars Affected:  core database - by way of a pointer, and 
//                                        routine synchronization.
//
//           Parameters:  None
//
//              Returns:  None
//
void update_high_signal_sensor( SENSOR * sensor )
{
    int value, sensor_id, k;

    // Determine the sensor type for sensor HI-2
    if(    (sensor[SENSOR_ID_ONE].setup.sensor_type == sensor[SENSOR_ID_TWO].setup.sensor_type)
        && (sensor[SENSOR_ID_ONE].setup.sensor_type != SENSOR_TYPE_BINARY) )
    {
	sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].setup = sensor[ SENSOR_ID_ONE ].setup;
    }
    else
    {
	sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].setup.sensor_type = SENSOR_TYPE_NONE;
    }		
	
    // Determine the sensor type for sensor HI-3
    if(    (sensor[SENSOR_ID_ONE].setup.sensor_type == sensor[SENSOR_ID_TWO].setup.sensor_type)
	&& (sensor[SENSOR_ID_ONE].setup.sensor_type == sensor[SENSOR_ID_THREE].setup.sensor_type)
        && (sensor[SENSOR_ID_ONE].setup.sensor_type != SENSOR_TYPE_BINARY) )
    {
	sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].setup = sensor[ SENSOR_ID_ONE ].setup;
    }
    else
    {
	sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].setup.sensor_type = SENSOR_TYPE_NONE;
    }		

    // Determine the status for sensor HI-2
    if( sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].setup.sensor_type != SENSOR_TYPE_NONE )
    {
        // Clear the sensor failed condition, for High Signal 2
	//
        sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].fail = 0;
		
	// Look for any failed sensors, and fail the HIGH_SIGNAL_2 sensor 
	//    if any are found.
	//
	for( k=SENSOR_ID_ONE; k<=SENSOR_ID_TWO; k++ )
	{
	    if( sensor[k].fail )
                sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].fail = 1;
	}
		
	// If all sensors are working, find the highest value
	//
	if(  sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].fail == 0 )
	{
            if( sensor[ SENSOR_ID_ONE ].value_int > sensor[ SENSOR_ID_TWO ].value_int )
                sensor_id = SENSOR_ID_ONE;
	    else
                sensor_id = SENSOR_ID_TWO;
			
            sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].value_int   = sensor[ sensor_id ].value_int;
            sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].value_float = sensor[ sensor_id ].value_float;
	}
        else   // Else, sensor has failed
        {
	    sensor[SENSOR_ID_HIGH_SIGNAL_2].value_float = 0;
     	    sensor[SENSOR_ID_HIGH_SIGNAL_2].value_int   = 0; 
        }
    }
    else      // Else, sensor type = None
    {
	sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].value_float       = 0;
     	sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].value_int         = 0; 
        sensor[ SENSOR_ID_HIGH_SIGNAL_2 ].fail              = 0;
    }

    // Determine the status for sensor HI-3
    if( sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].setup.sensor_type != SENSOR_TYPE_NONE )
    {
        // Clear the sensor failed condition, for High Signal 3
	//
        sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].fail = 0;
		
	// Look for any failed sensors, and fail the HIGH_SIGNAL_3 sensor 
	//    if any are found.
	//
	for( k=SENSOR_ID_ONE; k<=SENSOR_ID_THREE; k++ )
	{
	    if( sensor[k].fail )
                sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].fail = 1;
	}

	// If all sensors are working, find the highest value
	//
	if(  sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].fail == 0 )
	{
            value     = sensor[ SENSOR_ID_ONE ].value_int;
	    sensor_id = SENSOR_ID_ONE;
			
	    for( k=SENSOR_ID_ONE; k<=SENSOR_ID_THREE; k++ )
	    {
		if( sensor[k].value_int > value )
		{
         	    value     = sensor[k].value_int;
		    sensor_id = k;      
		}               
	    }

            sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].value_int   = sensor[ sensor_id ].value_int;
            sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].value_float = sensor[ sensor_id ].value_float;
	}
        else   // Else, sensor has failed
        {
	    sensor[SENSOR_ID_HIGH_SIGNAL_3].value_float = 0;
     	    sensor[SENSOR_ID_HIGH_SIGNAL_3].value_int   = 0; 
        }
    }	
    else      // Else, sensor type = None
    {
	sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].value_float = 0;
     	sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].value_int   = 0; 
        sensor[ SENSOR_ID_HIGH_SIGNAL_3 ].fail        = 0;
    }		
}


//
//   adc_to_resistance() - This routine converts the raw ADC measurement
//                         of a resistive input to the equivalent
//                         value in Ohms.
//
double
adc_to_resistance( uint16_t adc )
{
    double f_adc, resistance, numerator, denominator;

    f_adc       = (double) adc;

    numerator   = (5.398769e-2 * f_adc) / (1 - (4.4989745e-6 * f_adc));

    denominator = 1 - ((5.398769e-2 * f_adc ) / (192900 - (8.6785218e-1 * f_adc)));

    resistance  = numerator / denominator;

    return( resistance );
}


//
//   adc_to_voltage() - This routine converts the raw ADC measurement of
//                      a voltage input to the equivalent value in vdc.
//
//   The value 65,535 corresponds to a full scale output of the ADC.
//   This assumes that the ADC is performing 16 bit single ended conversions.
//
//   The gain of the circuit is 0.2587 (wiring terminal -> ADC input)
//
//   Vin  = voltage at wiring terminal
//   Vadc = voltage at micro, adc input
//   G    = gain = 0.2587, Vadc = (Vin)(G)        
//   Vref = ADC reference votage = 3 vdc
//   
//   ADC  = (65,535) (Vadc / Vref) = (65,535) ([(Vin)(G)] / Vref)
//
//   Vin  = (ADC) [(Vref / 65,535) / G] = (ADC) (1.7695e-4)
//
double  adc_to_voltage( uint16_t adc_count )
{
    double voltage;

    voltage = (double) adc_count * 1.7695e-4;

    return( voltage );
}


//
//  a99_resistance_to_temp() - This routine converts Ohms to degrees C.
//                             It assumes an A99 temperature sensor is
//                             the basis for the resistance value.
//
//     Returns value in degrees C
//
double a99_resistance_to_temp( double resistance )
{
    double temp;

    temp = -5917864296200000 * resistance * resistance;

    temp = temp + (3.74354357948544e20 * resistance);

    temp = temp - 8.32051569027462e22;

    temp = 8.94427190999915 * sqrt(temp);

    temp = 4372759107850 - temp;

    temp = (-69370000 * resistance) + temp;

    temp = temp * (0.5 / ((187400 * resistance) - 11812816157));
            
    return( temp );
}


//
//  nickel_resistance_to_temp() - This routine converts Ohms to degrees C.
//                                It assumes a nickel temperature sensor is
//                                the basis for the resistance value.
//
//     Returns value in degrees C
//
double nickel_resistance_to_temp( double resistance )
{
    double temp;

    temp = -218.903378 + (0.29949393 * resistance);

    temp = temp - (resistance * resistance * 6.8371854e-5);

    temp = temp + (resistance * resistance * resistance * 8.8598307e-9);

    return (temp);
}


//
//   percent_rh() - This routine converts the sensor voltage to
//                  engineering units, %RH, for the humidity sensor.
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = %RH
//              sensor->value_int = %RH
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
percent_rh( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{    
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


void   
inwc_p_0pt25( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//  inwc_p_0pt5() - This routine converts the sensor voltage to
//                  engineering units, INWC, for the DPT-2005
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.005 INWC
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = INWC x 1,000
//              sensor->value_int = INWC
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
inwc_p_0pt5( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//  inwc_p_2pt5() - This routine converts the sensor voltage to
//                  engineering units, INWC, for the DPT-xxxx
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.02 INWC
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = INWC x 100
//              sensor->value_int = INWC
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
inwc_p_2pt5( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}

//
//   inwc_p_5() - This routine converts the sensor voltage to engineering
//                units, INWC, for the DPT-xxxx pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.05 INWC
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = INWC x 100
//              sensor->value_int = INWC
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
inwc_p_5( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//      bar_p_8() - This routine converts the sensor voltage to
//                  engineering units, BAR, for the P499Rxx-401C
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.05 BAR
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = BAR x 100
//              sensor->value_int = BAR
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
bar_p_8( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//    inwc_p_10() - This routine converts the sensor voltage to
//                  engineering units, INWC, for the DPT-2100
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.05 INWC
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = INWC x 100
//              sensor->value_int = INWC
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
inwc_p_10( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     bar_p_15() - This routine converts the sensor voltage to
//                  engineering units, BAR, for the P499Rxx-402C
//                  pressure sensor.
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = BAR x 10
//              sensor->value_int = BAR
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
bar_p_15( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     bar_p_30() - This routine converts the sensor voltage to
//                  engineering units, BAR, for the P499Rxx-404C
//                  pressure sensor.
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = BAR x 10
//              sensor->value_int = BAR
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
bar_p_30( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     bar_p_50() - This routine converts the sensor voltage to
//                  engineering units, BAR, for the P499Rxx-405C
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.2 BAR
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = BAR x 10
//              sensor->value_int = BAR
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
bar_p_50( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     psi_p100() - This routine converts the sensor voltage to
//                  engineering units, PSI, for the P499RxS-101C
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.5 PSI
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = PSI x 10
//              sensor->value_int = PSI
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
psi_p100( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     psi_p110() - This routine converts the sensor voltage to
//                  engineering units, PSI, for the P499RxS-101C
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 0.5 PSI
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = PSI x 10
//              sensor->value_int = PSI
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
psi_p110( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     psi_p200() - This routine converts the sensor voltage to
//                  engineering units, PSI, for the P499RxS-107C
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 2 PSI
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = PSI
//              sensor->value_int = PSI
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
psi_p200( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     psi_p500() - This routine converts the sensor voltage to
//                  engineering units, PSI, for the P499RxS-105C
//                  pressure sensor.
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = PSI
//              sensor->value_int = PSI
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
psi_p500( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}


//
//     psi_p750() - This routine converts the sensor voltage to
//                  engineering units, PSI, for the P499RxS-107C
//                  pressure sensor.
//
//            NOTE: The integer value is rounded to +/- 2 PSI
//
// Global Vars Affected:  None
//
//           Parameters:  sensor  - This structure contains fields
//                                  that will be loaded with the 
//                                  converted value.
//
//            sensor->value_float = PSI
//              sensor->value_int = PSI
//
//                      adc_count - the raw ADC reading is used as
//                                  an input to the conversion routine.
//
//              Returns:  None
//
void   
psi_p750( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    sensor->value_float = signal_to_eng_units( sensor, cnvt, supply );
}

//
//  signal_to_eng_units ()
//
//  This routine converts the "signal" to engineering units. It used
//  with the voltage type sensors (pressure and humidity). These sensors
//  have a linear relationship between the sensor voltage (Vin) and their
//  corresponding engineering units. 
//
//  Non-ratiometric sensors: These sensors generate a voltage that
//      is directly related to the sensed condition.
//      (ie. 0 to 5 Vin <==> 0.0 to 2.5 inwc (P 2.5 sensor type)
// 
//  Ratiometric sensors: These sensors have a linear relationship between
//      the voltage they generate and the sensed condition, however, they
//      are powered by the C450. The actual voltage supplied by the C450
//      is measured during manufacture and stored as a calibration value.
//
//      Ideally, a ratiometric sensor follows this relationship;
//         0.5 to 4.5 Vin <==> Min to Max Eng Units.
//
//      This ideal relationship is scaled to account for the actual
//      voltage that is supplied. 5.0 volts is NOT assumed as the supply.
//
//  The data structure "cnvt" holds the conversion Min/Max engineering units
//      and a flag indicating if the sensor is ratiometric.
//
//
//  This equation uses linear equation in the form of Y = mX + b, where
//      the y-axis is the engineering units, and the x-axis is the 
//      sensor signal value (volts).
//
//  Returns:  The converted value in engineering units, as a float
//            type variable.
//
float   
signal_to_eng_units( SENSOR * sensor, const CONVERT_FACTORS * cnvt, float supply )
{
    float min_v, max_v, result;
    float m, b;

    if( cnvt->ratiometric )
    {
        min_v = 0.5 * (supply / 5.0);
        max_v = 4.5 * (supply / 5.0);
    }
    else
    {
        min_v = 0.0;
        max_v = 5.0;
    }

    if( max_v == min_v )   // This condition should never occur.
    {                       
        result = 0.0;
    }
    else
    {
        // Calculate the slope, "m", as m = (Y2 - Y1) / (X2 - X1)
        //
        m = (cnvt->max_eng_units - cnvt->min_eng_units) / (max_v - min_v);

        // Calculate y-intercept, "b", as b = Y1 - mX1
        //
        b = cnvt->min_eng_units - (m * min_v);

        // Calculate engineering units = Y = mx + b
        //    
        result = (float) ((m * sensor->signal) + b);
    }

    return( result );
}

//
//  psi_to_hg() - This routine converts the floating point representation
//                of a sensor input that represented in PSI, to an integer
//                value in the units of inches of mercury (in. Hg). It
//                further assumes that the PSI value is negative. This
//                negative value is converted to a positive in. Hg value.
//
//                This routine is used with a special case sensor, the P110,
//                which will display a pressure values greater than 0
//                as PSI, and will display values less than 0 as in Hg.
//
int   
psi_to_hg(   SENSOR * sensor )
{
    int  hg;
	
    hg = (int) (sensor->value_float * -2.036);

    return( hg );	
}


//
//  get_sensor_type() - This routine uses a given sensor ID to
//                      determine the type of the sensor.
//
uint8_t
get_sensor_type( SENSOR * sensor, uint8_t sensor_id )
{
    uint8_t sensor_type;

    switch( sensor_id )
    {
        case SENSOR_ID_ONE:  
        case SENSOR_ID_DIFF:
        case SENSOR_ID_HIGH_SIGNAL_2: 
        case SENSOR_ID_HIGH_SIGNAL_3:
            sensor_type = sensor[ SENSOR_ID_ONE ].setup.sensor_type;
        break;

        case SENSOR_ID_TWO:
            sensor_type = sensor[ SENSOR_ID_TWO ].setup.sensor_type;
        break;

        case SENSOR_ID_THREE: 
            sensor_type = sensor[ SENSOR_ID_THREE ].setup.sensor_type;
        break;

        default:
            sensor_type = SENSOR_TYPE_NONE;
        break;
    }

    return( sensor_type );
}


//
// sensor_int_to_float() - This routine converts the integer form of a sensor
//                         value, to its equivalent floating point form.
//
//                         This routine is applied not to sensor reading, but
//                         to setpoints related to a particular sensor type.
//
//                         Setpoints are recorded in their integer form.
//                         When running the analog control algorithm, the
//                         float value of the sensor is used. This requires
//                         a float version of the associated setpoints.
//                         This routine converts the integer version of
//                         those setpoints to float. (ie. Integer setpoint
//                         for DPT-2005 sensor = 25, is converted to a 
//                         float = 0.025).
//
// Global Vars Affected:  Convert[] - referenced for gain and offset
//
//           Parameters:  sens_int  - integer value to be converted. 
//
//                        sens_type - indicates the sensor type, and
//                                    determines which conversion to use.
//
//                       sens_float - floating point result.
//
//              Returns:  None
//
void 
sensor_int_to_float( int16_t  sens_int, 
                     uint8_t  sens_type,
                     float *  sens_float )
{
    *sens_float = (float) sens_int;         // Convert integer to float       
    
    switch( sens_type )                     // Scale as necessary
    {
        case SENSOR_TYPE_TEMP_C:            // A99 sensor, units = degrees C
        case SENSOR_TYPE_TEMP_HI_C:         // Temp sensor, units = degrees C
        case SENSOR_TYPE_P_15:              // Pressure, units = bAR
        case SENSOR_TYPE_P_30:              // Pressure, units = bAR
        case SENSOR_TYPE_P_50:              // Pressure, units = bAR
        case SENSOR_TYPE_P100:              // Pressure, units = PSI
        case SENSOR_TYPE_P110:              // Pressure, units = PSI
            *sens_float = *sens_float / 10.0;
        break;
        
        case SENSOR_TYPE_P_2pt5:            // Pressure, units = INWC
        case SENSOR_TYPE_P_5:               // Pressure, units = INWC
        case SENSOR_TYPE_P__8:              // Pressure, units = bAR
        case SENSOR_TYPE_P_10:              // Pressure, units = INWC
            *sens_float = *sens_float / 100.0;
        break;
        
        case SENSOR_TYPE_P_0pt25:
        case SENSOR_TYPE_P_0pt5:            // Pressure, units = INWC
            *sens_float = *sens_float / 1000.0;
        break;
    }
}


//
// sensor_float_to_int() - This routine converts the floating point form of
//                         a sensor value, to its equivalent integer form.
//
//                         This routine is applied not to sensor reading,
//                         but to setpoints related to a particular
//                         sensor type.
//
//                         Setpoints are recorded in their integer form.
//                         This routine converts the floating point
//                         version of those setpoints to an integer.
//                         (ie. Integer setpoint for DPT-2005 sensor = 25,
//                         its floating point value = 0.025).
//
// Global Vars Affected:  None
//
//           Parameters:  sens_float - floating point value to be converted. 
//
//                        sens_type - indicates the sensor type, and
//                                    determines which conversion to use.
//
//                        sens_int - integer result.
//
//              Returns:  None
//
void   sensor_float_to_int( float     sens_float,
                            uint8_t   sens_type, 
                            int16_t * sens_int )
{
    switch( sens_type )                     // Scale as necessary
    {       
        case SENSOR_TYPE_TEMP_F:            // A99 sensor, units = degrees F
        case SENSOR_TYPE_TEMP_HI_F:         // Temp sensor, units = degrees F
        case SENSOR_TYPE_RH:                // Humidity sensor, units = %rH
        case SENSOR_TYPE_P200:              // Pressure, units = PSI
        case SENSOR_TYPE_P500:              // Pressure, units = PSI
            // Display sensor values and force setpoints => +/- 1
            //
            if( sens_float >= 0 )
                *sens_int = (int) (sens_float + 0.5);
            else        // Some sensors can go negative
                *sens_int = (int) (sens_float - 0.5);
        break;

        case SENSOR_TYPE_P750:              // Pressure, units = PSI
            // Display sensor values and force setpoints => +/- 2
            //
            //   Examples: float => int
            //             70.95 => 70
            //             71.01 => 72
            //
            *sens_int = (int) sens_float;
            *sens_int = round_to_two( *sens_int );
        break;

        case SENSOR_TYPE_TEMP_C:            // A99 sensor, units = degrees C
        case SENSOR_TYPE_TEMP_HI_C:         // Temp sensor, units = degrees C
        case SENSOR_TYPE_P100:              // Pressure, units = PSI
        case SENSOR_TYPE_P110:              // Pressure, units = PSI
            // Display sensor values and force values => +/- 0.5
            //
            // Multiply by 10 and add "0.5" to convert the "43.86" to "438"
            //   ie. i) 43.86 * 10  = 438.6
            //      ii) 438.6 + 0.5 = 439.1
            //     iii) (int) 439.1 = 439
            //
            // Call "round_to_five()" to convert 439 -> 440.
            //
            // End result;  "43.86" => 440, displayed as 44.0
            //
            if( sens_float >= 0 )
                *sens_int = (int) ((sens_float * 10) + 0.5);
            else        // Some sensors can go negative
                *sens_int = (int) ((sens_float * 10) - 0.5);

            *sens_int = round_to_five( *sens_int );
        break;
        
        case SENSOR_TYPE_P_2pt5:            // Pressure, units = INWC
            // Display sensor values and force values => +/- 0.02
            //
            // Multiply by 100 and to convert the "1.368" to "54"            
            //
            //   ie. i)  1.368 * 100 = 136.8
            //      ii) (int) 136.8  = 136
            //
            // Call "round_to_two()" to convert 136 -> 136.
            //
            // End result;  "1.368" => 136, displayed as 1.36
            //
            *sens_int = (int) (sens_float * 100);
            *sens_int = round_to_two( *sens_int );
        break;

        case SENSOR_TYPE_P_5:               // Pressure, units = INWC
        case SENSOR_TYPE_P__8:              // Pressure, units = bAR
        case SENSOR_TYPE_P_10:              // Pressure, units = INWC
            // Display sensor values and force values => +/- 0.05
            //
            // Multiply by 100 and add "0.5" to convert the "4.686" to "469"
            //   ie. i) 4.686 * 100 = 468.6
            //      ii) 468.6 + 0.5 = 469.1
            //     iii) (int) 469.1 = 469
            //
            // Call "round_to_five()" to convert 469 -> 470.
            //
            // End result;  "4.686" => 470, displayed as 4.70
            //
            if( sens_float >= 0 )
                *sens_int = (int) ((sens_float * 100) + 0.5);
            else        // The P_8 can go negative
                *sens_int = (int) ((sens_float * 100) - 0.5);

            *sens_int = round_to_five( *sens_int );
        break;

        case SENSOR_TYPE_P_15:              // Pressure, units = bAR
        case SENSOR_TYPE_P_30:              // Pressure, units = bAR
            // Display sensor values and force values => +/- 0.1
            //
            // Multiply by 10 and add "0.5" to convert the "10.56" to "106"            
            //   ie. i)  10.56 * 10 = 105.6
            //      ii) 105.6 + 0.5 = 106.1
            //     iii) (int) 106.1 = 106
            //
            // End result;  "10.56" => 106, displayed as 10.6
            //
            if( sens_float >= 0 )
                *sens_int = (int) ((sens_float * 10) + 0.5);
            else        // The P_15 can go negative
                *sens_int = (int) ((sens_float * 10) - 0.5);
        break;

        case SENSOR_TYPE_P_50:              // Pressure, units = bAR
            // Display sensor values and force values => +/- 0.2
            //
            // Multiply by 10 and to convert the "5.368" to "54"            
            //
            //   ie. i)  5.368 * 10 = 53.68
            //      ii) (int) 53.68 = 53
            //
            // Call "round_to_two()" to convert 53 -> 54.
            //
            // End result;  "0.0058" => 5, displayed as 0.005
            //
            *sens_int = (int) (sens_float * 10);
            *sens_int = round_to_two( *sens_int );
        break;

        case SENSOR_TYPE_P_0pt25: 
        case SENSOR_TYPE_P_0pt5:            // Pressure, units = INWC
            // Display sensor values and force values => +/- 0.005
            //
            // Multiply by 1000 and add "0.5" to convert the "0.0068" to "7"            
            //   ie. i) 0.0058 * 1000 = 5.8
            //      ii)     5.8 + 0.5 = 6.3
            //     iii)     (int) 6.3 = 6
            //
            // Call "round_to_five()" to convert 6 -> 5.
            //
            // End result;  "0.0058" => 5, displayed as 0.005
            //
            *sens_int = (int) ((sens_float * 1000) + 0.5);
            *sens_int = round_to_five( *sens_int );
        break;

        case SENSOR_TYPE_BINARY:            // Open or Closed, no units
            *sens_int = (int) sens_float;
        break;

        default:
            *sens_int = 0;            // If sensor type = None or invalid
        break;
    }
}

//
//  limit_sensor_range() - This routine forces a converted
//                         sensor value to fall within the
//                         expected range. It is typically
//                         called shortly after the raw ADC
//                         counts have been converted to
//
//                         The range limits are found in
//                         the global array "SensorMinMax[]".
//                         Each sensor type has a unique
//                         range. The sensor type is used to
//                         index into this array.
//
// Global Vars Affected:  SensorMinMax[] - referenced for the
//                                         range limits.
//
//           Parameters:  sensor  - pointer to the sensor struct 
//                                  containing the value to 
//                                  be checked.
//
//              Returns:  None
//
void
limit_sensor_range( SENSOR * sensor )
{
    uint8_t idx;

    idx = sensor->setup.sensor_type;

    if( idx > MAX_SENSOR_TYPE )
        idx = SENSOR_TYPE_NONE;

    if( sensor->value_int < SensorMinMax[ idx ].min_value_int )
    {
        sensor->value_int   = SensorMinMax[ idx ].min_value_int;
        sensor->value_float = SensorMinMax[ idx ].min_value_float;
    }

    if( sensor->value_int > SensorMinMax[ idx ].max_value_int )
    {
        sensor->value_int   = SensorMinMax[ idx ].max_value_int;
        sensor->value_float = SensorMinMax[ idx ].max_value_float;
    }
}


//
//  resistive_input() - This routine determines if the 
//                      indicated sensor is a resistive type.
//                      Resistive types include the 
//                      temperature sensors. The result
//                      of this routine is typically used
//                      when selecting which ADC channel to
//                      convert. One channel is dedicated
//                      to resistive inputs, the other to
//                      voltage inputs.
//
// Global Vars Affected:  None
//
//           Parameters:  sensor_type - indicates type of sensor 
//
//              Returns:  True  - sensor type is resistive
//                        False - sensor type is NOT resistive
//
uint8_t
resistive_input( uint8_t sensor_type )
{
    if( (sensor_type == SENSOR_TYPE_TEMP_F)    || 
        (sensor_type == SENSOR_TYPE_TEMP_C)    ||
        (sensor_type == SENSOR_TYPE_TEMP_HI_F) || 
        (sensor_type == SENSOR_TYPE_TEMP_HI_C) ||
        (sensor_type == SENSOR_TYPE_BINARY)  )
    {
        return( TRUE );
    }
    else
        return( FALSE );    
}


//
//  round_to_five() - Some of the sensor conversions are expected to
//                    produce an integer result where the least significant 
//                    digit is 0 or 5. This routine rounds the least
//                    significant digit to the nearest value of 0 or 5,
//                    whichever is closer.
//
// Global Vars Affected:  None
//
//           Parameters:  num  - value to be rounded 
//
//              Returns:  result
//
int
round_to_five( int num )
{
    long rem, reply;
          
    if( num >= 0 )
    {
        rem   = num % 10;
        reply = num - rem;   // Values 10-19 become 10
    
        if( rem >= 8 )       // Values 18-19 become 20
            reply += 10;
            
        else if( rem >= 3 )  // Values 13-17 become 15
            reply += 5;
                             // Values 10-12 stay as 10
    }
    else
    {
        rem   = (0 - num) % 10;
        reply = num + rem;   // Values -10 to -19 become -10
    
        if( rem >= 8 )       // Values -18 to -19 become -20
            reply -= 10;
            
        else if( rem >= 3 )  // Values -13 to -17 become -15
            reply -= 5;
                             // Values -10 to -12 stay as -10
    }

    return( reply );
}

//
//   round_to_two() - Some of the sensor conversions are expected to
//                    produce an integer result where the least significant 
//                    digit is 0 or 2. This routine rounds the least
//                    significant digit to the nearest value of 0 or 2,
//                    whichever is closer.
//
// Global Vars Affected:  None
//
//           Parameters:  num  :  value to be rounded 
//
//              Returns:  result
//
int
round_to_two( int num )
{
    int rem, reply;
          
    if( num >= 0 )
    {
        rem   = num % 2;        // Value 10 remains as 10
        reply = num + rem;      // Value of 11 becomes 12
    }
    else
    {
        rem   = (0 - num) % 2;  // Value -10 remains as -10
        reply = num - rem;      // Value of -11 becomes -12
    }

    return( reply );
}