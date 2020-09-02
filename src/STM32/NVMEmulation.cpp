
//GA
#include "SoftwareReset.h"
#include "SoftwareResetData.h"
#include "RepRapFirmware.h"

__attribute__((__section__(".reset_data"), used)) uint32_t ResetData[FLASH_DATA_LENGTH/sizeof(uint32_t)];
/*
 
On STM32F4 we store reset data in flash. We have allocated a 16Kb sector for this purpose and the above
array maps to it.

*/

constexpr uint32_t SlotSize = 512/sizeof(uint32_t); // in 32 bit words
constexpr uint32_t MAX_SLOT = FLASH_DATA_LENGTH/sizeof(uint32_t)/SlotSize - 1;
constexpr uint32_t ResetDataSectorNo = 3;
static uint32_t currentSlot     = MAX_SLOT+1;
static ResetCause_t ResetCause = RESET_CAUSE_UNKNOWN;


void InitResetCause()
{

    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
    {
        ResetCause = RESET_CAUSE_LOW_POWER_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
    {
        ResetCause = RESET_CAUSE_WINDOW_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        ResetCause = RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
        ResetCause = RESET_CAUSE_SOFTWARE_RESET; // This reset is induced by calling the ARM CMSIS `NVIC_SystemReset()` function!
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
    {
        ResetCause = RESET_CAUSE_POWER_ON_POWER_DOWN_RESET;
    }
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
    {
        ResetCause = RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
    }
    // Needs to come *after* checking the `RCC_FLAG_PORRST` flag in order to ensure first that the reset cause is 
    // NOT a POR/PDR reset. See note below. 
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        ResetCause = RESET_CAUSE_BROWNOUT_RESET;
    }
    else
    {
        ResetCause = RESET_CAUSE_UNKNOWN;
    }

    // Clear all the reset flags or else they will remain set during future resets until system power is fully removed.
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

ResetCause_t GetResetCause()
{
    return ResetCause;
}

static uint32_t *GetSlotPtr(uint8_t slot)
{
    return (uint32_t *) ResetData + (slot*SlotSize);
}


//When the Sector is erased, all the bits will be high
//This checks if the first 4 bytes are all high for the designated software reset slot
//the first 2 bytes of a used reset slot will have the magic number in it.
bool IsSlotVacant(uint8_t slot)
{
    const uint32_t *p = GetSlotPtr(slot);
    
    for (size_t i = 0; i < SlotSize; ++i)
    {
        if (*p != 0xFFFFFFFF)
        {
            return false;
        }
        ++p;
    }
    return true;
}


void NVMEmulationRead(void *data, uint32_t dataLength)
{
    // find the most recently written data or slot 0 if all free
    currentSlot = MAX_SLOT;
    while (currentSlot > 0 && IsSlotVacant(currentSlot))
        currentSlot--;
    uint32_t *slotStartAddress = GetSlotPtr(currentSlot);
    memcpy(data, slotStartAddress, dataLength);
}

bool NVMEmulationErase()
{
    // Have we reached the last slot yet?
    if (currentSlot < MAX_SLOT)
    {
        currentSlot++;
        return true;
    }
    const irqflags_t flags = cpu_irq_save();
    FLASH_EraseInitTypeDef eraseInfo;
    uint32_t SectorError;
    bool ret = true;
    eraseInfo.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInfo.Sector = ResetDataSectorNo;
    eraseInfo.NbSectors = 1;
    eraseInfo.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    HAL_FLASH_Unlock();
    if (HAL_FLASHEx_Erase(&eraseInfo, &SectorError) != HAL_OK)
    {
        debugPrintf("Flash erase failed sector %d\n", static_cast<int>(SectorError));
        ret = false;
    }
    HAL_FLASH_Lock();
    cpu_irq_restore(flags);
    currentSlot = 0;    
    return ret;
    
}


bool WriteSoftwareResetData(uint8_t slot, const void *data, uint32_t dataLength)
{
    uint32_t *dst = GetSlotPtr(slot);
    uint32_t *src = (uint32_t *) data;
    uint32_t cnt = dataLength/sizeof(uint32_t);
    if (cnt*sizeof(uint32_t) != dataLength)
    {
        debugPrintf("Warning flash data not 32 bit aligned len %d\n", static_cast<int>(dataLength));
        cnt += 1;
    }
    bool ret = true;
    HAL_FLASH_Unlock();
    for(uint32_t i = 0; i < cnt; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t) dst, (uint64_t) *src) != HAL_OK)
        {
            debugPrintf("Flash write failed cnt %d\n", static_cast<int>(i));
            ret = false;
            break;
        }
        dst++;
        src++;
    }
    HAL_FLASH_Lock();   
    return ret;
}

bool NVMEmulationWrite(const void *data, uint32_t dataLength){
    uint8_t iap_ret_code;
    bool ret = false;
    if (dataLength != slotSize*sizeof(uint32_t))
    {
        debugPrintf("Bad flash data size\n");
        return false;
    }
    if (currentSlot > MAX_SLOT)
    {
        debugPrintf("Write to flash slot that has not been read slot is %d\n", (int)currentSlot);
        return false;
    }
    const irqflags_t flags = cpu_irq_save();
    uint32_t *dst = GetSlotPtr(currentSlot);
    uint32_t *src = (uint32_t *) data;
    uint32_t cnt = dataLength/sizeof(uint32_t);
    if (cnt*sizeof(uint32_t) != dataLength)
    {
        debugPrintf("Warning flash data not 32 bit aligned len %d\n", static_cast<int>(dataLength));
        cnt += 1;
    }
    bool ret = true;
    HAL_FLASH_Unlock();
    for(uint32_t i = 0; i < cnt; i++)
    {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t) dst, (uint64_t) *src) != HAL_OK)
        {
            debugPrintf("Flash write failed cnt %d\n", static_cast<int>(i));
            ret = false;
            break;
        }
        dst++;
        src++;
    }
    HAL_FLASH_Lock();      

    // Re-enable interrupt mode
    cpu_irq_restore(flags);
    return ret;
    
}

