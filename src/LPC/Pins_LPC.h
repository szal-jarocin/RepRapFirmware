#ifndef PINS_DUET_H__
#define PINS_DUET_H__


#include "Microstepping.h"

//NOTES:
// Filament detector pin and Fan RPM pin must be on a spare pin on Port0 or Port2 only (Untested)
// Azteeg X5 (and maybe others) probe endstop pin is not an ADC pin, so only Digital will be used, or select another spare ADC capable pin if need analog in


//TEMP TESTING
//#define NO_SOFTWARE_RESET_DATA
//#define NO_THERMOCOUPLE_SUPPORT
#define NO_FIRMWARE_UPDATE

//TODO:: implement firmware update
const size_t NumFirmwareUpdateModules = 0;

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif





#define FIRMWARE_NAME "RepRapFirmware for LPC17xx based Boards"

// Features definition
#define HAS_CPU_TEMP_SENSOR		     0				// enabling the CPU temperature sensor disables Due pin 13 due to bug in SAM3X
#define HAS_HIGH_SPEED_SD		     0

#define HAS_VOLTAGE_MONITOR		     0
#define ACTIVE_LOW_HEAT_ON		     0

#define HAS_RTOSPLUSTCP_NETWORKING   1
#define HAS_LWIP_NETWORKING          0
#define HAS_WIFI_NETWORKING          0

#define HAS_VREF_MONITOR             0
#define SUPPORT_NONLINEAR_EXTRUSION  0


#define SUPPORT_INKJET		0					// set nonzero to support inkjet control
#define SUPPORT_ROLAND		0					// set nonzero to support Roland mill
#define SUPPORT_SCANNER		0					// set nonzero to support FreeLSS scanners
#define SUPPORT_IOBITS		0					// set to support P parameter in G0/G1 commands
#define SUPPORT_DHT_SENSOR	0					// set nonzero to support DHT temperature/humidity sensors


//LCD Support with No Networking 
#ifdef LPC_NETWORKING
    #define SUPPORT_12864_LCD       0
#else
    #define SUPPORT_12864_LCD       1
#endif

constexpr size_t NumExtraHeaterProtections = 4;
constexpr size_t NumThermistorInputs = 2;
constexpr size_t NumTachos = 0;
constexpr Pin TachoPins[NumTachos] = { };


// The physical capabilities of the machine


#if defined(__AZTEEGX5MINI__)
# include "variants/AzteegX5Mini1_1.h"
#elif defined(__SMOOTHIEBOARD__)
# include "variants/Smoothieboard1.h"
#elif defined(__REARM__)
# include "variants/ReArm1_0.h"
#elif defined(__MBED__)
//Only used for debugging just use smoothie for now
# include "variants/Smoothieboard1.h"
#else

# error "Unknown LPC Variant"
#endif

constexpr size_t NUM_SERIAL_CHANNELS = 2;
// Use TX0/RX0 for the auxiliary serial line

#if defined(__MBED__)
#define SERIAL_MAIN_DEVICE Serial0 //TX0/RX0 connected to via seperate USB
#define SERIAL_AUX_DEVICE Serial //USB pins unconnected.
#else
#define SERIAL_MAIN_DEVICE Serial //USB
#define SERIAL_AUX_DEVICE Serial0 //TX0/RX0
#endif



// This next definition defines the highest one.
const int HighestLogicalPin = 60 + ARRAY_SIZE(SpecialPinMap) - 1;		// highest logical pin number on this electronics


// Timer allocation
//SD: LPC1768 has 4 timers + 1 RIT (Repetitive Interrupt Timer)
//PCLK_timer1-4 is set to 1/4 CCLK by CORE...


#define STEP_TC				(LPC_TIM0)
#define STEP_TC_IRQN		TIMER0_IRQn
#define STEP_TC_HANDLER		TIMER0_IRQHandler
#define STEP_TC_PCONPBIT    SBIT_PCTIM0
#define STEP_TC_PCLKBIT     PCLK_TIMER0
#define STEP_TC_TIMER       TIMER0


#define NETWORK_TC            (LPC_TIM1)
#define NETWORK_TC_IRQN       TIMER1_IRQn
#define NETWORK_TC_HANDLER    TIMER1_IRQHandler
#define NETWORK_TC_PCONPBIT   SBIT_PCTIM1
#define NETWORK_TC_PCLKBIT    PCLK_TIMER1
#define NETWORK_TC_TIMER      TIMER1

#endif
