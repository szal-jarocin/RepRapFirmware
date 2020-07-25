//SD

#ifndef SOFTWARERESETDATA_H_
#define SOFTWARERESETDATA_H_

#include "Core.h"
//#include "iap.h"


bool STM_IsSoftwareResetDataSlotVacant(uint8_t slot);
uint32_t *STM_GetSoftwareResetDataSlotPtr(uint8_t slot);
void STM_ReadSoftwareResetDataSlot(uint8_t slot, void *data, uint32_t dataLength);
bool STM_EraseSoftwareResetDataSlots();
bool STM_WriteSoftwareResetData(uint8_t slot, const void *data, uint32_t dataLength);

//Compatibility
inline void EraseAndReset() noexcept {};


#endif /* SOFTWARERESETDATA_H_ */
