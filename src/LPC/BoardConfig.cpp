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

enum singleValueType{
    svPinType = 0,
    svBoolType,
    svUint8Type,
    svUint16Type,
    svUint32Type,
    svFloatType,
};

struct pinConfig_t{
    const char* key;
    void *variable;
    singleValueType type;
};

struct pinConfigArray_t{
    const char* key;
    Pin *pins; //array of pins
    const size_t *maxArrayEntries;
};



static const pinConfig_t singleValueConfigs[]=
{
    {"lcd.lcdCSPin", &LcdCSPin, svPinType},
    {"lcd.lcdDCPin", &LcdDCPin, svPinType},
    {"lcd.lcdBeepPin", &LcdBeepPin, svPinType},
    {"lcd.encoderPinA", &EncoderPinA, svPinType},
    {"lcd.encoderPinB", &EncoderPinB, svPinType},
    {"lcd.encoderPinSw", &EncoderPinSw, svPinType},
    {"lcd.panelButtonPin", &PanelButtonPin, svPinType},

    {"leds.diagnostic", &DiagPin, svPinType},
    
    {"zProbe.pin", &Z_PROBE_PIN, svPinType},
    {"zProbe.modulationPin", &Z_PROBE_MOD_PIN, svPinType},

    {"atxPowerPin", &ATX_POWER_PIN, svPinType},

    {"steppers.hasDriverCurrentControl", &hasDriverCurrentControl, svBoolType},
    {"steppers.digipotFactor", &digipotFactor, svFloatType},
    
    {"externalSDCard.csPin", &SdSpiCSPins[1], svPinType},
    {"externalSDCard.cardDetectPin", &SdCardDetectPins[1], svPinType},
    {"externalSDCard.spiFrequencyHz", &ExternalSDCardFrequency, svUint32Type},

    {"lpc.slowPWM.frequencyHz", &Timer1Frequency, svUint16Type},
    {"lpc.fastPWM.frequencyHz", &Timer3Frequency, svUint16Type},

    


};




static const pinConfigArray_t pinArrayConfigs[] =
{
    {"stepper.enablePins", ENABLE_PINS, &MaxTotalDrivers},
    {"stepper.stepPins", STEP_PINS, &MaxTotalDrivers},
    {"stepper.directionPins", DIRECTION_PINS, &MaxTotalDrivers},
    {"endstop.pins", END_STOP_PINS, &NumEndstops},
    {"heat.tempSensePins", TEMP_SENSE_PINS, &NumThermistorInputs},
    {"heat.heatOnPins", HEAT_ON_PINS, &NumHeaters},
    {"heat.spiTempSensorCSPins", SpiTempSensorCsPins, &MaxSpiTempSensors},
    {"fan.pins", COOLING_FAN_PINS, &NUM_FANS},
    {"fan.tachoPins", TachoPins, &NumTachos},
    {"specialPinMap", SpecialPinMap, &MaxNumberSpecialPins},
    {"lpc.externalInterruptPins", ExternalInterruptPins, &MaxExtIntEntries},
    {"lpc.slowPWM.pins", Timer1PWMPins, &MaxTimerEntries},
    {"lpc.servoPins", Timer2PWMPins, &MaxTimerEntries},
    {"lpc.fastPWM.pins", Timer3PWMPins, &MaxTimerEntries},
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
        if (reprap.GetPlatform().GetMassStorage()->FileExists(reprap.GetPlatform().GetSysDir(), "board.txt"))
        {
            configFile = reprap.GetPlatform().OpenFile(reprap.GetPlatform().GetSysDir(), "board.txt", OpenMode::read);
        }
        else
        {
            reprap.GetPlatform().MessageF(UsbMessage, "Unable to read board configuration: %sboard.txt...\n",reprap.GetPlatform().GetSysDir() );
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
        
        reprap.GetPlatform().MessageF(UsbMessage, "Loading config from 0:/sys/board.txt...\n" );

        //uint32_t time = millis();
        BoardConfig::GetConfigKeys(configFile);
        configFile->Close();
        //time = millis() - time;
        //reprap.GetPlatform().MessageF(UsbMessage, "Took %ld ms to load config\n", time );

        
        //Calculate STEP_DRIVER_MASK (used for parallel writes)
        for(size_t i=0; i<MaxTotalDrivers; i++)
        {
            //It is assumed all pins will be on Port 2
            const Pin stepPin = STEP_PINS[i];
            STEP_DRIVER_MASK = 0;
            if( (stepPin >> 5) == 2) // divide by 32 to get port number
            {
                STEP_DRIVER_MASK |= (1 << (stepPin & 0x1f)); //this is a bitmask of all the stepper pins on Port2 used for Parallel Writes
            }
            else
            {
                if(stepPin != NoPin) debugPrintf("Error: Step Pin %d not on Port2\n", stepPin);
            }
        }

        //Configure the HW Timers
        ConfigureTimerForPWM(0, Timer1Frequency); //Timer1 is channel 0
        ConfigureTimerForPWM(1, 50);              //Setup Servos for 50Hz. Timer2 is channel 1
        ConfigureTimerForPWM(2, Timer3Frequency); //Timer3 is channel 2


        //Configure the External SDCard
        if(SdSpiCSPins[1] != NoPin)
        {
            //set the CSPin and the frequency for the External SDCard
            sd_mmc_reinit_slot(1, SdSpiCSPins[1], ExternalSDCardFrequency);
        }
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



void BoardConfig::Diagnostics(MessageType mtype)
{

    
    reprap.GetPlatform().MessageF(mtype, "== Configurable Board Settings ==\n");

    
    //Pin Array Configs
    const size_t numConfigs = ARRAY_SIZE(pinArrayConfigs);
    for(size_t i=0; i<numConfigs; i++)
    {
        pinConfigArray_t next = pinArrayConfigs[i];
        reprap.GetPlatform().MessageF(mtype, "%s = [ ", next.key );
        for(size_t p=0; p<*(next.maxArrayEntries); p++)
        {
            Pin pin = next.pins[p];
            if(pin == NoPin)
            {
                reprap.GetPlatform().MessageF(mtype, "NoPin ");
            }
            else
            {
                reprap.GetPlatform().MessageF(mtype, "%d.%d ", (pin >> 5), (pin & 0x1F) );
            }
        }
        reprap.GetPlatform().MessageF(mtype, "]\n");
    }
    
    //single pin values
    const size_t numSingleConfigs = ARRAY_SIZE(singleValueConfigs);
    for(size_t i=0; i<numSingleConfigs; i++)
    {
        pinConfig_t next = singleValueConfigs[i];
        
        reprap.GetPlatform().MessageF(mtype, "%s =  ", next.key);
        
        switch(next.type){
            case svPinType:
                {
                    Pin pin = *(Pin *)(next.variable);
                    if(pin == NoPin)
                    {
                        reprap.GetPlatform().MessageF(mtype, "NoPin\n");
                    }
                    else
                    {
                        reprap.GetPlatform().MessageF(mtype, "%d.%d\n", (pin >> 5), (pin & 0x1F) );
                    }
                }
                break;
                
            case svBoolType:
                reprap.GetPlatform().MessageF(mtype, "%s\n", (*(bool *)(next.variable) == true)?"true":"false" );
                break;
                
            case svFloatType:
                reprap.GetPlatform().MessageF(mtype, "%.2f\n",  (double) *(float *)(next.variable) );
                break;
            case svUint8Type:
                reprap.GetPlatform().MessageF(mtype, "%u\n",  *(uint8_t *)(next.variable) );
                break;
            case svUint16Type:
                reprap.GetPlatform().MessageF(mtype, "%d\n",  *(uint16_t *)(next.variable) );
                break;
            case svUint32Type:
                reprap.GetPlatform().MessageF(mtype, "%lu\n",  *(uint32_t *)(next.variable) );
                break;
            default:{
                
            }
        }
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

                    //Array (only Pin arrays are implemented
                    const size_t numConfigs = ARRAY_SIZE(pinArrayConfigs);
                    for(size_t i=0; i<numConfigs; i++)
                    {
                        pinConfigArray_t next = pinArrayConfigs[i];
                        if(StringEqualsIgnoreCase(key, next.key)){
                            //match

                            //create a temp array to read into. Only copys the arrays into the final destination when we know the array is properly defined
                            const size_t maxArraySize = *next.maxArrayEntries;
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
                                                next.pins[i] = readArray[i];
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

                        const size_t numConfigs = ARRAY_SIZE(singleValueConfigs);
                        for(size_t i=0; i<numConfigs; i++){
                            pinConfig_t next = singleValueConfigs[i];
                            if(StringEqualsIgnoreCase(key, next.key)){
                                //match
                                switch(next.type){
                                    case svPinType:
                                        *(Pin *)(next.variable) = StringToPin(valuePtr);
                                        break;

                                    case svBoolType:{
                                        bool res = false;

                                        if(strlen(valuePtr) == 1){
                                            //check for 0 or 1
                                            if(valuePtr[0] == '1') res = true;
                                        }
                                        else if(strlen(valuePtr) == 4 && StringEqualsIgnoreCase(valuePtr, "true"))
                                        {
                                            res = true;
                                        }
                                        *(bool *)(next.variable) = res;
                                    }
                                        break;

                                    case svFloatType:{
                                        const char *ptr = nullptr;
                                        *(float *)(next.variable) = SafeStrtof(valuePtr, &ptr);
                                    }
                                        break;
                                    case svUint8Type:{
                                        const char *ptr = nullptr;
                                        uint8_t val = SafeStrtoul(valuePtr, &ptr, 10);
                                        if(val < 0) val = 0;
                                        if(val > 0xFF) val = 0xFF;

                                        *(uint8_t *)(next.variable) = val;
                                    }
                                        break;
                                    case svUint16Type:{
                                        const char *ptr = nullptr;
                                        uint16_t val = SafeStrtoul(valuePtr, &ptr, 10);
                                        if(val < 0) val = 0;
                                        if(val > 0xFFFF) val = 0xFFFF;

                                        *(uint16_t *)(next.variable) = val;

                                    }
                                        break;
                                    case svUint32Type:{
                                        const char *ptr = nullptr;
                                        *(uint32_t *)(next.variable) = SafeStrtoul(valuePtr, &ptr, 10);
                                    }
                                        break;
                                    default:{

                                        debugPrintf("Unhandled SingleValueType\n");

                                    }
                                }
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
