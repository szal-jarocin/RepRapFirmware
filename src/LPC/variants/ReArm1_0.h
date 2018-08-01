#ifndef REARM_H__
#define REARM_H__


//Config for ReArm v1.0


//NOTES:
// Filament detector pin and Fan RPM pin must be on a spare pin on Port0 or Port2 only (UNTESTED)
// Azteeg X5 (and maybe others) probe endstop pin is not an ADC pin, so only Digital is supported, or select another spare ADC capable pin
// Note. ADC inputs are NOT 5V tolerant
// Currently No support For Thermocouple etc sensors, only thermistors!


//TODO:: implement firmware update
//#define IAP_FIRMWARE_FILE "RepRapFirmware-AzteegX5Mini1_1.bin"

// Default board type
#define DEFAULT_BOARD_TYPE BoardType::ReArm1_0
#define ELECTRONICS "ReArm v1.0"

#define REARM1_0



// The physical capabilities of the machine

// The number of drives in the machine, including X, Y, and Z plus extruder drives
constexpr size_t DRIVES = 5;

// Initialization macro used in statements needing to initialize values in arrays of size DRIVES.  E.g.,
// max_feed_rates[DRIVES] = {DRIVES_(1, 1, 1, 1, 1, 1, 1, 1, 1, 1)}
#define DRIVES_(a,b,c,d,e,f,g,h,i,j,k,l) { a,b,c,d,e }

// The number of heaters in the machine
// 0 is the heated bed even if there isn't one.
constexpr size_t Heaters = 2; //ReArm (Bed + Hotend1) // set to 3 if using 2nd heater and update macro below, and  Tempsense and heat pins below and remove(chance to another) cooling fan pin too

// Initialization macro used in statements needing to initialize values in arrays of size HEATERS.  E.g.,
#define HEATERS_(a,b,c,d,e,f,g,h) { a,b }

constexpr size_t MinAxes = 3;						// The minimum and default number of axes
constexpr size_t MaxAxes = 5;						// The maximum number of movement axes in the machine, usually just X, Y and Z, <= DRIVES
// Initialization macro used in statements needing to initialize values in arrays of size MAX_AXES
#define AXES_(a,b,c,d,e,f,g,h,i) { a,b,c,d,e }

constexpr size_t MaxExtruders = DRIVES - MinAxes;	// The maximum number of extruders
constexpr size_t MaxDriversPerAxis = 2;				// The maximum number of stepper drivers assigned to one axis


// The numbers of entries in each array must correspond with the values of DRIVES, AXES, or HEATERS. Set values to NoPin to flag unavailability.
// DRIVES
//                                              X      Y      Z     E1     E2
constexpr Pin ENABLE_PINS[DRIVES] =             { P0_10, P0_19, P0_21, P0_4,  P4_29};
constexpr Pin STEP_PINS[DRIVES] =               { P2_1,  P2_2,  P2_3,  P2_0,  P2_8};
constexpr uint8_t STEP_PIN_PORT2_POS[DRIVES] =  { 1,     2,     3,     0,     8}; //SD: Used for calculating bitmap for stepping drivers (this is position of the pins on the port)
constexpr uint32_t STEP_DRIVER_MASK =           0x0000010F; //SD: mask of the step pins on Port 2 used for writing to step pins in parallel
constexpr Pin DIRECTION_PINS[DRIVES] =          { P0_11, P0_20, P0_22, P0_5,  P2_13};



// Endstops
// Note: RepRapFirmware only as a single endstop per axis
//       gcode defines if it is a max ("high end") or min ("low end")
//       endstop.  gcode also sets if it is active HIGH or LOW
//

//RE-Arm has 6 endstops. We will assume MAX endstops headers are used, leaving P1_24, 1_26 free for other purposes. 1_29 (Z-min used for probe)

constexpr Pin END_STOP_PINS[DRIVES] = { P1_25, P1_27, P1_28, NoPin, NoPin}; // E stop could be mapped to a spare pin if needed...


//RaArm has no current control for drivers.

// Indices for motor current digipots (X,Y,Z,E1, E2)
//const uint8_t POT_WIPES[5] = { 0, 1, 2, 3, 0};
//const float digipotFactor = 106.0; //factor for converting current to digipot value
#define HAS_DRIVER_CURRENT_CONTROL 0


// HEATERS - The bed is assumed to be the at index 0

// Analogue pin numbers
//                                            Bed    H1
constexpr Pin TEMP_SENSE_PINS[Heaters] = HEATERS_(P0_24, P0_23, /*P0_25*/c, d, e, f, g, h);


// Heater outputs

// Note: P2_0 to P2_5 is hardware PWM capable, P2_7 is not

constexpr Pin HEAT_ON_PINS[Heaters] = HEATERS_(P2_7, P2_5, /*P2.4*/c, d, e, f, g, h); // bed, h0

// Default thermistor betas
constexpr float BED_R25 = 100000.0;
constexpr float BED_BETA = 4066.0;
constexpr float BED_SHC = 0.0;
constexpr float EXT_R25 = 100000.0;
constexpr float EXT_BETA = 4066.0;
constexpr float EXT_SHC = 0.0;

// Thermistor series resistor value in Ohms
constexpr float THERMISTOR_SERIES_RS = 4700.0;


constexpr size_t MaxSpiTempSensors = 1;
// Digital pins the 31855s have their select lines tied to
constexpr Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { P0_27 };
constexpr SSPChannel TempSensorSSPChannel = SSP0;


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
// SD:: Note: Only GPIO pins on Port0 and Port2 support this. If needed choose from spare pins (UNTESTED)
constexpr Pin COOLING_FAN_RPM_PIN = NoPin;


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

constexpr size_t NumSdCards = 2;//Note: use 2 even if only using 1 (internal) card
constexpr Pin SdCardDetectPins[NumSdCards] = { NoPin, P1_31 };
constexpr Pin SdWriteProtectPins[NumSdCards] = { NoPin, NoPin };
constexpr Pin SdSpiCSPins[NumSdCards] = { P0_6, P1_23};// Internal, external. If need 1_23 pin, and no ext sd card set to NoPin Note:: ("slot" 0 in CORE is configured to be LCP SSP1 to match default RRF behaviour)


// Definition of which pins we allow to be controlled using M42

constexpr Pin SpecialPinMap[] =
{
    //Note: PWM Channels 5 & 6 are used by the Hotend Heaters and cant be used.
    //servos
    //  LPC PIN         RAMPS
    //Servos
    P1_20, //60    Servo1   11      PWM1[2]
    P1_21, //61    Servo2    6      PWM1[3]
    P1_19, //62    Servo3    5
    P1_18, //63    Servo4    4      PWM1[1]    
    //Spare Endstops
    P1_26, //      Y-Min    14      PWM1[6] (do not use pwm - chan6 in use by heaters)
    P1_24, //      X-Min     3      PWM1[5] (do not use pwm - chan5 in use by heaters)

    
    
    //P0_27, //    Aux1  A3/57    // Conifigured as SPI Thermocouple CS
    P0_28, //    Aux1  A4/58
    

    
    
//J3
    //P0_15, // J3                  SCLK
    //P0_16, // J3
    //P1_23, // J3          53      PWM1[4]
    //P2_11, // J3          35
    //P1_31, // J3          49
    //P0_18, // J3                  MOSI
    //P2_6,  // J3/Aux2  A5/59
    //P0_17, // J3                  MISO
    //P3_25,  // J3          33      PWM1[2]
    //P3_26,  // J3          31      PWM1[3]

//J5
    //P1_22, // J5          41
    //P1_30  // J5          37
    //P1_21, //61    Servo2    6      PWM1[3] //overlaps with Servo2?? check me
    //P0_26, // J2/Aux2  A9/63


};


//TODO:: determine pins for LCD
//SPI LCD Common Settings (RRD Full Graphic Smart Display)
constexpr SSPChannel LcdSpiChannel = SSP0;     //SSP0 (MISO0, MOSI0, SCK0)
constexpr Pin LcdCSPin =       P0_16; //LCD Chip Select
constexpr Pin LcdDCPin =       P2_6;  //DataControl Pin (A0) if none used set to NoPin
constexpr Pin LcdBeepPin =     P1_30;
constexpr Pin EncoderPinA =    P3_25;
constexpr Pin EncoderPinB =    P3_26;
constexpr Pin EncoderPinSw =   P2_11; //click
constexpr Pin PanelButtonPin = P1_22; //Extra button on Viki and RRD Panels (reset/back etc configurable)

//VIKI2.0 Specific options
constexpr Pin VikiRedLedPin = NoPin;
constexpr Pin VikiBlueLedPin = NoPin;



#endif
