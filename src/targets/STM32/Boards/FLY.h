#ifndef FLY_H
#define FLY_H

#include "../Pins_STM32.h"



constexpr PinEntry PinTable_FLY_F407ZG[] =
{
    //Thermistors
    {PA_0, PinCapability::ainrw, "e0temp,t0"},
    {PC_1, PinCapability::ainrw, "e1temp,t1"},
    {PC_0, PinCapability::ainrw, "e2temp,t2"},
    {PF_10, PinCapability::ainrw, "e3temp,t3"},
    {PF_5, PinCapability::ainrw, "e4temp,t4"},
    {PF_4, PinCapability::ainrw, "e5temp,t5"},
    {PF_3, PinCapability::ainrw, "bedtemp,tb"},

    //Endstops
    {PC_3, PinCapability::rw, "xmin,xstop"},
    {PC_2, PinCapability::rw, "xmax,xstopmax"},
    {PF_2, PinCapability::rw, "ymin,ystop"},
    {PF_3, PinCapability::rw, "ymax,ystopmax"},
    {PF_0, PinCapability::rw, "zmin,zstop"},
    {PC_15, PinCapability::rw, "zmax,zstopmax"},
    {PC_14, PinCapability::rwpwm, "z3"},
    {PA_3, PinCapability::rwpwm, "dljc"},
	
    //Heaters and Fans (Big and Small Mosfets}
    {PE_2,  PinCapability::wpwm, "bed,hbed" },
    {PF_7,  PinCapability::wpwm, "e0heat,he0" },
    {PF_6,  PinCapability::wpwm, "e1heat,he1" },
    {PE_6,  PinCapability::wpwm, "e2heat,he2" },
    {PE_5,  PinCapability::wpwm, "e3heat,he3" },
    {PE_4,  PinCapability::wpwm, "e4heat,he4" },
    {PE_3,  PinCapability::wpwm, "e5heat,he5" },
    
    {PE_8,  PinCapability::wpwm, "fan0,fan" },
    {PF_9,  PinCapability::wpwm, "fan1" },
    {PA_2,  PinCapability::wpwm, "fan2" },
    {PA_1,  PinCapability::wpwm, "fan3" },
    {PE_13,  PinCapability::wpwm, "fan4" },
    {PB_11,  PinCapability::wpwm, "fan5" },

    // Servo
    {PE_11,  PinCapability::wpwm, "servo0" },

    //EXP1
    {PB_10, PinCapability::rwpwm, "PB10"},
    {PE_14, PinCapability::rwpwm, "PE14"},
    {PE_10, PinCapability::rwpwm, "PE10"},
    {PE_8, PinCapability::rwpwm, "PE8"},
    {PE_15, PinCapability::rwpwm, "PE15"},
    {PE_12, PinCapability::rwpwm, "PE12"},
    {PE_9, PinCapability::rwpwm, "PE9"},
    {PE_7, PinCapability::rwpwm, "PE7"},

    //EXP2
    {PB_14, PinCapability::rwpwm, "PB14"},
    {PB_13, PinCapability::rwpwm, "PB13"},
    {PC_5, PinCapability::rwpwm, "PC5"},
    {PC_4, PinCapability::rwpwm, "PC4"},
    {PF_11, PinCapability::rwpwm, "PF11"},
    {PB_15, PinCapability::rwpwm, "PB15"},
    {PB_2, PinCapability::rwpwm, "PB_2"},

	//SD
	{PC_13, PinCapability::rwpwm, "SDCD"},
	{PC_9, PinCapability::rwpwm, "SD_D1"},
	{PC_8, PinCapability::rwpwm, "SD_D0"},
	{PC_12, PinCapability::rwpwm, "SD_SCK"},
	{PD_2, PinCapability::rwpwm, "SD_CMD"},
	{PC_11, PinCapability::rwpwm, "SD_D3"},
	{PC_10, PinCapability::rwpwm, "SD_D2"},

	// UART
	{PA_9, PinCapability::rwpwm, "TX1"},
	{PA_10, PinCapability::rwpwm, "RX1"},
};

constexpr BoardDefaults fly_f407zg_Defaults = {
	9,											// Number of drivers
    {PE_1, PG_12,  PD_7, PD_4,  PD_0, PG_8, PG_5, PG_2, PD_9},   	//enablePins
    {PB_9, PB_8, PA_8, PC_7,  PC_6, PD_15, PD_14, PD_13, PD_12},	//stepPins
    {PE_0, PG_11, PD_6, PD_3,  PA_15, PG_7, PG_4, PD_11, PD_8},    	//dirPins
#if TMC_SOFT_UART
    {PG_13, PG_10, PD_5, PD_1,
#if STARTUP_DELAY
    // Avoid clash with jtag pins
    NoPin,
#else
    PA_14,
#endif
     PG_6, PG_3, PD_10, PB_12},                 //uartPins
    9,                                      	// Smart drivers
#endif
    0                                       	//digiPot Factor
};

#endif