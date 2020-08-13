#ifndef BIQU_SKR_H
#define BIQU_SKR_H

#include "../Pins_STM32.h"

// List of assignable pins and their mapping from names to MPU ports. This is indexed by logical pin number.
// The names must match user input that has been converted to lowercase and had _ and - characters stripped out.
// Aliases are separate by the , character.
// If a pin name is prefixed by ! then this means the pin is hardware inverted. The same pin may have names for both the inverted and non-inverted cases,
// for example the inverted heater pins on the expansion connector are available as non-inverted servo pins on a DFueX.

constexpr PinEntry PinTable_BIQU_SKR_PRO_v1_1[] =
{
    //Thermistors
    {PF_3, PinCapability::ainrw, "e0temp,t0"},
    {PF_4, PinCapability::ainrw, "e1temp,t1"},
    {PF_5, PinCapability::ainrw, "e2temp,t2"},
    {PF_6, PinCapability::ainrw, "bedtemp,t3"},

    //Endstops
    {PB_10, PinCapability::rwpwm, "xstop,x-stop"},
    {PE_12, PinCapability::rwpwm, "ystop,y-stop"},
    {PG_8, PinCapability::rwpwm, "zstop,z-stop"},
    {PE_15, PinCapability::rwpwm, "e0stop,e0det"},
    {PE_10, PinCapability::rwpwm, "e1stop,e1det"},
    {PG_5, PinCapability::rwpwm, "e2stop,e2det"},
	{PA_2, PinCapability::rwpwm, "probe"},
	
    //Heaters and Fans (Big and Small Mosfets}
    {PD_12,  PinCapability::wpwm, "bed,hbed" },
    {PB_1,  PinCapability::wpwm, "e0heat,he0" },
    {PD_14,  PinCapability::wpwm, "e1heat,he1" },
    {PB_0,  PinCapability::wpwm, "e2heat,he2" },
    {PC_8,  PinCapability::wpwm, "fan0,fan" },
    {PE_5,  PinCapability::wpwm, "fan1" },
    {PE_6,  PinCapability::wpwm, "fan2" },

    //Servos
    {PA_1,  PinCapability::rwpwm, "servo0" },
	
    //EXP1
    {PG_4, PinCapability::rwpwm, "PG4"},
    {PA_8, PinCapability::rwpwm, "PA8"},
    {PD_11, PinCapability::rwpwm, "PD11"},
    {PD_10, PinCapability::rwpwm, "PD10"},
    {PG_2, PinCapability::rwpwm, "PG2"},
    {PG_3, PinCapability::rwpwm, "PG3"},
    {PG_6, PinCapability::rwpwm, "PG6"},
    {PG_7, PinCapability::rwpwm, "PG7"},

    //EXP2
    {PB_14, PinCapability::rwpwm, "PB14"},
    {PB_13, PinCapability::rwpwm, "PB13"},
    {PG_10, PinCapability::rwpwm, "PG10"},
    {PB_12, PinCapability::rwpwm, "PB12"},
    {PG_11, PinCapability::rwpwm, "PG11"},
    {PB_15, PinCapability::rwpwm, "PB15"},
    {PF_12, PinCapability::rwpwm, "PF12"},
    {PF_13, PinCapability::rwpwm, "PF13"},


	//Wifi
	{PG_0, PinCapability::rwpwm, "wifi1,PG0"},
	{PG_1, PinCapability::rwpwm, "wifi2,PG1"},
	{PC_7, PinCapability::rwpwm, "wifi3,PC7"},
	{PC_6, PinCapability::rwpwm, "wifi4,PC6"},
	{PF_14, PinCapability::rwpwm, "wifi5,PF14"},
	{PF_15, PinCapability::rwpwm, "wifi6,PF15"},

};

constexpr BoardDefaults biquskr_pro_1_1_Defaults = {
    {PF_2, PD_7,  PC_0, PC_3,  PA_3, PF_0},    //enablePins
    {PE_9, PE_11, PE_13, PE_14,  PD_15, PD_13},    //stepPins
    {PF_1, PE_8, PC_2, PA_0,  PE_7, PG_9},    //dirPins
#if TMC_SOFT_UART
    {PC_13, PE_3, PE_1, PD_4, PD_1, PD_6},        //uartPins
    6,                                      // Smart drivers
#endif
    0                                       //digiPot Factor
};


constexpr PinEntry PinTable_BIQU_GTR_v1_0[] =
{
    //Thermistors
    {PC_1, PinCapability::ainrw, "e0temp,t0"},
    {PC_2, PinCapability::ainrw, "e1temp,t1"},
    {PC_3, PinCapability::ainrw, "e2temp,t2"},
    {PC_0, PinCapability::ainrw, "bedtemp,t3"},

    //Endstops
    {PF_2, PinCapability::rwpwm, "xstop,x-stop"},
    {PC_13, PinCapability::rwpwm, "ystop,y-stop"},
    {PE_0, PinCapability::rwpwm, "zstop,z-stop"},
    {PG_14, PinCapability::rwpwm, "e0stop,e0det"},
    {PG_9, PinCapability::rwpwm, "e1stop,e1det"},
    {PD_3, PinCapability::rwpwm, "e2stop,e2det"},
	{PH_11, PinCapability::rwpwm, "probe"},
	
    //Heaters and Fans (Big and Small Mosfets}
    {PA_2,  PinCapability::wpwm, "bed,hbed" },
    {PB_1,  PinCapability::wpwm, "e0heat,he0" },
    {PA_1,  PinCapability::wpwm, "e1heat,he1" },
    {PB_0,  PinCapability::wpwm, "e2heat,he2" },
    {PE_5,  PinCapability::wpwm, "fan0,fan" },
    {PE_6,  PinCapability::wpwm, "fan1" },
    {PC_8,  PinCapability::wpwm, "fan2" },

    //Servos
    {PB_11,  PinCapability::rwpwm, "servo0" },
	
    //EXP1
    {PC_11, PinCapability::rwpwm, "PC11"},
    {PA_15, PinCapability::rwpwm, "PA15"},
    {PC_10, PinCapability::rwpwm, "PC10"},
    {PA_8, PinCapability::rwpwm, "PA8"},
    {PG_8, PinCapability::rwpwm, "PG8"},
    {PG_7, PinCapability::rwpwm, "PG7"},
    {PG_6, PinCapability::rwpwm, "PG6"},
    {PG_5, PinCapability::rwpwm, "PG5"},

    //EXP2
    {PB_14, PinCapability::rwpwm, "PB14"},
    {PB_13, PinCapability::rwpwm, "PB13"},
    {PD_10, PinCapability::rwpwm, "PD10"},
    {PB_12, PinCapability::rwpwm, "PB12"},
    {PH_10, PinCapability::rwpwm, "PH10"},
    {PB_15, PinCapability::rwpwm, "PB15"},
    {PB_10, PinCapability::rwpwm, "PB10"},

	//Wifi
	{PF_11, PinCapability::rwpwm, "wifi1,PF11"},
	{PC_7, PinCapability::rwpwm, "wifi3,PC7"},
	{PC_6, PinCapability::rwpwm, "wifi4,PC6"},
	{PC_5, PinCapability::rwpwm, "wifi5,PC5"},
};

constexpr BoardDefaults biqu_gtr_1_0_Defaults = {
    {PF_1, PE_4,  PB_9, PG_13,  PD_7, PD_2},    //enablePins
    {PC_15, PE_3, PB_8, PG_12,  PD_6, PD_1},    //stepPins
    {PF_0, PE_2, PC_7, PG_11,  PD_5, PD_0},    //dirPins
#if TMC_SOFT_UART
    {PC_14, PE_1, PB_5, PG_10, PD_4, PC_12},        //uartPins
    6,                                      // Smart drivers
#endif
    0                                       //digiPot Factor
};

#endif