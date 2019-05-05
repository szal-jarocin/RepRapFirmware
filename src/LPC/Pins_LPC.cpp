#include "RepRapFirmware.h"
#include "Pins_LPC.h"
#include "BoardConfig.h"


//Default values for configurable variables.

//The Smoothie Bootloader turns off Pins 2.4, 2.5, 2.6 and 2.7 which are used as Heater pins
//Therefore, heaters do not need to be turned off immediately and can wait until they are locaed in config file to initialise.

//Pin END_STOP_PINS[NumEndstops] = {NoPin, NoPin, NoPin, NoPin, NoPin, NoPin};

Pin Z_PROBE_PIN = NoPin;
Pin Z_PROBE_MOD_PIN =   NoPin;


Pin TEMP_SENSE_PINS[NumThermistorInputs] = {NoPin, NoPin, NoPin};
//Pin HEAT_ON_PINS[NumHeaters] =             {NoPin, NoPin, NoPin};


Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { NoPin, NoPin };


Pin ATX_POWER_PIN = NoPin;

uint16_t Timer1Frequency = 10; //default for Timer1 (slowPWM) for HeatBeds
uint16_t Timer3Frequency = 120; // default for Timer2 (fastPWM) for Hotends/fans etc

//Pin COOLING_FAN_PINS[NUM_FANS] = { NoPin, NoPin };
//Pin TachoPins[NumTachos] = { NoPin };

Pin SdCardDetectPins[NumSdCards] = { NoPin, NoPin };
Pin SdSpiCSPins[NumSdCards] = { P0_6, NoPin };// Internal, external. Note:: ("slot" 0 in CORE is configured to be LCP SSP1 to match default RRF behaviour)
uint32_t ExternalSDCardFrequency = 2000000; //default to 2MHz

uint32_t InternalSDCardFrequency = 10000000; //default to 10MHz

//Pin SpecialPinMap[MaxNumberSpecialPins] = { NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin, NoPin };


Pin LcdCSPin =       NoPin; //LCD Chip Select
Pin LcdDCPin =       NoPin;  //DataControl Pin (A0) if none used set to NoPin
Pin LcdBeepPin =     NoPin;
Pin EncoderPinA =    NoPin;
Pin EncoderPinB =    NoPin;
Pin EncoderPinSw =   NoPin; //click
Pin PanelButtonPin = NoPin; //Extra button on Viki and RRD Panels (reset/back etc)

Pin DiagPin = NoPin;

Pin ENABLE_PINS[NumDirectDrivers] = {NoPin, NoPin, NoPin, NoPin, NoPin};
Pin STEP_PINS[NumDirectDrivers]  = {NoPin, NoPin, NoPin, NoPin, NoPin};
Pin DIRECTION_PINS[NumDirectDrivers]  = {NoPin, NoPin, NoPin, NoPin, NoPin};
uint32_t STEP_DRIVER_MASK = 0; //SD: mask of the step pins on Port 2 used for writing to step pins in parallel

bool hasStepPinsOnDifferentPorts = false; //for boards that don't have all step pins on port2

bool hasDriverCurrentControl = false;
float digipotFactor = 113.33; //factor for converting current to digipot value


// Hardware-dependent pins functions

//PinEntry extends PinDescription so we will just cast g_APinDescription
PinEntry *PinTable = (PinEntry *) g_APinDescription;

// Function to look up a pin name pass back the corresponding index into the pin table
// On this platform, the mapping from pin names to pins is fixed, so this is a simple lookup
#warning TODO:: implement for LPC
bool LookupPinName(const char*pn, LogicalPin& lpin, bool& hardwareInverted)
{
    if (StringEqualsIgnoreCase(pn, NoPinName))
    {
        lpin = NoLogicalPin;
        hardwareInverted = false;
        return true;
    }
    
    bool hwInverted = false;
    
    
#warning TODO:: this only supports one pin entry and not multiple as RRF does (i.e comma seperated)
    for (size_t lp = 0; lp < NumNamedPins; ++lp)
    {
        
        const char *p = pn;
        //check modifiers
        while(*p != 0 && !isDigit(*p)){
            if(*p == '!') hwInverted = true;
            //ignore other modifiers for now?
            ++p;
        }
        
        if(BoardConfig::StringToPin(p) == PinTable[lp].pin)
        {
            lpin = (LogicalPin) lp;
            hardwareInverted = hwInverted;
            return true;
        }
    }
    
    
    
    return false;
}

// Return true if the pin can be used for the specified function
#warning TODO::implement for LPC
bool PinEntry::CanDo(PinAccess access) const
{
    return true;
    
    
    switch (access)
    {
            
        case PinAccess::read:
        case PinAccess::readWithPullup:
        case PinAccess::write0:
        case PinAccess::write1:
            return true;
            
        case PinAccess::readAnalog:
            debugPrintf("CanDo: readAnalog: %d.%d\n", (pin>>5), (pin & 0x1f) );
            return (ulADCChannelNumber != NO_ADC);
            
        case PinAccess::pwm:
            return IsPwmCapable(pin);
            
        case PinAccess::servo:
            return IsServoCapable(pin);
            
        default:
            return false;
    }
}
