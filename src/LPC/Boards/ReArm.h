#ifndef REARM_H
#define REARM_H

#include "../PINS_LPC.h"


// List of assignable pins and their mapping from names to MPU ports. This is indexed by logical pin number.
// The names must match user input that has been concerted to lowercase and had _ and - characters stripped out.
// Aliases are separate by the , character.
// If a pin name is prefixed by ! then this means the pin is hardware inverted. The same pin may have names for both the inverted and non-inverted cases,
// for example the inverted heater pins on the expansion connector are available as non-inverted servo pins on a DFueX.

constexpr PinEntry PinTable_Rearm[] =
{

    //Thermistors
    {P0_23, PinCapability::ainrw, "t0,a13"},        //Ext0 Therm
    {P0_24, PinCapability::ainrw, "t1,a14"},        //Bed Therm
    {P0_25, PinCapability::ainrw, "t2,a15"},        //Ext1 Therm
    
    //Endstops
    {P1_24, PinCapability::read, "xmin,d3,x"},//X-. X as the - gets stripped in RRF3
    {P1_25, PinCapability::read, "xmax,d2,x+"},
    {P1_26, PinCapability::read, "ymin,d14,y"},
    {P1_27, PinCapability::read, "ymax,d15,y+"},
    {P1_29, PinCapability::read, "zmin,d18,z"},
    {P1_28, PinCapability::read, "zmax,d19,z+"},

    //Heaters and Fans (Mosfets)
    {P2_7, PinCapability::wpwm, "d8"  },            //HB Mosfet
    {P2_4, PinCapability::wpwm, "d9" },             //HE2 Mosfet
    {P2_5, PinCapability::wpwm, "d10" },            //HE1 Mosfet

    //Servos (only 1st 3 servos supported as a servo)
    {P1_20, PinCapability::wpwm, "servo0,d11"},     //PWM1[2]
    {P1_21, PinCapability::wpwm, "servo1,d6"},      //PWM1[3]
    {P1_19, PinCapability::wpwm, "servo2,d5"},
    {P1_18, PinCapability::rw, "d4"},               //PWM1[1]
    
    //Ramps  AUX1 Pins
    //P0_2 D1 (TXD0)  (Used by AUX Serial)
    //P0_3 D0 (RXD0)  (Used by Aux Serial)
    {P0_27, PinCapability::rw, "d57"},
    {P0_28, PinCapability::rw, "d58"},

    //Ramps Aux2 Pins
    {P0_26,  PinCapability::ainrw, "a9,d63"},       //(ADC3, DAC)
    //D40,     (NC)
    //D42,     (NC)
    //A11/D65, (NC)
    {P2_6, PinCapability::rw, "D59"},               //a5 (overlaps J3)
    //A10/D64, (NC)
    //D44,     (NC)
    //A12/D66  (NC)
    
    //Ramps Aux3 Pins
    {P0_15, PinCapability::rw, "sck,d52"},
    {P0_17, PinCapability::rw, "miso,d50"},
    {P1_23, PinCapability::rw, "d53"},              //(overlaps J3)
    {P0_18, PinCapability::rw, "mosi,d51"},         //(overlaps Aux4)
    {P1_31, PinCapability::rw, "d49"},
    
    //Ramps Aux4 Pins
    //D32, (NC)
    //D47, (NC)
    //D45, (NC)
    //D43, (NC)
    {P1_22,PinCapability::rw, "d41"},
    //D39, (NC)
    {P1_30, PinCapability::rw, "d37"},
    {P2_11, PinCapability::rw, "d35"},
    {P3_25, PinCapability::rw, "d33"},
    {P3_26, PinCapability::rw, "d31"},
    //D29, (NC)
    //D27, (NC)
    //D25, (NC)
    {P0_15, PinCapability::rw, "d23"},              //(SCK)  (Overlaps Aux3)
    {P0_18, PinCapability::rw, "d17"},              //(MOSI) (overlaps Aux3)
    {P0_16, PinCapability::rw, "d16"},              //(CS)
    
    //Ramps I2C Header
    {P0_0, PinCapability::rw, "sca,d20,20"},        //labelled on board as 20
    {P0_1, PinCapability::rw, "scl,d21,21"},        //labelled on board as 21
    
    
    //ReArm Headers
    //J3
    //P0_15, SCLK (overlaps Aux3/4)
    //P0_16, (Overlaps  Aux4)
    {P1_23, PinCapability::rw,"d53"},               //PWM1[4]
    {P2_11, PinCapability::rw,"d35"},
    {P1_31, PinCapability::rw,"d49"},
    //P0_18, MOSI (overlaps Aux4/Aux3)
    {P2_6,  PinCapability::rw,"d59"},               //(overlaps Aux2)
    //P0_17, MISO (overlaps Aux3)
    {P3_25, PinCapability::rw,"d33"},               //PWM1[2]
    {P3_26, PinCapability::rw,"d31"},               //PWM1[3]
    
    //J5
    {P1_22, PinCapability::rw,"d41"},
    {P1_30, PinCapability::rw,"d37"},
    //P1_21, d6     PWM1[3] //overlaps with Servos
    //P0_26, a9/d63 //Overlaps Aux2

};


constexpr BoardDefaults rearmDefaults = {
    {P0_10, P0_19, P0_21, P0_4, P4_29},     //enablePins
    {P2_1,  P2_2,  P2_3,  P2_0, P2_8},      //stepPins
    {P0_11, P0_20, P0_22, P0_5, P2_13},     //dirPins
    false,                                  //currentControl
    113.33,                                 //digiPot Factor
    {P2_7, NoPin, NoPin},                   //slowPWM
    {NoPin, NoPin, NoPin},                  //fastPWM
    {P1_20, P1_19, P1_21},                  //ServoPWM (Servo1,2,3)
};



#endif
