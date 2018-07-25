#ifndef AZTEEGX5MINI_H__
#define AZTEEGX5MINI_H__


//Config for Azteeg X5 Mini V1.1


//NOTES:
// Filament detector pin and Fan RPM pin must be on a spare pin on Port0 or Port2 only (UNTESTED)
// Azteeg X5 (and maybe others) probe endstop pin is not an ADC pin, so only Digital is supported, or select another spare ADC capable pin
// Note. ADC inputs are NOT 5V tolerant
// Currently No support For Thermocouple etc sensors, only thermistors!



//TODO:: implement firmware update
//#define IAP_UPDATE_FILE "iapazteeg.bin"
//#define IAP_FIRMWARE_FILE "RepRapFirmware-AzteegX5Mini1_1.bin"

// Default board type
#define DEFAULT_BOARD_TYPE BoardType::AzteegX5Mini1_1
#define ELECTRONICS "AzteegX5Mini1_1"

#define AZTEEGX5MINI1_1




// The physical capabilities of the machine

// The number of drives in the machine, including X, Y, and Z plus extruder drives
constexpr size_t DRIVES = 4;

// Initialization macro used in statements needing to initialize values in arrays of size DRIVES.  E.g.,
// max_feed_rates[DRIVES] = {DRIVES_(1, 1, 1, 1, 1, 1, 1, 1, 1, 1)}
#define DRIVES_(a,b,c,d,e,f,g,h,i,j,k,l) { a,b,c,d }

// The number of heaters in the machine
// 0 is the heated bed even if there isn't one.
constexpr size_t Heaters = 2; //Azteeg X5 Mini (Bed + Hotend)

// Initialization macro used in statements needing to initialize values in arrays of size HEATERS.  E.g.,
#define HEATERS_(a,b,c,d,e,f,g,h) { a,b }

constexpr size_t MinAxes = 3;						// The minimum and default number of axes
constexpr size_t MaxAxes = 4;						// The maximum number of movement axes in the machine, usually just X, Y and Z, <= DRIVES
// Initialization macro used in statements needing to initialize values in arrays of size MAX_AXES
#define AXES_(a,b,c,d,e,f,g,h,i) { a,b,c,d }

constexpr size_t MaxExtruders = DRIVES - MinAxes;	// The maximum number of extruders
constexpr size_t MaxDriversPerAxis = 2;				// The maximum number of stepper drivers assigned to one axis


// The numbers of entries in each array must correspond with the values of DRIVES, AXES, or HEATERS. Set values to NoPin to flag unavailability.
// DRIVES
//                                              X      Y      Z      E0
constexpr Pin ENABLE_PINS[DRIVES] =             { P0_10, P0_19, P0_21, P0_4};
constexpr Pin STEP_PINS[DRIVES] =               { P2_1,  P2_2,  P2_3,  P2_0};
constexpr uint8_t STEP_PIN_PORT2_POS[DRIVES] =  { 1,     2,     3,     0}; //SD: Used for calculating bitmap for stepping drivers (this is position of the pin on port2.)
constexpr uint32_t STEP_DRIVER_MASK =           0x0000000F; //SD: mask of the step pins on Port 2 used for writing to step pins in parallel
constexpr Pin DIRECTION_PINS[DRIVES] =          { P0_11, P0_20, P0_22, P0_5};



// Endstops
// Note: RepRapFirmware only as a single endstop per axis
//       gcode defines if it is a max ("high end") or min ("low end")
//       endstop.  gcode also sets if it is active HIGH or LOW
//

//Azteeg X5 Mini has 3 Endstops and one Probe
constexpr Pin END_STOP_PINS[DRIVES] = { P1_24, P1_26, P1_28, NoPin}; // E stop could be mapped to a spare pin if needed...


//Azteeg X5MiniV1.1 uses MCP4451-103

// Indices for motor current digipots (X,Y,Z,E)
constexpr uint8_t POT_WIPES[4] = { 0, 1, 2, 3};
constexpr float digipotFactor = 106.0; //factor for converting current to digipot value

// HEATERS - The bed is assumed to be the at index 0

// Analogue pin numbers
//                                            Bed     Hotend
constexpr Pin TEMP_SENSE_PINS[Heaters] = HEATERS_(P0_23, P0_24, c, d, e, f, g, h);


// Heater outputs

// Note: P2_5 is hardware PWM capable, P2_7 is not

constexpr Pin HEAT_ON_PINS[Heaters] = HEATERS_(P2_7, P2_5, c, d, e, f, g, h); // bed, h0

// Default thermistor betas
constexpr float BED_R25 = 100000.0;
constexpr float BED_BETA = 4066.0;
constexpr float BED_SHC = 0.0;
constexpr float EXT_R25 = 100000.0;
constexpr float EXT_BETA = 4066.0;
constexpr float EXT_SHC = 0.0;

// Thermistor series resistor value in Ohms
constexpr float THERMISTOR_SERIES_RS = 4700.0;


//???
constexpr size_t MaxSpiTempSensors = 1;
// Digital pins the 31855s have their select lines tied to
constexpr Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { NoPin };


// Digital pin number that controls the ATX power on/off
constexpr Pin ATX_POWER_PIN = NoPin;

// Z Probe pin
// Must be an ADC capable pin.  Can be any of the ARM's A/D capable
// pins even a non-Arduino pin.

//Note: Azteeg X5 uses pin P1_29 which is NOT an ADC pin. Use a spare if need Analog in, else use digital options for probe
constexpr Pin Z_PROBE_PIN = P1_29;

// Digital pin number to turn the IR LED on (high) or off (low)
constexpr Pin Z_PROBE_MOD_PIN = NoPin;

constexpr Pin DiagPin = NoPin;


// Use a PWM capable pin
constexpr size_t NUM_FANS = 1;
constexpr Pin COOLING_FAN_PINS[NUM_FANS] = { P2_4 }; // Fan 0 is a Hardware PWM pin

// Firmware will attach a FALLING interrupt to this pin
// see FanInterrupt() in Platform.cpp
// SD:: Note: Only GPIO pins on Port0 and Port2 support this. If needed choose from spare pins
constexpr Pin COOLING_FAN_RPM_PIN = NoPin;
//const Pin COOLING_FAN_RPM_PIN = P0_17;

//SD: Internal SDCard is on SSP1
//    MOSI, MISO, SCLK, CS
//    P0_9, P0_8, P0_7, P0_6

//SD:: 2nd SDCard can be connected to SSP0
//    MOSI, MISO, SCLK
//    P0_18 P0_17 P0_15

// SD cards
//sd:: Internal SD card is on SSP1
//NOTE::: Although this is 2nd in the List, SSP1 is Configured to be Slot0 in coreNG to be compatible with RRF
//default to supporting 2 card..... if need 1_23 then change CS no No pin

constexpr size_t NumSdCards = 2; //
constexpr Pin SdCardDetectPins[NumSdCards] = { NoPin, P1_31 }; //2nd card  CS pin P1_31
constexpr Pin SdWriteProtectPins[NumSdCards] = { NoPin, NoPin };
constexpr Pin SdSpiCSPins[NumSdCards] = { P0_6, P1_23 };// Internal, external. Note:: ("slot" 0 in CORE is configured to be LCP SSP1 to match default RRF behaviour)

// Definition of which pins we allow to be controlled using M42

// M42 and M208 commands now use logical pin numbers, not firmware pin numbers.
// This is the mapping from logical pins 60+ to firmware pin numbers


constexpr Pin SpecialPinMap[] =
{

    //Exp1
    P1_30,
    //P1_22,   // Viki_Btn (red button)
    P0_26,
    //P0_25,   // Viki_Buzzer
    P0_27,   //(SDA)
    //P4_29,   // Viki_BlueLed
    P0_28    //(SCL)
    //P2_8,    // Viki_RedLed
    //(3.3v)
    //(GND)
    
    //Exp2 (used for Viki2.0 LCD default)
    //P1_31,    //Viki_SD Card Detect
    //P3_26,    //Viki_EncB
    //P2_11,    //Viki_Btn (wheel click button)
    //P3_25,    //Viki_EncA
    //P1_23     //Viki_SD Chip Select (if using also change SdSpiCSPins above to NoPin)
    //P0_17,    //SSP0(MISO)
    //P0_16,    //Viki_Lcd ChipSelect
    //P2_6,     //Viki_A0
    //P0_15,    //SSP0(SCLK)
    //P0_18,    //SSP0(MOSI)
    
    
};

//Azteeg X5 Mini 1.1 EXP1 Header (not compat with V2.0 etc etc)

// P1_30        P1_22
// P0_26        P0_25
// P0_27(SDA)   P4_29
// P0_28(SCL)   P2_8
// 3.3v         GND

//Azteeg X5 Mini 1.1 Exp2 Header

// P1_31    P3_26
// P2_11    P3_25
// P1_23    P0_17
// P0_16    P2_6
// P0_15    P0_18
// GND      +5V



// Flash locations (may be expanded in the future)
//const uint32_t IAP_FLASH_START = 0x000F0000;
//const uint32_t IAP_FLASH_END = 0x000FFBFF;		// don't touch the last 1KB, it's used for NvData


//Viki2.0 Controller
// SDCD     ENCB
// BTN      ENCA
// SDCS     MISO
// LCS      A0
// SCK      MOSI
// GND      +Vin

// BTN
// BUZZER
// BLUE-LED
// RED-LED


//SPI LCD Common Settings (Viki2.0 and RRD Full Graphic Smart Display
constexpr LPC_SSP_TypeDef* LcdSpiDevice = LPC_SSP0;     //SSP0 (MISO0, MOSI0, SCK0)
constexpr Pin LcdCSPin =       P0_16; //LCD Chip Select
constexpr Pin LcdDCPin =       P2_6;  //DataControl Pin (A0) if none used set to NoPin
constexpr Pin LcdBeepPin =     P0_25;
constexpr Pin EncoderPinA =    P3_25;
constexpr Pin EncoderPinB =    P3_26;
constexpr Pin EncoderPinSw =   P2_11; //click
constexpr Pin PanelButtonPin = P1_22; //Extra button on Viki and RRD Panels (reset/back etc configurable)


//VIKI2.0 Specific options
constexpr Pin VikiRedLedPin = P2_8;
constexpr Pin VikiBlueLedPin = P4_29;


#endif
