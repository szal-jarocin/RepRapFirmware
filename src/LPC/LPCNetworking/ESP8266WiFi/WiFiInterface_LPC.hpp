//Author: sdavi

#include "chip.h" //LPC Open
#include "DMA.h"
#include "HardwareSPI.h"

//ESP connected to SSP0
static HardwareSPI *spiDevice;

//maintain compatibility with RRF code
void spi_disable(SSPChannel sspChannel) noexcept
{
    spiDevice->disable();
}

static inline void DisableSpi() noexcept
{
	spi_disable(ESP_SPI);
}

static inline void spi_dma_disable() noexcept
{
    // For RRF compatibility
}

static bool spi_dma_check_rx_complete() noexcept
{
    //DMA Interrupt will notifiy when transfer is complete, just return true here
    return true;
}


// SPI completion handler
void ESP_SPI_HANDLER(HardwareSPI *spiDevice) noexcept
{
	wifiInterface->SpiInterrupt();
}

void WiFiInterface::spi_slave_dma_setup(uint32_t dataOutSize, uint32_t dataInSize) noexcept
{
    //Find the largest transfer size
    const uint32_t dsize = MAX(dataOutSize + sizeof(MessageHeaderSamToEsp), dataInSize + sizeof(MessageHeaderEspToSam));
    // clear any previous transaction
    spiDevice->disable();
    spiDevice->startTransfer((const uint8_t *)bufferOut, (uint8_t *)bufferIn, dsize, ESP_SPI_HANDLER);
}

void WiFiInterface::SpiInterrupt() noexcept
{
    digitalWrite(SamTfrReadyPin, LOW);

    transferPending = false;
    TaskBase::GiveFromISR(espWaitingTask);
}


// Set up the SPI system
void WiFiInterface::SetupSpi() noexcept
{
    spiDevice = &HardwareSPI::SSP0;
    //In Slave mode, the SSP clock rate provided by the master must not exceed 1/12 of the
    //SSP peripheral clock (which is set to PCLK/1 above), therefore to cater for LPC1768
    //(100MHz) max Master SCK is limited to 8.33MHz
    spiDevice->configureDevice(SSP_MODE_SLAVE, SSP_BITS_8, SSP_CLOCK_MODE1, SystemCoreClock/2, true);
}
    
