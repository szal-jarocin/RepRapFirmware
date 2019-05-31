#ifndef MKSSBASE_H
#define MKSSBASE_H

#include "../PINS_LPC.h"


// List of assignable pins and their mapping from names to MPU ports. This is indexed by logical pin number.
// The names must match user input that has been concerted to lowercase and had _ and - characters stripped out.
// Aliases are separate by the , character.
// If a pin name is prefixed by ! then this means the pin is hardware inverted. The same pin may have names for both the inverted and non-inverted cases,
// for example the inverted heater pins on the expansion connector are available as non-inverted servo pins on a DFueX.

constexpr PinEntry PinTable_MKSSbase[] =
{
    //Thermistors
    {P0_23, PinCapability::ainrw, "th1"},
    {P0_24, PinCapability::ainrw, "th2"},
    {P0_25, PinCapability::ainrw, "th3"},
    {P0_26, PinCapability::ainrw, "th4"},

    //Endstops
    {P1_24, PinCapability::rw, "xmin,x"}, //X-. X as the - gets stripped in RRF3
    {P1_25, PinCapability::rw, "xmax,x+"}, //X+
    {P1_26, PinCapability::rw, "ymin,y"},
    {P1_27, PinCapability::rw, "ymax,y+"},
    {P1_28, PinCapability::rw, "zmin,z"},
    {P1_29, PinCapability::rw, "zmax,z+"},

    //Heaters and Fans
    {P2_5, PinCapability::rwpwm, "bed"},
    {P2_7, PinCapability::rwpwm, "e1"},
    {P2_6, PinCapability::rwpwm, "e2"},
    {P2_4, PinCapability::rwpwm, "fan"},


    //J7
    {P0_17, PinCapability::rw, "p0.17"},
    {P0_16, PinCapability::rw, "p0.16"},
    {P0_14, PinCapability::rw, "p0.14"},
    
    //J8
    {P1_22, PinCapability::rw, "p1.22"},
    {P1_23, PinCapability::rw, "p1.23"},
    {P2_12, PinCapability::rw, "p2.12"},
    {P2_11, PinCapability::rw, "p2.11"},
    {P4_28, PinCapability::rw, "p4.28"},
    
    //Aux-1
    //P0.2
    //P0.3
    
    //Exp1
    {P1_31, PinCapability::rw, "p1.31"},
    {P0_18, PinCapability::rw, "p0.18"},
    {P0_14, PinCapability::rw, "p0.14"},
    {P1_30, PinCapability::rw, "p1.30"},
    {P0_16, PinCapability::rw, "p0.16"},
    
    //Exp2
    {P0_8,  PinCapability::rw, "p0.8"},
    {P3_25, PinCapability::rw, "p3.25"},
    {P3_26, PinCapability::rw, "p3.26"},
    {P0_27, PinCapability::rw, "p0.27"},
    {P0_7,  PinCapability::rw, "p0.7"},
    {P0_28, PinCapability::rw, "p0.28"},
    {P0_9,  PinCapability::rw, "p0.9"},
    
};


constexpr BoardDefaults mkssbaseDefaults = {
    {P0_4,  P0_10, P0_19, P0_21,  P4_29},   //enablePins
    {P2_0,  P2_1,  P2_2,  P2_3,   P2_8},    //stepPins
    {P0_5,  P0_11, P0_20, P0_22,  P2_13},   //dirPins
    true,                                   //currentControl
    113.33,                                 //digiPot Factor
    {P2_5, NoPin, NoPin},                   //slowPWM
    {P2_7, P2_6, NoPin},                    //fastPWM
    {NoPin, NoPin, NoPin},                  //ServoPWM
};

#endif
