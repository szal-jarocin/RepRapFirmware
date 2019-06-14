/*
 * Board Config
 *
 *  Created on: 3 Feb 2019
 *      Author: sdavi
 */

#ifndef BOARDCONFIG_H_
#define BOARDCONFIG_H_

#include "Core.h"
#include "RepRapFirmware.h"
#include "MessageType.h"
#include "Storage/FileStore.h"
#include "RepRap.h"


enum configValueType{
    cvPinType = 0,
    cvBoolType,
    cvUint8Type,
    cvUint16Type,
    cvUint32Type,
    cvFloatType,
    cvStringType,
};


class Platform; //fwd decl

class BoardConfig {

public:
    static void Init();

    static void Diagnostics(MessageType mtype);
    static Pin StringToPin(const char *strvalue);

private:
    BoardConfig();
    static bool GetConfigKeys(FileStore *configFile );
    static void SetValueFromString(configValueType type, void *variable, const char *valuePtr);
    static void PrintValue(MessageType mtype, configValueType configType, void *variable);
    static void PrintPinArray(MessageType mtype, Pin arr[], uint16_t numEntries);

};

#endif /* BOARDCONFIG_H_ */
