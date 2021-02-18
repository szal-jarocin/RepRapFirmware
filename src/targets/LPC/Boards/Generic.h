#ifndef GENERIC_H
#define GENERIC_H

#include "../Pins_LPC.h"


// List of assignable pins and their mapping from names to MPU ports. This is indexed by logical pin number.
// The names must match user input that has been concerted to lowercase and had _ and - characters stripped out.
// Aliases are separate by the , character.
// If a pin name is prefixed by ! then this means the pin is hardware inverted. The same pin may have names for both the inverted and non-inverted cases,
// for example the inverted heater pins on the expansion connector are available as non-inverted servo pins on a DFueX.

//assumes user are selecting correct pins, and correctly setup etc so we will set most pins as rwpwm (since any pin can do PWM on a timer as long as there are free slots available)
constexpr PinEntry PinTable_Generic[] =
{
    {P0_0, "P0.0"},
    {P0_1, "P0.1"},
    {P0_2, "P0.2"},
    {P0_3, "P0.3"},
    {P0_4, "P0.4"},
    {P0_5, "P0.5"},
    //{P0_6, "0.6"}, //Internal SD_CS
    //{P0_7, "0.7"}, //Internal SD_SCK
    //{P0_8, "0.8"}, //Internal SD_MISO
    //{P0_9, "0.9"}, //Internal SD_MOSI
    {P0_10, "P0.10"},
    {P0_11, "P0.11"},
    //{P0_12, "0.12"},  //N/A
    //{P0_13, "0.13"},  //N/A
    //{P0_14, "0.14"},  //N/A
    //{P0_15, "0.15"}, //SSP0_SCK
    {P0_16, "P0.16"},
    //{P0_17, "0.17"}, //SSP0_MISO
    //{P0_18, "0.18"}, //SSP0_MOSI
    {P0_19, "P0.19"},
    {P0_20, "P0.20"},
    {P0_21, "P0.21"},
    {P0_22, "P0.22"},
    {P0_23, "P0.23"},
    {P0_24, "P0.24"},
    {P0_25, "P0.25"},
    {P0_26, "P0.26"},
    {P0_27, "P0.27"},
    {P0_28, "P0.28"},
    //{P0_29, "0.29"},    //USB D+
    //{P0_30, "0.30"}, //USB D-
    //{P0_31, "0.31"}, //N/A
    
    //Pins P1.[0,1,4,8,9,10,14,15,16,17] used by Ethernet
    {P1_0, "P1.0"},  //ENET_TXD0
    {P1_1, "P1.1"},  //ENET_TXD1
    {P1_2, "P1.2"},  //N/A
    {P1_3, "P1.3"},  //N/A
    {P1_4, "P1.4"},  //ENET_TX_EN
    {P1_5, "P1.5"},  //N/A
    {P1_6, "P1.6"},  //N/A
    {P1_7, "P1.7"},  //N/A
    {P1_8, "P1.8"},  //ENET_CRS
    {P1_9, "P1.9"},  //ENET_RXD0
    {P1_10, "P1.10"},  //ENET_RXD1
    {P1_11, "P1.11"},  //N/A
    {P1_12, "P1.12"},  //N/A
    {P1_13, "P1.13"},  //N/A
    {P1_14, "P1.14"},  //ENET_RX_ER
    {P1_15, "P1.15"},  //ENET_REF_CLK
    {P1_16, "P1.16"},  //ENET_MDC
    {P1_17, "P1.17"},  //ENET_MDIO
    {P1_18, "P1.18"},
    {P1_19, "P1.19"},
    {P1_20, "P1.20"},
    {P1_21, "P1.21"},
    {P1_22, "P1.22"},
    {P1_23, "P1.23"},
    {P1_24, "P1.24"},
    {P1_25, "P1.25"},
    {P1_26, "P1.26"},
    {P1_27, "P1.27"},
    {P1_28, "P1.28"},
    {P1_29, "P1.29"},
    {P1_30, "P1.30"},
    {P1_31, "P1.31"},
    {P2_0, "P2.0"},
    {P2_1, "P2.1"},
    {P2_2, "P2.2"},
    {P2_3, "P2.3"},
    {P2_4, "P2.4"},
    {P2_5, "P2.5"},
    {P2_6, "P2.6"},
    {P2_7, "P2.7"},
    {P2_8, "P2.8"},
    //{P2_9, "2.9"}, //USB Connect
    {P2_10, "P2.10"},
    {P2_11, "P2.11"},
    {P2_12, "P2.12"},
    {P2_13, "P2.13"},
    //Pins 2.14-2.31 are N/A
    //{P2_14, "2.14"},
    //{P2_15, "2.15"},
    //{P2_16, "2.16"},
    //{P2_17, "2.17"},
    //{P2_18, "2.18"},
    //{P2_19, "2.19"},
    //{P2_20, "2.20"},
    //{P2_21, "2.21"},
    //{P2_22, "2.22"},
    //{P2_23, "2.23"},
    //{P2_24, "2.24"},
    //{P2_25, "2.25"},
    //{P2_26, "2.26"},
    //{P2_27, "2.27"},
    //{P2_28, "2.28"},
    //{P2_29, "2.29"},
    //{P2_30, "2.30"},
    //{P2_31, "2.31"},
    //Pins 3.0-3.24 are N/A
    //{P3_0, "3.0"},
    //{P3_1, "3.1"},
    //{P3_2, "3.2"},
    //{P3_3, "3.3"},
    //{P3_4, "3.4"},
    //{P3_5, "3.5"},
    //{P3_6, "3.6"},
    //{P3_7, "3.7"},
    //{P3_8, "3.8"},
    //{P3_9, "3.9"},
    //{P3_10, "3.10"},
    //{P3_11, "3.11"},
    //{P3_12, "3.12"},
    //{P3_13, "3.13"},
    //{P3_14, "3.14"},
    //{P3_15, "3.15"},
    //{P3_16, "3.16"},
    //{P3_17, "3.17"},
    //{P3_18, "3.18"},
    //{P3_19, "3.19"},
    //{P3_20, "3.20"},
    //{P3_21, "3.21"},
    //{P3_22, "3.22"},
    //{P3_23, "3.23"},
    //{P3_24, "3.24"},
    {P3_25, "P3.25"},
    {P3_26, "P3.26"},
    //pins 3.27-3.31 are N/A
    //{P3_27, "3.27"},
    //{P3_28, "3.28"},
    //{P3_29, "3.29"},
    //{P3_30, "3.30"},
    //{P3_31, "3.31"},
    //Pins 4.0-4.27 are N/A
    //{P4_0, "4.0"},
    //{P4_1, "4.1"},
    //{P4_2, "4.2"},
    //{P4_3, "4.3"},
    //{P4_4, "4.4"},
    //{P4_5, "4.5"},
    //{P4_6, "4.6"},
    //{P4_7, "4.7"},
    //{P4_8, "4.8"},
    //{P4_9, "4.9"},
    //{P4_10, "4.10"},
    //{P4_11, "4.11"},
    //{P4_12, "4.12"},
    //{P4_13, "4.13"},
    //{P4_14, "4.14"},
    //{P4_15, "4.15"},
    //{P4_16, "4.16"},
    //{P4_17, "4.17"},
    //{P4_18, "4.18"},
    //{P4_19, "4.19"},
    //{P4_20, "4.20"},
    //{P4_21, "4.21"},
    //{P4_22, "4.22"},
    //{P4_23, "4.23"},
    //{P4_24, "4.24"},
    //{P4_25, "4.25"},
    //{P4_26, "4.26"},
    //{P4_27, "4.27"},
    {P4_28, "P4.28"},
    {P4_29, "P4.29"},
    //Pins 4.30-4.31 are N/A
    //{P4_30, "4.30"},
    //{P4_31, "4.31"},
        
};

//NoPin for Generic, Actual pins must be specified in /sys/Board.txt 
constexpr BoardDefaults genericDefaults = {
    5,
    {NoPin, NoPin, NoPin, NoPin, NoPin},    //enablePins
    {NoPin, NoPin, NoPin, NoPin, NoPin},    //stepPins
    {NoPin, NoPin, NoPin, NoPin, NoPin},    //dirPins
#if HAS_SMART_DRIVERS
    {NoPin, NoPin, NoPin, NoPin, NoPin},    //uartPins
    0,                                      // Smart drivers
#endif
    0,                                      //digiPot Factor    
};

#endif