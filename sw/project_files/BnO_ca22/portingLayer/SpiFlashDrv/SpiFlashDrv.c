#include "product.config"

#ifdef HAS_SPI_FLASH

#include "commontypes.h"
#include "stm32f0xx.h"
#include "trace.h"
#include "bsp.h"
#include "attachedDevices.h"
#include "GPIODrv.h"
#include "HWspiDrv.h"

#include "SpiFlashDrv_priv.h"
#include "SpiFlashDrv.h"

#define FLASH_DEBUG_ENABLE
#if defined(FLASH_DEBUG_ENABLE)
#define FLASH_printf    printf
#else
#define FLASH_printf(...)
#endif

#define SPI_FLASH_CS_DISABLE    do { \
                                    GpioDrv_SetBit(&spiflash_gpio, GPIO_OUT_SPI_FLASH_CS); \
                                }while(0)
#define SPI_FLASH_CS_ENABLE     do { \
                                    GpioDrv_ClearBit(&spiflash_gpio, GPIO_OUT_SPI_FLASH_CS); \
                                }while(0)

static cGpioDrv spiflash_gpio;
static cHWspiDrv spiflash_drv;
static cHWspiDrv *p_spiflash_drv;

static bool SpiFlash_WaitWriteEnd()
{
    bool success = FALSE;
    uint8_t cmd[4]={FLASH_CMD_RDSR};
    uint32_t retry_cnt=1000000;

    SPI_FLASH_CS_ENABLE;

    HWspiDrv_WriteRead(p_spiflash_drv, cmd, NULL, 1, 3);

    while( retry_cnt )
    {
        retry_cnt --;
        HWspiDrv_WriteRead(p_spiflash_drv, NULL, cmd, 1, 3);
        if( (cmd[0] & FLASH_FLAG_WIP) == 0 )
        {
            break;
        }
#ifdef HAS_IWDG
        else
        {    // since the retry_cnt is a big number, should feed the watchdog here
            IWDG_ReloadCounter();
        }
#endif
    }

    if( retry_cnt )
        success = TRUE;
    else
        success = FALSE;
    
    SPI_FLASH_CS_DISABLE;

    return success;
}

static void SpiFlash_WriteEnable()
{
    uint8_t cmd[4] = {FLASH_CMD_WREN};

    SPI_FLASH_CS_ENABLE;

    HWspiDrv_WriteRead(p_spiflash_drv, cmd, NULL, 1, 3);

    SPI_FLASH_CS_DISABLE;
}

/**
 * Construct the driver instance.
 * @return : none
 */
void SpiFlashDrv_Ctor(void)
{
    tSpiDevice *p_device;

    p_spiflash_drv = &spiflash_drv;

    ASSERT( ! p_spiflash_drv->isReady );

    spiflash_gpio.gpioConfig = (tGPIODevice *) getDevicebyIdAndType(INT_FLASH_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(spiflash_gpio.gpioConfig);
    GpioDrv_Ctor(&spiflash_gpio, spiflash_gpio.gpioConfig);

    p_device = (tSpiDevice *) getDevicebyIdAndType(INT_FLASH_DEV_ID, SPI_DEV_TYPE, NULL);
    ASSERT(p_device);

    HWspiDrv_Ctor(p_spiflash_drv, p_device);

    p_spiflash_drv->isReady = TRUE;
    p_spiflash_drv->isBusy = FALSE;

    // read a dummy byte
    uint8_t tmp;
    SpiFlashDrv_ReadBytes(0, 1, &tmp);
}

/**
 * Exit & clean up the driver.
 */
void SpiFlashDrv_Xtor(void)
{
    GpioDrv_Xtor(&spiflash_gpio);
    HWspiDrv_Xtor(p_spiflash_drv);
    p_spiflash_drv->isReady = FALSE;
    p_spiflash_drv->isBusy = TRUE;
    p_spiflash_drv = NULL;
}

/**
 * get the manufacturer & device ID
 * @return : combine the ID
 */
uint32_t SpiFlashDrv_GetDeviceID()
{
    uint8_t cmd[4] = {FLASH_CMD_REMS, 0x00, 0x00, 0x00};
    uint32_t flash_id;

    SPI_FLASH_CS_ENABLE;

    HWspiDrv_WriteRead(p_spiflash_drv, cmd, NULL, 4, 5);
    HWspiDrv_WriteRead(p_spiflash_drv, NULL, cmd, 2, 5);

    flash_id = ((uint32_t)cmd[0] << 8) | cmd[1];
    
    SPI_FLASH_CS_DISABLE;

    FLASH_printf("\n\rFlash device ID : 0x%x.", flash_id);

    return flash_id;
}

/**
 * erase one sector
 * @return : TRUE if success, FALSE if failure
 */
bool SpiFlashDrv_EraseSector(uint32_t addr)
{
    uint8_t cmd[4] = {FLASH_CMD_SE, 0x00, 0x00, 0x00};

    cmd[1] = (addr >> 16) & 0xff;
    cmd[2] = (addr >> 8) & 0xff;
    cmd[3] = addr & 0xff;
    
    SpiFlash_WriteEnable();

    SPI_FLASH_CS_ENABLE;
    HWspiDrv_WriteRead(p_spiflash_drv, cmd, NULL, 4, 5);
    SPI_FLASH_CS_DISABLE;

    return SpiFlash_WaitWriteEnd();
}

/**
 * read n bytes from the flash
 * @return : NONE
 */
void SpiFlashDrv_ReadBytes(uint32_t addr, uint32_t len, uint8_t *p_buf)
{
    uint8_t cmd[4] = {FLASH_CMD_READ, 0x00, 0x00, 0x00};

    cmd[1] = (addr >> 16) & 0xff;
    cmd[2] = (addr >> 8) & 0xff;
    cmd[3] = addr & 0xff;

    SPI_FLASH_CS_ENABLE;
    // send read command
    HWspiDrv_WriteRead(p_spiflash_drv, cmd, NULL, 4, 5);
    // read data
    HWspiDrv_WriteRead(p_spiflash_drv, NULL, p_buf, len, len);
    SPI_FLASH_CS_DISABLE;
}

/**
 * write n bytes to the flash
 * @NOTE : do NOT cross the page
 * @return : NONE
 */
void SpiFlashDrv_WriteBytes(uint32_t addr, uint32_t len, uint8_t *p_buf)
{
    uint8_t cmd[4] = {FLASH_CMD_PP, 0x00, 0x00, 0x00};

    if( ((addr & 0xff) + len) > 256 )
    {
        ALWAYS_printf("\n\rERROR : Data exceed page boundary, addr=0x%x.", addr);
        ASSERT(0);
    }

    SpiFlash_WriteEnable();

    cmd[1] = (addr >> 16) & 0xff;
    cmd[2] = (addr >> 8) & 0xff;
    cmd[3] = addr & 0xff;
    SPI_FLASH_CS_ENABLE;
    // send write command
    HWspiDrv_WriteRead(p_spiflash_drv, cmd, NULL, 4, 5);
    // write data
    HWspiDrv_WriteRead(p_spiflash_drv, p_buf, NULL, len, len);
    SPI_FLASH_CS_DISABLE;

    SpiFlash_WaitWriteEnd();
}


#endif  // HAS_SPI_FLASH

