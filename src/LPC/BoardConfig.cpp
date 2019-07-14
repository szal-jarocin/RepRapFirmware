/*
 * Board Config
 *
 *  Created on: 3 Feb 2019
 *      Author: sdavi
 */



#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include "BoardConfig.h"
#include "RepRapFirmware.h"
#include "GCodes/GCodeResult.h"
#include "sd_mmc.h"

#include "Platform.h"

struct boardConfigEntry_t{
    const char* key;
    void *variable;
    const size_t *maxArrayEntries;
    configValueType type;
};


static const boardConfigEntry_t boardConfigs[]=
{

    {"lpc.board", &lpcBoardName, nullptr, cvStringType},

    {"leds.diagnostic", &DiagPin, nullptr, cvPinType},
    {"lpc.internalSDCard.spiFrequencyHz", &InternalSDCardFrequency, nullptr, cvUint32Type},

    {"stepper.enablePins", ENABLE_PINS, &MaxTotalDrivers, cvPinType},
    {"stepper.stepPins", STEP_PINS, &MaxTotalDrivers, cvPinType},
    {"stepper.directionPins", DIRECTION_PINS, &MaxTotalDrivers, cvPinType},
    {"stepper.hasDriverCurrentControl", &hasDriverCurrentControl, nullptr, cvBoolType},
    {"stepper.digipotFactor", &digipotFactor, nullptr, cvFloatType},

    
    //{"endstop.pins", END_STOP_PINS, &NumEndstops, cvPinType},
    //{"zProbe.pin", &Z_PROBE_PIN, nullptr, cvPinType},
    //{"zProbe.modulationPin", &Z_PROBE_MOD_PIN, nullptr, cvPinType},

    {"heat.tempSensePins", TEMP_SENSE_PINS, &NumThermistorInputs, cvPinType},
    //{"heat.heatOnPins", HEAT_ON_PINS, &NumHeaters, cvPinType},
    {"heat.spiTempSensorCSPins", SpiTempSensorCsPins, &MaxSpiTempSensors, cvPinType},
    
    //{"fan.pins", COOLING_FAN_PINS, &NUM_FANS, cvPinType},
    //{"fan.tachoPins", TachoPins, &NumTachos, cvPinType},
    
    {"atxPowerPin", &ATX_POWER_PIN, nullptr, cvPinType},
    {"atxPowerPinInverted", &ATX_POWER_INVERTED, nullptr, cvBoolType},
    
    {"lpc.HWPWM.frequencyHz", &HardwarePWMFrequency, nullptr, cvUint16Type},
    
    {"lpc.slowPWM.frequencyHz", &Timer1Frequency, nullptr, cvUint16Type},
    //{"lpc.slowPWM.pins", Timer1PWMPins, &MaxTimerEntries, cvPinType},

    {"lpc.fastPWM.frequencyHz", &Timer3Frequency, nullptr, cvUint16Type},
    //{"lpc.fastPWM.pins", Timer3PWMPins, &MaxTimerEntries, cvPinType},

    //{"lpc.servoPins", Timer2PWMPins, &MaxTimerEntries, cvPinType},

    //{"specialPinMap", SpecialPinMap, &MaxNumberSpecialPins, cvPinType},
    //{"lpc.externalInterruptPins", ExternalInterruptPins, &MaxExtIntEntries, cvPinType},
    
    {"externalSDCard.csPin", &SdSpiCSPins[1], nullptr, cvPinType},
    {"externalSDCard.cardDetectPin", &SdCardDetectPins[1], nullptr, cvPinType},
    {"lpc.externalSDCard.spiFrequencyHz", &ExternalSDCardFrequency, nullptr, cvUint32Type},
    
    {"lcd.lcdCSPin", &LcdCSPin, nullptr, cvPinType},
    {"lcd.lcdBeepPin", &LcdBeepPin, nullptr, cvPinType},
    {"lcd.encoderPinA", &EncoderPinA, nullptr, cvPinType},
    {"lcd.encoderPinB", &EncoderPinB, nullptr, cvPinType},
    {"lcd.encoderPinSw", &EncoderPinSw, nullptr, cvPinType},
    {"lcd.lcdDCPin", &LcdDCPin, nullptr, cvPinType},
    {"lcd.panelButtonPin", &PanelButtonPin, nullptr, cvPinType},

    {"lpc.uartPanelDueMode", &UARTPanelDueMode, nullptr, cvBoolType},
    
};


static inline bool isSpaceOrTab(char c)
{
    return (c == ' ' || c == '\t');
}
    
BoardConfig::BoardConfig()
{
    
}


void BoardConfig::Init() {

    GCodeResult rslt;
    String<100> reply;
    FileStore *configFile = nullptr;

    
    
    //Mount the Internal SDCard
    do
    {
        rslt = reprap.GetPlatform().GetMassStorage()->Mount(0, reply.GetRef(), false);
    }
    while (rslt == GCodeResult::notFinished);
    
    if (rslt == GCodeResult::ok)
    {
        //Open File
        if (reprap.GetPlatform().SysFileExists("board.txt"))
        {
            configFile = reprap.GetPlatform().OpenFile(DEFAULT_SYS_DIR, "board.txt", OpenMode::read);
        }
        else
        {
            delay(3000);        // Wait a few seconds so users have a chance to see this
            reprap.GetPlatform().MessageF(UsbMessage, "Unable to read board configuration: %sboard.txt...\n",DEFAULT_SYS_DIR );
            return;
        }
    }
    else
    {
        // failed to mount card
        delay(3000);        // Wait a few seconds so users have a chance to see this
        reprap.GetPlatform().MessageF(UsbMessage, "%s\n", reply.c_str());
        return;
    }
    
    if(configFile != nullptr){
        
        reprap.GetPlatform().MessageF(UsbMessage, "Loading config from %sboard.txt...\n", DEFAULT_SYS_DIR );

        //uint32_t time = millis();
        BoardConfig::GetConfigKeys(configFile);
        configFile->Close();
        
        //time = millis() - time;
        //reprap.GetPlatform().MessageF(UsbMessage, "Took %ld ms to load config\n", time );

        
        if(!SetBoard(lpcBoardName)) // load the Correct PinTable for the defined Board (RRF3)
        {
            reprap.GetPlatform().MessageF(UsbMessage, "Unknown board: %s\n", lpcBoardName );
        }
        
        
        
        //Calculate STEP_DRIVER_MASK (used for parallel writes)
        STEP_DRIVER_MASK = 0;
        for(size_t i=0; i<MaxTotalDrivers; i++)
        {
            //It is assumed all pins will be on Port 2
            const Pin stepPin = STEP_PINS[i];
            if( stepPin != NoPin && (stepPin >> 5) == 2) // divide by 32 to get port number
            {
                STEP_DRIVER_MASK |= (1 << (stepPin & 0x1f)); //this is a bitmask of all the stepper pins on Port2 used for Parallel Writes
            }
            else
            {
                if(stepPin != NoPin)
                {
                    hasStepPinsOnDifferentPorts = true;
                    // will use a basic mapping for stepping instead of parallel port writes.....
                    reprap.GetPlatform().MessageF(UsbMessage, "Step pins are on different ports. Pins will not be written in parallel.\n" );
                }
            }
        }
        

        //Configure the HW Timers
        ConfigureTimerForPWM(0, Timer1Frequency); //Timer1 is channel 0
        ConfigureTimerForPWM(1, 50);              //Setup Servos for 50Hz. Timer2 is channel 1
        ConfigureTimerForPWM(2, Timer3Frequency); //Timer3 is channel 2


        //Internal SDCard SPI Frequency
        sd_mmc_reinit_slot(0, SdSpiCSPins[0], InternalSDCardFrequency);
        
        //Configure the External SDCard
        if(SdSpiCSPins[1] != NoPin)
        {
            //set the CSPin and the frequency for the External SDCard
            sd_mmc_reinit_slot(1, SdSpiCSPins[1], ExternalSDCardFrequency);
        }
        
        //Init pins from LCD (not used anywhere yet)
        //make sure to init ButtonPin as input incase user presses button
        if(PanelButtonPin != NoPin) pinMode(PanelButtonPin, INPUT); //unused
        if(LcdDCPin != NoPin) pinMode(LcdDCPin, OUTPUT_HIGH); //unused
        
        if(LcdBeepPin != NoPin) pinMode(LcdBeepPin, OUTPUT_LOW);
        // Set the 12864 display CS pin low to prevent it from receiving garbage due to other SPI traffic
        if(LcdCSPin != NoPin) pinMode(LcdCSPin, OUTPUT_LOW);

        
    }
}


//Convert a pin string into a RRF Pin
Pin BoardConfig::StringToPin(const char *strvalue){
    //check size.. should be 3chars or 4 chars i.e. 0.1, 2.25, 2nd char should always be .
    uint8_t len = strlen(strvalue);
    if((len == 3 || len == 4) && strvalue[1] == '.' ){
        const char *ptr = NULL;
        
        uint8_t port = SafeStrtol(strvalue, &ptr, 10);
        if(ptr > strvalue && port <= 4){
            uint8_t pin = SafeStrtol(strvalue+2, &ptr, 10);
            
            if(ptr > strvalue+2 && pin < 32){
                //Convert the Port and Pin to match the arrays in CoreLPC
                Pin lpcpin = (Pin) ( (port << 5) | pin);
                return lpcpin;
            }
        }
    }
    //debugPrintf("Invalid pin %s, defaulting to NoPin\n", strvalue);
    
    return NoPin;
}



void BoardConfig::PrintValue(MessageType mtype, configValueType configType, void *variable)
{
    switch(configType){
        case cvPinType:
            {
                Pin pin = *(Pin *)(variable);
                if(pin == NoPin)
                {
                    reprap.GetPlatform().MessageF(mtype, "NoPin ");
                }
                else
                {
                    reprap.GetPlatform().MessageF(mtype, "%d.%d ", (pin >> 5), (pin & 0x1F) );
                }
            }
            break;
        case cvBoolType:
            reprap.GetPlatform().MessageF(mtype, "%s ", (*(bool *)(variable) == true)?"true":"false" );
            break;
        case cvFloatType:
            reprap.GetPlatform().MessageF(mtype, "%.2f ",  (double) *(float *)(variable) );
            break;
        case cvUint8Type:
            reprap.GetPlatform().MessageF(mtype, "%u ",  *(uint8_t *)(variable) );
            break;
        case cvUint16Type:
            reprap.GetPlatform().MessageF(mtype, "%d ",  *(uint16_t *)(variable) );
            break;
        case cvUint32Type:
            reprap.GetPlatform().MessageF(mtype, "%lu ",  *(uint32_t *)(variable) );
            break;
        case cvStringType:
            reprap.GetPlatform().MessageF(mtype, "%s ",  (char *)(variable) );
            break;
        default:{
            
        }
    }
}

void BoardConfig::Diagnostics(MessageType mtype)
{

    
    reprap.GetPlatform().MessageF(mtype, "== Configurable Board.txt Settings ==\n");

    
    //Pin Array Configs
    const size_t numConfigs = ARRAY_SIZE(boardConfigs);
    for(size_t i=0; i<numConfigs; i++)
    {
        boardConfigEntry_t next = boardConfigs[i];

        reprap.GetPlatform().MessageF(mtype, "%s = ", next.key );
        if(next.maxArrayEntries != nullptr)
        {
            reprap.GetPlatform().MessageF(mtype, "[ ");
            for(size_t p=0; p<*(next.maxArrayEntries); p++)
            {
                //TODO:: handle other values
                if(next.type == cvPinType){
                    BoardConfig::PrintValue(mtype, next.type, (void *)&((Pin *)(next.variable))[p]);
                }
            }
            reprap.GetPlatform().MessageF(mtype, "]\n");
        }
        else
        {
            BoardConfig::PrintValue(mtype, next.type, next.variable);
            reprap.GetPlatform().MessageF(mtype, "\n");

        }
    }
    
    reprap.GetPlatform().MessageF(mtype, "\n=== Other LPC Hardware Settings ===\n");
    
    //Print out the PWM and timers freq
    LPCPWMInfo freqs = {};
    GetTimerInfo(&freqs);
    reprap.GetPlatform().MessageF(mtype, "Hardware PWM=%dHz ", freqs.hwPWMFreq );
    PrintPinArray(mtype, UsedHardwarePWMChannel, NumPwmChannels);

    reprap.GetPlatform().MessageF(mtype, "Timer 1 PWM=%dHz ", freqs.tim1Freq );
    PrintPinArray(mtype, Timer1PWMPins, MaxTimerEntries);
    reprap.GetPlatform().MessageF(mtype, "Timer 2 PWM=%dHz ", freqs.tim2Freq );
    PrintPinArray(mtype, Timer2PWMPins, MaxTimerEntries);
    reprap.GetPlatform().MessageF(mtype, "Timer 3 PWM=%dHz ", freqs.tim3Freq );
    PrintPinArray(mtype, Timer3PWMPins, MaxTimerEntries);
}

void BoardConfig::PrintPinArray(MessageType mtype, Pin arr[], uint16_t numEntries)
{
    reprap.GetPlatform().MessageF(mtype, "[ ");
    for(uint8_t i=0; i<numEntries; i++)
    {
        if(arr[i] != NoPin)
        {
            reprap.GetPlatform().MessageF(mtype, "%d.%d ", (arr[i]>>5), (arr[i] & 0x1f));
        }
        else
        {
            reprap.GetPlatform().MessageF(mtype, "NoPin ");
        }
        
    }
    reprap.GetPlatform().MessageF(mtype, "]\n");
}


                  
void BoardConfig::SetValueFromString(configValueType type, void *variable, const char *valuePtr)
{
    switch(type)
    {
        case cvPinType:
            *(Pin *)(variable) = StringToPin(valuePtr);
            break;
            
        case cvBoolType:
            {
                bool res = false;
                
                if(strlen(valuePtr) == 1){
                    //check for 0 or 1
                    if(valuePtr[0] == '1') res = true;
                        }
                else if(strlen(valuePtr) == 4 && StringEqualsIgnoreCase(valuePtr, "true"))
                {
                    res = true;
                }
                *(bool *)(variable) = res;
            }
            break;
            
        case cvFloatType:
            {
                const char *ptr = nullptr;
                *(float *)(variable) = SafeStrtof(valuePtr, &ptr);
            }
            break;
        case cvUint8Type:
            {
                const char *ptr = nullptr;
                uint8_t val = SafeStrtoul(valuePtr, &ptr, 10);
                if(val < 0) val = 0;
                if(val > 0xFF) val = 0xFF;
                
                *(uint8_t *)(variable) = val;
            }
            break;
        case cvUint16Type:
            {
                const char *ptr = nullptr;
                uint16_t val = SafeStrtoul(valuePtr, &ptr, 10);
                if(val < 0) val = 0;
                if(val > 0xFFFF) val = 0xFFFF;
                
                *(uint16_t *)(variable) = val;
                    
            }
            break;
        case cvUint32Type:
            {
                const char *ptr = nullptr;
                *(uint32_t *)(variable) = SafeStrtoul(valuePtr, &ptr, 10);
            }
            break;
            
        case cvStringType:
            {
                
                //TODO:: string Type only handles Board Name variable
                if(strlen(valuePtr)+1 < MaxBoardNameLength){
                    strcpy((char *)(variable), valuePtr);
                }
            }
            break;
            
        default:
            debugPrintf("Unhandled ValueType\n");
    }
}



bool BoardConfig::GetConfigKeys(FileStore *configFile){

    size_t maxLineLength = 120;
    char line[maxLineLength];

    FilePosition fileSize = configFile->Length();
    size_t len = configFile->ReadLine(line, maxLineLength);
    while(len >= 0 && configFile->Position() != fileSize ) //
    {
        size_t pos = 0;
        while(pos < len && isSpaceOrTab(line[pos] && line[pos] != 0) == true) pos++;

        if(pos < len){

            //check for comments
            if(line[pos] == '/' || line[pos] == '#'){
                //Comment - Skipping
            } else {

                const char* key = line + pos;
                while(pos < len && !isSpaceOrTab(line[pos]) && line[pos] != '=' && line[pos] != 0) pos++;
                line[pos] = 0;// null terminate the string

                pos++;

                //eat whitespace and = if needed
                while(pos < maxLineLength && line[pos] != 0 && (isSpaceOrTab(line[pos]) == true || line[pos] == '=') ) pos++; //skip spaces and =

                //debugPrintf("Key: %s", key);

                if(pos < len && line[pos] == '{'){
                    pos++; //skip the {

                    //Array of Values:
                    //TODO:: only Pin arrays are currently implemented
                    
                    const size_t numConfigs = ARRAY_SIZE(boardConfigs);
                    for(size_t i=0; i<numConfigs; i++)
                    {
                        boardConfigEntry_t next = boardConfigs[i];
                        //Currently only handles Arrays of Pins
                        if(next.maxArrayEntries != nullptr /*&& next.type == cvPinType*/ && StringEqualsIgnoreCase(key, next.key)){
                            //match

                            //create a temp array to read into. Only copy the array entries into the final destination when we know the array is properly defined
                            const size_t maxArraySize = *next.maxArrayEntries;
                            
                            //Pin Array Type
                            Pin readArray[maxArraySize];

                            //eat whitespace
                            while(pos < maxLineLength && line[pos] != 0 && isSpaceOrTab(line[pos]) == true ) pos++;


                            //debugPrintf("[Array]: ");

                            bool searching = true;

                            size_t arrIdx = 0;

                            //search for values in Array
                            while( searching ){
                                if(pos < maxLineLength){

                                    while(pos < maxLineLength && (isSpaceOrTab(line[pos]) == true)) pos++; // eat whitespace

                                    if(pos == maxLineLength){
                                        debugPrintf("Got to end of line before end of array, line must be longer than maxLineLength");
                                        searching = false;
                                        break;
                                    }

                                    bool closedSuccessfully = false;
                                    //check brace isnt closed
                                    if(pos < maxLineLength && line[pos] == '}')
                                    {
                                        closedSuccessfully = true;
                                        arrIdx--; // we got the closing brace before getting a value this round, decrement arrIdx
                                    }
                                    else
                                    {

                                        if(arrIdx >= maxArraySize ) {
                                            debugPrintf("Error : Too many entries defined in config for array\n");
                                            searching = false;
                                            break;
                                        }

                                        //Try to Read the next Value

                                        //should be at first char of value now
                                        const char *valuePtr = line+pos;

                                        //read until end condition - space,comma,}  or null / # ;
                                        while(pos < maxLineLength && line[pos] != 0 && !isSpaceOrTab(line[pos]) && line[pos] != ',' && line[pos] != '}' && line[pos] != '/' && line[pos] != '#' && line[pos] != ';'){
                                            pos++;
                                        }

                                        //see if we ended due to comment, ;, or null
                                        if(pos == maxLineLength || line[pos] == 0 || line[pos] == '/' || line[pos] == '#' || line[pos]==';')
                                        {
                                            debugPrintf("Error: Array ended without Closing Brace?\n");
                                            searching = false;
                                            break;
                                        }

                                        //check if there is a closing brace after value without any whitespace, before it gets overwritten with a null
                                        if(line[pos] == '}')
                                        {
                                            closedSuccessfully = true;
                                        }

                                        line[pos] = 0; // null terminate the string

                                        //debugPrintf("%s ", valuePtr);

                                        //Put into the Temp Array
                                        if(arrIdx >= 0 && arrIdx<maxArraySize)
                                        {
                                            readArray[arrIdx] = StringToPin(valuePtr);
                                            
                                            //TODO:: HANDLE OTHER VALUE TYPES??
                                            

                                        }
                                    }

                                    if(closedSuccessfully == true){
                                        //Array Closed - Finished Searching
                                        //debugPrintf("]\n");

                                        if(arrIdx >= 0 && arrIdx < maxArraySize){ //arrIndx will be -1 if closed before reading any values
                                            //debugPrintf("Copying Array into dst (%d items)\n", arrIdx+1);
                                            //All values read successfully, copy temp array into Final destination
                                            //dest array may be larger, dont overrite the default values
                                            for(size_t i=0; i<(arrIdx+1); i++ ){
                                                ((Pin *)(next.variable))[i] = readArray[i];
                                            }
                                            //Success!
                                            searching = false;
                                            break;

                                        }
                                        //failed to set values
                                        searching = false;
                                        break;

                                    }
                                    arrIdx++;
                                    pos++;

                                } else {
                                    debugPrintf("Unable to find values for Key\n");
                                    searching = false;
                                    break;
                                }
                            }//end while(searching)
                        }//end if matched key

                    }//end for




                } else {
                    //single value


                    if(pos < maxLineLength && line[pos] != 0)
                    {
                        //should be at first char of value now
                        const char *valuePtr = line+pos;

                        //read until end condition - space, ;, comment, null,etc
                        while(pos < maxLineLength && line[pos] != 0 && !isSpaceOrTab(line[pos]) && line[pos] != ';' && line[pos] != '/') pos++;

                        //overrite the end condition with null....
                        line[pos] = 0; // null terminate the string

                        //debugPrintf(" = %s\n", valuePtr);
                        const size_t numConfigs = ARRAY_SIZE(boardConfigs);
                        for(size_t i=0; i<numConfigs; i++)
                        {
                            boardConfigEntry_t next = boardConfigs[i];
                            //Single Value config entries have nullptr for maxArrayEntries
                            if(next.maxArrayEntries == nullptr && StringEqualsIgnoreCase(key, next.key))
                            {
                                //match
                                BoardConfig::SetValueFromString(next.type, next.variable, valuePtr);
                            }
                        }
                    }
                }
            }

        }else {
            //Empty Line - Nothing to do here
            //debugPrintf("Empty Line\n");
        }


        len = configFile->ReadLine(line, maxLineLength);
    }
    return false;
}
