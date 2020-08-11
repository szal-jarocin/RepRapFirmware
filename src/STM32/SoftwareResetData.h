//SD

#ifndef SOFTWARERESETDATA_H_
#define SOFTWARERESETDATA_H_

#include "Core.h"

typedef enum ResetCause_e
{
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_LOW_POWER_RESET,
    RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    RESET_CAUSE_SOFTWARE_RESET,
    RESET_CAUSE_POWER_ON_POWER_DOWN_RESET,
    RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    RESET_CAUSE_BROWNOUT_RESET,
} ResetCause_t;

void InitResetCause();
ResetCause_t GetResetCause();
bool IsSoftwareResetDataSlotVacant(uint8_t slot);
uint32_t *GetSoftwareResetDataSlotPtr(uint8_t slot);
void ReadSoftwareResetDataSlot(uint8_t slot, void *data, uint32_t dataLength);
bool EraseSoftwareResetDataSlots();
bool WriteSoftwareResetData(uint8_t slot, const void *data, uint32_t dataLength);

//Compatibility
inline void EraseAndReset() noexcept {};


#endif /* SOFTWARERESETDATA_H_ */
