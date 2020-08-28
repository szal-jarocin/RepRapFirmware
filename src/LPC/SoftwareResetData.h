//SD

#ifndef SOFTWARERESETDATA_H_
#define SOFTWARERESETDATA_H_

#include "Core.h"
#include "iap.h"


void LPC_ReadSoftwareResetData(void *data, uint32_t dataLength);
bool LPC_EraseSoftwareResetData();
bool LPC_WriteSoftwareResetData(const void *data, uint32_t dataLength);

//Compatibility
inline void EraseAndReset(){};


#endif /* SOFTWARERESETDATA_H_ */
