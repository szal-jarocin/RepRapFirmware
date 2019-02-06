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

class Platform; //fwd decl

class BoardConfig {

public:
    static void Init();

    static void Diagnostics(MessageType mtype);

    static Pin StringToPin(const char *strvalue);

private:
    BoardConfig();
    static bool GetConfigKeys(FileStore *configFile );
        
    
    Platform *platform;                                            // The instance of the RepRap hardware class
};
#endif /* BOARDCONFIG_H_ */
