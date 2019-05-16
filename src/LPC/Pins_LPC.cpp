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
Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { NoPin, NoPin };


Pin ATX_POWER_PIN = NoPin;

uint16_t Timer1Frequency = 10; //default for Timer1 (slowPWM) for HeatBeds
uint16_t Timer3Frequency = 120; // default for Timer2 (fastPWM) for Hotends/fans etc

Pin SdCardDetectPins[NumSdCards] = { NoPin, NoPin };
Pin SdSpiCSPins[NumSdCards] = { P0_6, NoPin };// Internal, external. Note:: ("slot" 0 in CORE is configured to be LCP SSP1 to match default RRF behaviour)
uint32_t ExternalSDCardFrequency = 2000000; //default to 2MHz
uint32_t InternalSDCardFrequency = 10000000; //default to 10MHz


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
float digipotFactor = 113.33; //defualt factor for converting current to digipot value

bool UARTPanelDueMode = false; //disable PanelDue support by default



#ifdef __MBED__
    PinEntry *PinTable = (PinEntry *) PinTable_Mbed;
    size_t NumNamedLPCPins = ARRAY_SIZE(PinTable_Mbed);
#else

#error TODO:: Need a default PinTable and NumNamedPins.....

#endif

char lpcBoardName[MaxBoardNameLength] = "";

bool IsEmptyPinArray(Pin *arr, size_t len)
{
    for(size_t i=0; i<len; i++)
    {
        if(arr[i] != NoPin) return false;
    }
    
    return true;
}

void SetDefaultPinArray(const Pin *src, Pin *dst, size_t len)
{
    if(IsEmptyPinArray(dst, len))
    {
        
        //array is empty from board.txt config, set to defaults
        for(size_t i=0; i<len; i++)
        {
            dst[i] = src[i];
        }
    }
}

bool SetBoard(const char* bn)
{
    
    const size_t numBoards = ARRAY_SIZE(LPC_Boards);

    for(size_t i=0; i<numBoards; i++)
    {
        if(StringEqualsIgnoreCase(bn, LPC_Boards[i].boardName))
        {
            PinTable = (PinEntry *)LPC_Boards[i].boardPinTable;
            NumNamedLPCPins = LPC_Boards[i].numNamedEntries;

            //copy defaults (if needed)
            SetDefaultPinArray(LPC_Boards[i].defaults.enablePins, ENABLE_PINS, MaxTotalDrivers);
            SetDefaultPinArray(LPC_Boards[i].defaults.stepPins, STEP_PINS, MaxTotalDrivers);
            SetDefaultPinArray(LPC_Boards[i].defaults.dirPins, DIRECTION_PINS, MaxTotalDrivers);

            hasDriverCurrentControl = LPC_Boards[i].defaults.hasDriverCurrentControl;
            digipotFactor = LPC_Boards[i].defaults.digipotFactor;
            
            SetDefaultPinArray(LPC_Boards[i].defaults.slowPwmPins, Timer1PWMPins, MaxTimerEntries);
            SetDefaultPinArray(LPC_Boards[i].defaults.fastPwmPins, Timer3PWMPins, MaxTimerEntries);
            SetDefaultPinArray(LPC_Boards[i].defaults.servoPwmPins, Timer2PWMPins, MaxTimerEntries);
            
            return true;
        }
    }
    
    return false;
}



// Function to look up a pin name pass back the corresponding index into the pin table
// On this platform, the mapping from pin names to pins is fixed, so this is a simple lookup
bool LookupPinName(const char*pn, LogicalPin& lpin, bool& hardwareInverted)
{
    if (StringEqualsIgnoreCase(pn, NoPinName))
    {
        lpin = NoLogicalPin;
        hardwareInverted = false;
        return true;
    }
    
    for (size_t lp = 0; lp < NumNamedLPCPins; ++lp)
    {
        const char *q = PinTable[lp].names;
        while (*q != 0)
        {
            // Try the next alias in the list of names for this pin
            const char *p = pn;
            bool hwInverted = (*q == '!');
            if (hwInverted)
            {
                ++q;
            }
            while (*q != ',' && *q != 0 && *p == *q)
            {
                ++p;
                ++q;
            }
            if (*p == 0 && (*q == 0 || *q == ','))
            {
                // Found a match
                lpin = (LogicalPin)lp;
                hardwareInverted = hwInverted;
                return true;
            }
            
            // Skip to the start of the next alias
            while (*q != 0 && *q != ',')
            {
                ++q;
            }
            if (*q == ',')
            {
                ++q;
            }
        }
    }
    return false;
}




// Return true if the pin can be used for the specified function
bool PinEntry::CanDo(PinAccess access) const
{
    switch (access)
    {
            
        case PinAccess::read:
        case PinAccess::readWithPullup:
            return ((uint8_t)cap & (uint8_t)PinCapability::read) != 0;
            
        case PinAccess::write0:
        case PinAccess::write1:
            return ((uint8_t)cap & (uint8_t)PinCapability::write) != 0;
            
        case PinAccess::readAnalog:
            //return (ulADCChannelNumber != NO_ADC);
            return ((uint8_t)cap & (uint8_t)PinCapability::ain) != 0;
            
        case PinAccess::pwm:
            //return ((uint8_t)cap & (uint8_t)PinCapability::pwm) != 0;
            return IsPwmCapable(pin);
            
        case PinAccess::servo:
            return IsServoCapable(pin);
            
        default:
            return false;
    }
}
