#include "Pins_LPC.h"
#include "RepRapFirmware.h"

//Default values for configurable variables.

//The Smoothie Bootloader turns off Pins 2.4, 2.5, 2.6 and 2.7 which are used as Heater pins
//Therefore, heaters do not need to be turned off immediately and can wait until they are locaed in config file to initialise.

Pin END_STOP_PINS[NumEndstops] = {NoPin, NoPin, NoPin, NoPin, NoPin, NoPin};

Pin Z_PROBE_PIN = NoPin;
Pin Z_PROBE_MOD_PIN =   NoPin;


Pin TEMP_SENSE_PINS[NumThermistorInputs] = {NoPin, NoPin, NoPin};
Pin HEAT_ON_PINS[NumHeaters] =             {NoPin, NoPin, NoPin};


Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { NoPin, NoPin };


Pin ATX_POWER_PIN = NoPin;

uint16_t Timer1Frequency = 10; //default for Timer1 (slowPWM) for HeatBeds
uint16_t Timer3Frequency = 120; // default for Timer2 (fastPWM) for Hotends/fans etc

Pin COOLING_FAN_PINS[NUM_FANS] = { NoPin, NoPin };
Pin TachoPins[NumTachos] = { NoPin };

Pin SdCardDetectPins[NumSdCards] = { NoPin, NoPin };
Pin SdSpiCSPins[NumSdCards] = { P0_6, NoPin };// Internal, external. Note:: ("slot" 0 in CORE is configured to be LCP SSP1 to match default RRF behaviour)
uint32_t ExternalSDCardFrequency = 2000000; //default to 2MHz

Pin SpecialPinMap[MaxNumberSpecialPins] = { NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin };


Pin LcdCSPin =       NoPin; //LCD Chip Select
Pin LcdDCPin =       NoPin;  //DataControl Pin (A0) if none used set to NoPin
Pin LcdBeepPin =     NoPin;
Pin EncoderPinA =    NoPin;
Pin EncoderPinB =    NoPin;
Pin EncoderPinSw =   NoPin; //click
Pin PanelButtonPin = NoPin; //Extra button on Viki and RRD Panels (reset/back etc)

//Pin LED1 = NoPin;
//Pin LED2 = NoPin;
//Pin LED3 = NoPin;
//Pin LED4 = NoPin;
//Pin LED_PLAY = NoPin;

Pin StatusLEDPin = NoPin;
Pin DiagPin = NoPin;

Pin ENABLE_PINS[NumDirectDrivers] = {NoPin, NoPin, NoPin, NoPin, NoPin};
Pin STEP_PINS[NumDirectDrivers]  = {NoPin, NoPin, NoPin, NoPin, NoPin};
Pin DIRECTION_PINS[NumDirectDrivers]  = {NoPin, NoPin, NoPin, NoPin, NoPin};
//
//uint8_t STEP_PIN_PORT2_POS[NumDirectDrivers] = {0,0,0,0,0}; //SD: Used for calculating bitmap for stepping drivers (this is position of the pins on the port)
uint32_t STEP_DRIVER_MASK = 0; //SD: mask of the step pins on Port 2 used for writing to step pins in parallel



//TODO:: change DEFINE to bool (UPDATE IN PLATFORM.CPP FROM TYPEDEF)



bool hasDriverCurrentControl = false;
float digipotFactor = 113.33; //factor for converting current to digipot value




