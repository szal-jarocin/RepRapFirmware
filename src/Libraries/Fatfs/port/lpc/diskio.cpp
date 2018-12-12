/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/
//SD :: Modified to work with RRF
//SD :: Updated for RTOS




#include "diskio.h"
#include <stdio.h>
#include <string.h>
#include "SDCard.h"


#include "RepRapFirmware.h"
#include "RepRap.h"
#include "Tasks.h"


#ifndef FFSDEBUG_ENABLED
#define FFSDEBUG_ENABLED 0
#endif

#if FFSDEBUG_ENABLED
#define FFSDEBUG(FMT, ...) printf(FMT, ##__VA_ARGS__)
#else
#define FFSDEBUG(FMT, ...)
#endif

extern SDCard *_ffs[2]; //Defined in CoreLPC



//TODO:: added from RRF, but no retries implemented yet
static unsigned int highestSdRetriesDone = 0;

unsigned int DiskioGetAndClearMaxRetryCount()
{
    const unsigned int ret = highestSdRetriesDone;
    highestSdRetriesDone = 0;
    return ret;
}



DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)
{
	FFSDEBUG("disk_initialize on drv [%d]\n", drv);

    MutexLocker lock(Tasks::GetSpiMutex());

	return (DSTATUS)_ffs[drv]->disk_initialize();
}

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	FFSDEBUG("disk_status on drv [%d]\n", drv);
    MutexLocker lock(Tasks::GetSpiMutex());
    
	return (DSTATUS)_ffs[drv]->disk_status();
}

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
)
{
    MutexLocker lock(Tasks::GetSpiMutex());
    
    if (reprap.Debug(moduleStorage))
    {
        debugPrintf("Read %u %u %lu\n", drv, count, sector);
    }
    
	FFSDEBUG("disk_read(sector %d, count %d) on drv [%d]\n", sector, count, drv);
	for(unsigned int s=sector; s<sector+count; s++) {
		FFSDEBUG(" disk_read(sector %d)\n", s);
		int res = _ffs[drv]->disk_read(buff, s);
		if(res) {
			return RES_PARERR;
		}
		buff += 512;
	}
	return RES_OK;
}

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)
{
    MutexLocker lock(Tasks::GetSpiMutex());
    
    if (reprap.Debug(moduleStorage))
    {
        debugPrintf("Write %u %u %lu\n", drv, count, sector);
    }
    
	FFSDEBUG("disk_write(sector %d, count %d) on drv [%d]\n", sector, count, drv);
	for(unsigned int s=sector; s<sector+count; s++) {
		FFSDEBUG(" disk_write(sector %d)\n", s);
		int res = _ffs[drv]->disk_write(buff, sector);
		if(res) {
			return RES_PARERR;
		}
		buff += 512;
	}
	return RES_OK;
}
#endif /* _READONLY */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	FFSDEBUG("disk_ioctl(%d)\n", ctrl);
    
    DRESULT result = RES_PARERR;

	switch(ctrl) {
		case CTRL_SYNC:
        {
            MutexLocker lock(Tasks::GetSpiMutex());
            
			if(_ffs[drv] == NULL) {
				result = RES_NOTRDY;
			} else if(_ffs[drv]->disk_sync()) {
				result = RES_ERROR;
            } else {
                result = RES_OK;
            }
        }
        break;
            
		case GET_SECTOR_COUNT:
        {
			if(_ffs[drv] == NULL) {
				result = RES_NOTRDY;
			} else {
                MutexLocker lock(Tasks::GetSpiMutex());
                
				int res = _ffs[drv]->disk_sectors();
				if(res > 0) {
					*((DWORD*)buff) = res; // minimum allowed
					result = RES_OK;
				} else {
					result = RES_ERROR;
				}
			}
        }
        break;
		
        case GET_BLOCK_SIZE:{
			*((DWORD*)buff) = 1; // default when not known
			result = RES_OK;
        }
        break;

	}
	return result;
}


