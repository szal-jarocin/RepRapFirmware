#ifndef PINS_DUET_H__
#define PINS_DUET_H__

#include "Microstepping.h"
#include "sd_mmc.h"
#include "RepRapFirmware.h"

const size_t NumFirmwareUpdateModules = 1;

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif





#define FIRMWARE_NAME "RepRapFirmware for LPC176x based Boards"

// Default board type
#define DEFAULT_BOARD_TYPE BoardType::Lpc
#define ELECTRONICS "LPC176x"
#define LPC_ELECTRONICS_STRING "LPC176x"
#define LPC_BOARD_STRING "LPC176x"

#define FIRMWARE_FILE "firmware.bin"

// Features definition
#define SUPPORT_OBJECT_MODEL             1
#define HAS_CPU_TEMP_SENSOR		         0	// enabling the CPU temperature sensor disables Due pin 13 due to bug in SAM3X
#define HAS_HIGH_SPEED_SD		         0
#define HAS_VOLTAGE_MONITOR		         0
#define ACTIVE_LOW_HEAT_ON		         0
#define HAS_LWIP_NETWORKING              0
#define HAS_WIFI_NETWORKING              0
#define HAS_VREF_MONITOR                 0
#define SUPPORT_NONLINEAR_EXTRUSION      0
#define SUPPORT_INKJET		             0	// set nonzero to support inkjet control
#define SUPPORT_ROLAND		             0	// set nonzero to support Roland mill
#define SUPPORT_SCANNER		             0	// set nonzero to support FreeLSS scanners
#define SUPPORT_IOBITS		             0	// set to support P parameter in G0/G1 commands
#define SUPPORT_DHT_SENSOR	             0	// set nonzero to support DHT temperature/humidity sensors
#define SUPPORT_WORKPLACE_COORDINATES    1

#define SUPPORT_TELNET                   0
#define SUPPORT_FTP                      0
#define NO_PANELDUE                      1


#if defined(LPC_NETWORKING)
    #define HAS_RTOSPLUSTCP_NETWORKING   1
    #define SUPPORT_12864_LCD            1
#else
    #define HAS_RTOSPLUSTCP_NETWORKING   0
    #define SUPPORT_12864_LCD            1
#endif



#warning ***Temporary***
#define NO_TRIGGERS                      1    // Temporary!!!
#define NO_EXTRUDER_ENDSTOPS             1    // Temporary!!!



// Tacho Pins - Only GPIO pins on Port0 and Port2 are supported.
//constexpr size_t NumTachos = 1;
//extern Pin TachoPins[NumTachos];


// The physical capabilities of the machine
constexpr size_t NumDirectDrivers = 5;                // The maximum number of drives supported by the electronics
constexpr size_t MaxTotalDrivers = NumDirectDrivers;
constexpr size_t MaxSmartDrivers = 0;                // The maximum number of smart drivers

//constexpr size_t NumEndstops = 6;                    // The number of inputs we have for endstops, filament sensors etc.

//constexpr size_t NumHeaters = 3;                    // The number of heaters in the machine; 0 is the heated bed even if there isn't one
constexpr size_t NumTotalHeaters = 3;                // The maximum number of heaters in the machine
constexpr size_t NumExtraHeaterProtections = 3;     // The number of extra heater protection instances
constexpr size_t NumThermistorInputs = 3;

constexpr size_t MaxGpioPorts = 10;

constexpr size_t MinAxes = 3;                        // The minimum and default number of axes
constexpr size_t MaxAxes = 5;                        // The maximum number of movement axes in the machine, usually just X, Y and Z, <= DRIVES

constexpr size_t MaxExtruders = NumDirectDrivers - MinAxes;    // The maximum number of extruders
constexpr size_t NumDefaultExtruders = 1;            // The number of drivers that we configure as extruders by default
constexpr size_t MaxDriversPerAxis = 2;                // The maximum number of stepper drivers assigned to one axis

constexpr size_t MaxHeatersPerTool = 2;
constexpr size_t MaxExtrudersPerTool = 2;


//Steppers
extern Pin ENABLE_PINS[NumDirectDrivers];
extern Pin STEP_PINS[NumDirectDrivers];
extern Pin DIRECTION_PINS[NumDirectDrivers];
extern uint32_t STEP_DRIVER_MASK; // Mask for parallel write to all steppers on port 2 (calculated in firmware)
extern bool hasStepPinsOnDifferentPorts;
extern bool hasDriverCurrentControl;
extern float digipotFactor;

//Smoothie uses MCP4451 for current control
// Indices for motor current digipots (X,Y,Z,E) - E is on 2nd digipot chip
constexpr uint8_t POT_WIPES[5] = { 0, 1, 2, 3, 0};


//endstops
//TODO:: this needs to be updated in BoardCOnfig LPC
//extern Pin END_STOP_PINS[NumEndstops];

extern Pin Z_PROBE_PIN; // Z Probe pin
extern Pin Z_PROBE_MOD_PIN; // Digital pin number to turn the IR LED on (high) or off (low)

// HEATERS - The bed is assumed to be the at index 0
extern Pin TEMP_SENSE_PINS[NumThermistorInputs];
//extern Pin HEAT_ON_PINS[NumHeaters];


//Timer 0 is used for the Step Generation
//Timer 2 is fixed at 50Hz for servos
//Timer Freqs for 1 and 3 set in config.
extern uint16_t Timer1Frequency;
extern uint16_t Timer3Frequency;


// Default thermistor betas
constexpr float BED_R25 = 100000.0;
constexpr float BED_BETA = 3988.0;
constexpr float BED_SHC = 0.0;
constexpr float EXT_R25 = 100000.0;
constexpr float EXT_BETA = 4388.0;
constexpr float EXT_SHC = 0.0;
constexpr float THERMISTOR_SERIES_RS = 4700.0; // Thermistor series resistor value in Ohms

constexpr size_t MaxSpiTempSensors = 2;
extern Pin SpiTempSensorCsPins[MaxSpiTempSensors]; // Digital pins the 31855s have their select lines tied to
constexpr SSPChannel TempSensorSSPChannel = SSP0; //Conect SPI Temp sensor to SSP0

extern Pin ATX_POWER_PIN;// Digital pin number that controls the ATX power on/off


constexpr size_t MaxZProbes = 1;

constexpr size_t NumTotalFans = 3;

//SD: Internal SDCard is on SSP1
//    MOSI, MISO, SCLK, CS
//    P0_9, P0_8, P0_7, P0_6

//SD:: 2nd SDCard can be connected to SSP0
//    MOSI, MISO, SCLK
//    P0_18 P0_17 P0_15

// SD cards
constexpr size_t NumSdCards = _DRIVES; //_DRIVES is defined in CoreLPC (and used by FatFS) allow one internal and one external
extern Pin SdCardDetectPins[NumSdCards];
constexpr Pin SdWriteProtectPins[NumSdCards] = { NoPin, NoPin }; //unused on LPC boards
extern Pin SdSpiCSPins[NumSdCards];
extern uint32_t ExternalSDCardFrequency;
extern uint32_t InternalSDCardFrequency;


// Definition of which pins we allow to be controlled using M42 etc
//constexpr size_t MaxNumberSpecialPins = 10;
//extern Pin SpecialPinMap[MaxNumberSpecialPins];

constexpr uint32_t LcdSpiClockFrequency = 2000000;    // 2.0MHz
constexpr SSPChannel LcdSpiChannel = SSP0;
extern Pin LcdCSPin;
extern Pin LcdDCPin;
extern Pin LcdBeepPin;
extern Pin EncoderPinA;
extern Pin EncoderPinB;
extern Pin EncoderPinSw;
extern Pin PanelButtonPin;


//LEDs
extern Pin DiagPin;


constexpr size_t NUM_SERIAL_CHANNELS = 2;



// Default pin allocations




// Enum to represent allowed types of pin access
// We don't have a separate bit for servo, because Duet PWM-capable ports can be used for servos if they are on the Duet main board
enum class PinCapability: uint8_t
{
    // Individual capabilities
    read = 1,
    ain = 2,
    write = 4,
    pwm = 8,
    
    // Combinations
    ainr = 1|2,
    rw = 1|4,
    wpwm = 4|8,
    rwpwm = 1|4|8,
    ainrw = 1|2|4,
    ainrwpwm = 1|2|4|8
};

constexpr inline PinCapability operator|(PinCapability a, PinCapability b)
{
    return (PinCapability)((uint8_t)a | (uint8_t)b);
}

// Struct to represent a pin that can be assigned to various functions
//// This can be varied to suit the hardware. It is a struct not a class so that it can be direct initialised in read-only memory.
//struct PinEntry
//{
//    bool CanDo(PinAccess access) const;
//    Pin GetPin() const { return pin; }
//    PinCapability GetCapability() const { return cap; }
//    const char* GetNames() const { return names; }
//
//    Pin pin;
//    PinCapability cap;
//    const char *names;
//};

struct PinEntry : PinDescription
{
    bool CanDo(PinAccess access) const;
    Pin GetPin() const { return pin; }
    //PinCapability GetCapability() const { return cap; } //todo check for PWM on Timer etc
    const char* GetNames() const { return "TODO_PINNAME"; } //generate name i.e. "P1_20"

    //Pin pin;
    //PinCapability cap;
    //const char *names;
};


// List of assignable pins and their mapping from names to MPU ports. This is indexed by logical pin number.
// The names must match user input that has been concerted to lowercase and had _ and - characters stripped out.
// Aliases are separate by the , character.
// If a pin name is prefixed by ! then this means the pin is hardware inverted. The same pin may have names for both the inverted and non-inverted cases,
// for example the inverted heater pins on the expansion connector are available as non-inverted servo pins on a DFueX.


//constexpr PinEntry PinTable_Smoothieboard[] =
//{
//      {pin, "XMin"}
//};

//constexpr PinEntry PinTable[] =
//{
    // Heater outputs
//    { PortCPin(0),    PinCapability::wpwm,    "!bedheat" },
//    { PortCPin(1),    PinCapability::wpwm,    "!e0heat" },
//    { PortAPin(16), PinCapability::wpwm,    "!e1heat" },
//
//    // Fan outputs
//    { PortCPin(23),    PinCapability::wpwm,    "fan0" },
//    { PortCPin(22),    PinCapability::wpwm,    "fan1" },
//    { PortCPin(29),    PinCapability::wpwm,    "fan2" },
//
//    // Endstop inputs
//    { PortAPin(24),    PinCapability::read,    "xstop" },
//    { PortBPin(6),    PinCapability::read,    "ystop" },
//    { PortCPin(10),    PinCapability::read,    "zstop" },
//    { PortAPin(25),    PinCapability::read,    "e0stop" },
//    { PortCPin(7),    PinCapability::read,    "e1stop" },
//
//    // Misc
//    { Z_PROBE_PIN,    PinCapability::ainr,    "zprobe.in" },
//    { Z_PROBE_MOD_PIN, PinCapability::write, "zprobe.mod,servo" },
//    { ATX_POWER_PIN, PinCapability::write,    "pson" },
//    { PortAPin(21), PinCapability::ainrw,    "exp.pa21" },
//    { PortAPin(22), PinCapability::ainrw,    "exp.pa22" },
//    { PortAPin(3),    PinCapability::rw,        "exp.pa3" },
//    { PortAPin(4),    PinCapability::rw,        "exp.pa4" },
//};

//constexpr unsigned int NumNamedPins = ARRAY_SIZE(PinTable);


extern PinEntry *PinTable;
constexpr unsigned int NumNamedPins = MaxPinNumber;


 bool LookupPinName(const char *pn, LogicalPin& lpin, bool& hardwareInverted);

//constexpr const char *DefaultEndstopPinNames[] = { "xstop", "ystop", "zstop" };
//constexpr const char *DefaultZProbePinNames = "^zprobe";
//constexpr const char *DefaultHeaterPinNames[] = { "bedheat", "e0heat", "e1heat" };
//constexpr const char *DefaultFanPinNames[] = { "fan0", "fan1", "fan2" };
constexpr const char *DefaultEndstopPinNames[] = { "nil", "nil", "nil" };
constexpr const char *DefaultZProbePinNames = "nil";
constexpr const char *DefaultHeaterPinNames[] = {  };
constexpr const char *DefaultFanPinNames[] = { "nil", "nil" };
constexpr PwmFrequency DefaultFanPwmFrequencies[] = { DefaultFanPwmFreq };




// Use TX0/RX0 for the auxiliary serial line
#if defined(__MBED__)
    #define SERIAL_MAIN_DEVICE Serial0 //TX0/RX0 connected to via seperate USB
    #define SERIAL_AUX_DEVICE Serial //USB pins unconnected.
#else
    #define SERIAL_MAIN_DEVICE Serial //USB
    #define SERIAL_AUX_DEVICE Serial0 //TX0/RX0
#endif


//Timer 0 is used for Step Generation
#define STEP_TC				(LPC_TIM0)
#define STEP_TC_IRQN		TIMER0_IRQn
#define STEP_TC_HANDLER		TIMER0_IRQHandler
#define STEP_TC_PCONPBIT    SBIT_PCTIM0
#define STEP_TC_PCLKBIT     PCLK_TIMER0
#define STEP_TC_TIMER       TIMER0

#endif
