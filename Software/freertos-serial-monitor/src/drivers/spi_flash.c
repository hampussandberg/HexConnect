/**
 ******************************************************************************
 * @file	spi_flash.c
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-09-07
 * @brief	Code inspired by Spark Core common-lib (https://github.com/spark/core-common-lib)
 ******************************************************************************
	Copyright (c) 2014 Hampus Sandberg.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation, either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"

/* Private defines -----------------------------------------------------------*/
#define FLASH_SPI				(SPI2)
#define FLASH_SPI_CLK_ENABLE()	(__SPI2_CLK_ENABLE())
#define FLASH_PORT				(GPIOB)
#define FLASH_GPIO_CLK_ENABLE()	(__GPIOB_CLK_ENABLE())
#define FLASH_CS_PIN			(GPIO_PIN_12)
#define FLASH_SCK_PIN			(GPIO_PIN_13)
#define FLASH_MISO_PIN			(GPIO_PIN_14)
#define FLASH_MOSI_PIN			(GPIO_PIN_15)

/* SPI FLASH Commands */
#define SPI_FLASH_CMD_RDSR			(0x05)		/* Read Status Register */
#define SPI_FLASH_CMD_WRSR			(0x01)		/* Write Status Register */
#define SPI_FLASH_CMD_EWSR			(0x50)		/* Write Enable Status */

#define SPI_FLASH_CMD_WRDI			(0x04)		/* Write Disable */
#define SPI_FLASH_CMD_WREN			(0x06)		/* Write Enable */
#define SPI_FLASH_CMD_READ			(0x03)		/* Read Data Bytes */
#define SPI_FLASH_CMD_WRITE 		(0x02)		/* Byte Program / Page program (S25FL127SABMFI101) */
#define SPI_FLASH_CMD_AAIP			(0xAD)		/* Auto Address Increment word program (SST25VF016B) */
#define SPI_FLASH_CMD_EBSY			(0x70)		/* Enable SO RY/BY# Status (SST25VF016B) */
#define SPI_FLASH_CMD_DBSY			(0x80)		/* Disable SO RY/BY# Status (SST25VF016B) */
#define SPI_FLASH_CMD_4KB_SE		(0x20)		/* 4 KB Sector Erase instruction */
#define SPI_FLASH_CMD_64KB_SE		(0xD8)		/* 64 KB Sector Erase instruction */
#define SPI_FLASH_CMD_BE            (0xC7)		/* Bulk Chip Erase instruction */
#define SPI_FLASH_CMD_RDID			(0x9F)		/* JEDEC ID Read */

#define SPI_FLASH_DUMMY_BYTE		(0xFF)
#define SPI_FLASH_WIP_FLAG			(0x01)		/* Write In Progress (WIP) flag */
#define SPI_FLASH_S25FL127SABMFI101_ID	(0x012018)	/* Device ID for the S25FL127SABMFI101 */
#define SPI_FLASH_SST25VF016B_ID		(0xBF2541)	/* Device ID for the SST25VF016B */

/*
 * The memory is split into sectors where the first 64KB are split into 16x4KB sectors and
 * the rest of the memory are 64KB sectors. This is important when doing a sector erase
 * as you can erase the 4KB sectors separately.
 */
#define SPI_FLASH_64KB_SECTOR_START_ADDRESS		(0x010000)

/* Private typedefs ----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static SPI_HandleTypeDef SPI_Handle = {
		.Instance 				= FLASH_SPI,
		.Init.Mode 				= SPI_MODE_MASTER,
		.Init.Direction 		= SPI_DIRECTION_2LINES,
		.Init.DataSize 			= SPI_DATASIZE_8BIT,
		.Init.CLKPolarity 		= SPI_POLARITY_LOW,
		.Init.CLKPhase 			= SPI_PHASE_1EDGE,
		.Init.NSS 				= SPI_NSS_SOFT,
		.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2,
		.Init.FirstBit 			= SPI_FIRSTBIT_MSB,
		.Init.TIMode			= SPI_TIMODE_DISABLED,
		.Init.CRCCalculation	= SPI_CRCCALCULATION_DISABLED,
		.Init.CRCPolynomial 	= 1,
};

static DMA_HandleTypeDef DMA_HandleTx = {
		.Instance					= DMA1_Stream4,
		.Init.Channel 				= DMA_CHANNEL_0,
		.Init.Direction 			= DMA_MEMORY_TO_PERIPH,
		.Init.PeriphInc 			= DMA_PINC_DISABLE,
		.Init.MemInc 				= DMA_MINC_ENABLE,
		.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE,
		.Init.MemDataAlignment 		= DMA_MDATAALIGN_BYTE,
		.Init.Mode 					= DMA_NORMAL,
		.Init.Priority				= DMA_PRIORITY_LOW,
		.Init.FIFOMode 				= DMA_FIFOMODE_DISABLE,
		.Init.FIFOThreshold      	= DMA_FIFO_THRESHOLD_FULL,
		.Init.MemBurst				= DMA_MBURST_INC4,
		.Init.PeriphBurst			= DMA_PBURST_INC4,
};

static DMA_HandleTypeDef DMA_HandleRx = {
		.Instance					= DMA1_Stream3,
		.Init.Channel 				= DMA_CHANNEL_0,
		.Init.Direction 			= DMA_PERIPH_TO_MEMORY,
		.Init.PeriphInc 			= DMA_PINC_DISABLE,
		.Init.MemInc 				= DMA_MINC_ENABLE,
		.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_BYTE,
		.Init.MemDataAlignment 		= DMA_MDATAALIGN_BYTE,
		.Init.Mode 					= DMA_NORMAL,
		.Init.Priority				= DMA_PRIORITY_HIGH,
		.Init.FIFOMode 				= DMA_FIFOMODE_DISABLE,
		.Init.FIFOThreshold      	= DMA_FIFO_THRESHOLD_FULL,
		.Init.MemBurst 				= DMA_MBURST_SINGLE,
		.Init.MemBurst				= DMA_MBURST_INC4,
		.Init.PeriphBurst			= DMA_PBURST_INC4,
};

static uint32_t prvDeviceId = 0;
static SemaphoreHandle_t xSemaphore;
static bool prvInitialized = false;
static bool prvDmaTransferIsDone = true;

/* Private function prototypes -----------------------------------------------*/
static inline void prvSPI_FLASH_CS_LOW();
static inline void prvSPI_FLASH_CS_HIGH();
static void prvSPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte);
static void prvSPI_FLASH_WriteBytes(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite);
static void prvSPI_FLASH_WriteDisable();
static void prvSPI_FLASH_WriteEnable();
static uint8_t prvSPI_FLASH_SendReceiveByte(uint8_t Byte);
static void prvSPI_FLASH_WaitForWriteEnd();

/* Functions -----------------------------------------------------------------*/
/**
 * @brief	Initializes the SPI FLASH
 * @param	None
 * @retval	None
 */
ErrorStatus SPI_FLASH_Init()
{
	/* Make sure we only initialize it once */
	if (!prvInitialized)
	{
		/* Mutex semaphore for mutual exclusion to the SPI Flash device */
		xSemaphore = xSemaphoreCreateMutex();

		/* Init GPIO */
		FLASH_GPIO_CLK_ENABLE();
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.Pin  		= FLASH_SCK_PIN | FLASH_MISO_PIN | FLASH_MOSI_PIN;
		GPIO_InitStructure.Mode  		= GPIO_MODE_AF_PP;
		GPIO_InitStructure.Alternate	= GPIO_AF5_SPI2;
		GPIO_InitStructure.Pull			= GPIO_NOPULL;
		GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
		HAL_GPIO_Init(FLASH_PORT, &GPIO_InitStructure);

		GPIO_InitStructure.Pin  		= FLASH_CS_PIN;
		GPIO_InitStructure.Mode  		= GPIO_MODE_OUTPUT_PP;
		HAL_GPIO_Init(FLASH_PORT, &GPIO_InitStructure);
		/* Deselect the FLASH */
		prvSPI_FLASH_CS_HIGH();


		/* DMA Init */
		__DMA1_CLK_ENABLE();
		HAL_DMA_Init(&DMA_HandleTx);
		/* Associate the initialized DMA handle to the the SPI handle */
		__HAL_LINKDMA(&SPI_Handle, hdmatx, DMA_HandleTx);

		HAL_DMA_Init(&DMA_HandleRx);
		/* Associate the initialized DMA handle to the the SPI handle */
		__HAL_LINKDMA(&SPI_Handle, hdmarx, DMA_HandleRx);

		/* NVIC configuration for DMA transfer complete interrupt (SPI2_TX) */
		HAL_NVIC_SetPriority(DMA1_Stream4_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream4_IRQn);

		/* NVIC configuration for DMA transfer complete interrupt (SPI2_RX) */
		HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY-1, 0);
		HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

		/* Init SPI */
		FLASH_SPI_CLK_ENABLE();
		HAL_SPI_Init(&SPI_Handle);

		/* Read FLASH identification */
		prvDeviceId = SPI_FLASH_ReadID();

		if (prvDeviceId == SPI_FLASH_S25FL127SABMFI101_ID)
		{
			/* Select the FLASH */
			prvSPI_FLASH_CS_LOW();
			/* Send "Write Enable Status" instruction */
			prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_EWSR);
			/* Deselect the FLASH */
			prvSPI_FLASH_CS_HIGH();

			/* Select the FLASH */
			prvSPI_FLASH_CS_LOW();
			/* Send "Write Status Register" instruction and set all bits to 0 */
			prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRSR);
			prvSPI_FLASH_SendReceiveByte(0);
			/* Deselect the FLASH */
			prvSPI_FLASH_CS_HIGH();

			prvInitialized = true;

			return SUCCESS;
		}
	}
	return ERROR;
}

/**
  * @brief  Reads FLASH identification
  * @param  None
  * @retval FLASH identification
  */
uint32_t SPI_FLASH_ReadID()
{
	uint8_t byte[3];

	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
	  /* Select the FLASH */
	  prvSPI_FLASH_CS_LOW();

	  /* Send "JEDEC ID Read" instruction */
	  prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_RDID);

	  /* Read three bytes from the FLASH */
	  byte[0] = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
	  byte[1] = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
	  byte[2] = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);

	  /* Deselect the FLASH */
	  prvSPI_FLASH_CS_HIGH();

	  /* Give back the semaphore */
	  xSemaphoreGive(xSemaphore);
	}
	return (byte[0] << 16) | (byte[1] << 8) | byte[2];
}

/**
  * @brief  Write a buffer to the FLASH
  * @note   Addresses to be written must be in the erased state
  * @param	pBuff: pointer to the buffer with data to write
  * @param  WriteAddress: start of FLASH's internal address to write to
  * @param  NumByteToWrite: number of bytes to write to the FLASH
  * @retval None
  */
void SPI_FLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		prvSPI_FLASH_WriteBytes(pBuffer, WriteAddress, NumByteToWrite);

		/* TODO: Something is wrong with this functions. Data are overwritten sometime */
//		uint32_t evenBytes;
//		/* If write starts at an odd address, need to use single byte write
//		* to write the first address. */
//		if ((WriteAddress & 0x1) == 0x1)
//		{
//			prvSPI_FLASH_WriteByte(WriteAddress, *pBuffer++);
//			WriteAddress++;
//			NumByteToWrite--;
//		}
//
//		/* Write bulk of bytes using auto increment write, with restriction
//		* that address must always be even and two bytes are written at a time. */
//		evenBytes = NumByteToWrite & ~0x1;
//		if (evenBytes)
//		{
//			prvSPI_FLASH_WriteBytes(pBuffer, WriteAddress, evenBytes);
//			NumByteToWrite -= evenBytes;
//		}
//
//		/* If number of bytes to write is odd, need to use a single byte write
//		* to write the last address. */
//		if (NumByteToWrite)
//		{
//			pBuffer += evenBytes;
//			WriteAddress += evenBytes;
//			prvSPI_FLASH_WriteByte(WriteAddress, *pBuffer++);
//		}

		/* Give back the semaphore */
		xSemaphoreGive(xSemaphore);
	}
}

/**
  * @brief  Write one byte to the FLASH.
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: FLASH's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void SPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		prvSPI_FLASH_WriteByte(WriteAddress, Byte);

		/* Give back the semaphore */
		xSemaphoreGive(xSemaphore);
	}
}

/**
  * @brief  Write one byte to the FLASH. Can be called from an ISR
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: FLASH's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
void SPI_FLASH_WriteByteFromISR(uint32_t WriteAddress, uint8_t Byte)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTakeFromISR(xSemaphore, NULL) == pdTRUE)
	{
		prvSPI_FLASH_WriteByte(WriteAddress, Byte);

		/* Give back the semaphore */
		xSemaphoreGiveFromISR(xSemaphore, NULL);
	}
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuff: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddress: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @retval None
  * @time	~165.6 us when Size = NumByteToRead
  */
void SPI_FLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddress, uint32_t NumByteToRead)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (NumByteToRead != 0 && xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		/* Select the FLASH */
		prvSPI_FLASH_CS_LOW();

		/* Send "Read from Memory " instruction */
		prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_READ);

		/* Send ReadAddr high nibble address byte to read from */
		prvSPI_FLASH_SendReceiveByte((ReadAddress & 0xFF0000) >> 16);
		/* Send ReadAddr medium nibble address byte to read from */
		prvSPI_FLASH_SendReceiveByte((ReadAddress& 0xFF00) >> 8);
		/* Send ReadAddr low nibble address byte to read from */
		prvSPI_FLASH_SendReceiveByte(ReadAddress & 0xFF);

		while (NumByteToRead) /* while there is data to be read */
		{
			/* Read a byte from the FLASH and point to the next location */
			*pBuffer++ = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
			/* Decrement NumByteToRead */
			NumByteToRead--;
		}

		/* Deselect the FLASH */
		prvSPI_FLASH_CS_HIGH();

		/* Give back the semaphore */
		xSemaphoreGive(xSemaphore);
	}
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuff: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddress: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @param	BlockTime: ms to wait for buffer to be read
  * @retval None
  * @time	~51.6 us when Size = NumByteToRead
  */
ErrorStatus SPI_FLASH_ReadBufferDMA(uint8_t* pBuffer, uint32_t ReadAddress, uint32_t NumByteToRead, TickType_t BlockTime)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (NumByteToRead != 0 && xSemaphoreTake(xSemaphore, BlockTime) == pdTRUE)
	{
		prvDmaTransferIsDone = false;

		/* Select the FLASH */
		prvSPI_FLASH_CS_LOW();

		/* Send "Read from Memory " instruction */
		prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_READ);

		/* Send ReadAddr high nibble address byte to read from */
		prvSPI_FLASH_SendReceiveByte((ReadAddress & 0xFF0000) >> 16);
		/* Send ReadAddr medium nibble address byte to read from */
		prvSPI_FLASH_SendReceiveByte((ReadAddress& 0xFF00) >> 8);
		/* Send ReadAddr low nibble address byte to read from */
		prvSPI_FLASH_SendReceiveByte(ReadAddress & 0xFF);

		HAL_SPI_TransmitReceive_DMA(&SPI_Handle, pBuffer, pBuffer, NumByteToRead);

		/* Wait for the DMA transfer to be done */
		/* TODO: Non blocking */
		while (!prvDmaTransferIsDone);

		/* Deselect the FLASH */
		prvSPI_FLASH_CS_HIGH();

		/* Give back the semaphore */
		xSemaphoreGive(xSemaphore);

		return SUCCESS;
	}
	else
		return ERROR;
}

/**
  * @brief  Erases the specified FLASH sector
  * @param  SectorAddr: address of the sector to erase
  * @retval None
  */
ErrorStatus SPI_FLASH_EraseSector(uint32_t SectorAddress)
{
	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTake(xSemaphore, 10000) == pdTRUE)
	{
		/* Enable the write access to the FLASH */
		prvSPI_FLASH_WriteEnable();

		/* Sector Erase */
		/* Select the FLASH: Chip Select low */
		prvSPI_FLASH_CS_LOW();

		/* Send Sector Erase instruction depending on which sector to erase */
		if (SectorAddress >= SPI_FLASH_64KB_SECTOR_START_ADDRESS)
		  prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_64KB_SE);
		else
		  prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_4KB_SE);

		/* Send SectorAddr high nibble address byte */
		prvSPI_FLASH_SendReceiveByte((SectorAddress & 0xFF0000) >> 16);
		/* Send SectorAddr medium nibble address byte */
		prvSPI_FLASH_SendReceiveByte((SectorAddress & 0xFF00) >> 8);
		/* Send SectorAddr low nibble address byte */
		prvSPI_FLASH_SendReceiveByte(SectorAddress & 0xFF);
		/* Deselect the FLASH: Chip Select high */
		prvSPI_FLASH_CS_HIGH();

		/* Wait till the end of Flash writing */
		prvSPI_FLASH_WaitForWriteEnd();

		/* Give back the semaphore */
		xSemaphoreGive(xSemaphore);

		return SUCCESS;
	}
	else
		return ERROR;
}

/**
  * @brief  Erases the entire FLASH
  * @param  None
  * @retval None
  */
void SPI_FLASH_EraseBulk()
{
	/* Try to take the semaphore in case some other process is using the device */
	if (xSemaphoreTake(xSemaphore, 100) == pdTRUE)
	{
		/* Enable the write access to the FLASH */
		prvSPI_FLASH_WriteEnable();

		/* Bulk Erase */
		/* Select the FLASH */
		prvSPI_FLASH_CS_LOW();
		/* Send Bulk Erase instruction  */
		prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_BE);
		/* Deselect the FLASH */
		prvSPI_FLASH_CS_HIGH();

		/* Wait till the end of Flash writing */
		prvSPI_FLASH_WaitForWriteEnd();

		/* Give back the semaphore */
		xSemaphoreGive(xSemaphore);
	}
}

/**
  * @brief  Return the status of if the SPI FLASH is intialized or not
  * @param  None
  * @retval true: The device is initialized
  * @retval false: The device is not initialized
  */
bool SPI_FLASH_Initialized()
{
	return prvInitialized;
}

/* Private functions .--------------------------------------------------------*/
/**
 * @brief	Pull the CS pin LOW
 * @param	None
 * @retval	None
 */
static inline void prvSPI_FLASH_CS_LOW()
{
	HAL_GPIO_WritePin(FLASH_PORT, FLASH_CS_PIN, GPIO_PIN_RESET);
}

/**
 * @brief	Pull the CS pin HIGH
 * @param	None
 * @retval	None
 */
static inline void prvSPI_FLASH_CS_HIGH()
{
	HAL_GPIO_WritePin(FLASH_PORT, FLASH_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  Write one byte to the FLASH.
  * @note   Addresses to be written must be in the erased state
  * @param  WriteAddress: FLASH's internal address to write to.
  * @param  Byte: the data to be written.
  * @retval None
  */
static void prvSPI_FLASH_WriteByte(uint32_t WriteAddress, uint8_t Byte)
{
	/* Enable the write access to the FLASH */
	prvSPI_FLASH_WriteEnable();

	/* Select the FLASH */
	prvSPI_FLASH_CS_LOW();
	/* Send "Byte Program" instruction */
	prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRITE);
	/* Send WriteAddress high, medium and low nibble address byte to write to */
	prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF0000) >> 16);
	prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF00) >> 8);
	prvSPI_FLASH_SendReceiveByte(WriteAddress & 0xFF);
	/* Send the byte */
	prvSPI_FLASH_SendReceiveByte(Byte);
	/* Deselect the FLASH */
	prvSPI_FLASH_CS_HIGH();
	/* Wait till the end of Flash writing */
	prvSPI_FLASH_WaitForWriteEnd();
}

/**
  * @brief  Writes more than one byte to the FLASH.
  * @note   The address must be even and the number of bytes must be a multiple
  *         of two.
  * @note   Addresses to be written must be in the erased state
  * @param  pBuffer: pointer to the buffer containing the data to be written
  *         to the FLASH.
  * @param  WriteAddress: FLASH's internal address to write to, must be even.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be even.
  * @retval None
  */
static void prvSPI_FLASH_WriteBytes(uint8_t* pBuffer, uint32_t WriteAddress, uint32_t NumByteToWrite)
{
	/* Enable the write access to the FLASH */
	prvSPI_FLASH_WriteEnable();

	/* Select the FLASH */
	prvSPI_FLASH_CS_LOW();
	/* The SST25VF016B has a special command for continuous programming */
	if (prvDeviceId == SPI_FLASH_SST25VF016B_ID)
	{
		/* Send "Auto Address Increment Word-Program" instruction */
		prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_AAIP);
		/* Send WriteAddress high, medium and low nibble address byte to write to */
		prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF0000) >> 16);
		prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF00) >> 8);
		prvSPI_FLASH_SendReceiveByte(WriteAddress & 0xFF);
		/* Send the first two bytes */
		prvSPI_FLASH_SendReceiveByte(*pBuffer++);
		prvSPI_FLASH_SendReceiveByte(*pBuffer++);
		/* Update NumByteToWrite */
		NumByteToWrite -= 2;
		/* Deselect the FLASH */
		prvSPI_FLASH_CS_HIGH();
		/* Wait till the end of Flash writing */
		prvSPI_FLASH_WaitForWriteEnd();

		/* While there is data to be written to the FLASH */
		while (NumByteToWrite)
		{
			/* Select the FLASH */
			prvSPI_FLASH_CS_LOW();
			/* Send "Auto Address Increment Word-Program" instruction */
			prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_AAIP);
			/* Send the next two bytes and point on the byte after that */
			prvSPI_FLASH_SendReceiveByte(*pBuffer++);
			prvSPI_FLASH_SendReceiveByte(*pBuffer++);
			/* Update NumByteToWrite */
			NumByteToWrite -= 2;
		}
	}
	else
	{
		/* Send write command */
		prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRITE);
		/* Send WriteAddress high, medium and low nibble address byte to write to */
		prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF0000) >> 16);
		prvSPI_FLASH_SendReceiveByte((WriteAddress & 0xFF00) >> 8);
		prvSPI_FLASH_SendReceiveByte(WriteAddress & 0xFF);

		/* While there is data to be written to the FLASH */
		while (NumByteToWrite)
		{
			/* Send one byte */
			prvSPI_FLASH_SendReceiveByte(*pBuffer++);
			/* Update NumByteToWrite */
			NumByteToWrite--;
		}
	}

	/* Deselect the FLASH */
	prvSPI_FLASH_CS_HIGH();
	/* Wait till the end of Flash writing */
	prvSPI_FLASH_WaitForWriteEnd();

	/* Disable the write access to the FLASH */
	prvSPI_FLASH_WriteDisable();
}

/**
  * @brief  Disables the write access to the FLASH
  * @param  None
  * @retval None
  */
static void prvSPI_FLASH_WriteDisable()
{
	/* Select the FLASH */
	prvSPI_FLASH_CS_LOW();

	/* Send "Write Disable" instruction */
	prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WRDI);

	/* Deselect the FLASH */
	prvSPI_FLASH_CS_HIGH();
}

/**
  * @brief  Enables the write access to the FLASH
  * @param  None
  * @retval None
  */
static void prvSPI_FLASH_WriteEnable()
{
	/* Select the FLASH */
	prvSPI_FLASH_CS_LOW();

	/* Send "Write Enable" instruction */
	prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_WREN);

	/* Deselect the FLASH */
	prvSPI_FLASH_CS_HIGH();
}

/**
  * @brief  Send one byte to the SPI FLASH and return the byte received back
  * @param  Byte: The byte to send
  * @retval The byte received from the SPI FLASH
  */
static uint8_t prvSPI_FLASH_SendReceiveByte(uint8_t Byte)
{
	/* TODO: Do this RAW instead of using HAL??? A lot of overhead in HAL */
	uint8_t rxByte;
	HAL_SPI_TransmitReceive(&SPI_Handle, &Byte, &rxByte, 1, 10);	/* TODO: Check timeout */
	return rxByte;
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write operation has completed.
  * @param  None
  * @retval None
  */
static void prvSPI_FLASH_WaitForWriteEnd()
{
	uint8_t flashStatus = 0;

	/* Select the FLASH */
	prvSPI_FLASH_CS_LOW();

	/* Send "Read Status Register" instruction */
	prvSPI_FLASH_SendReceiveByte(SPI_FLASH_CMD_RDSR);

	/* Loop as long as the memory is busy with a write cycle */
	do
	{
		/* TODO: Handle blocking */
		/* Send a dummy byte to generate the clock needed by the FLASH
		and put the value of the status register in FLASH_Status variable */
		flashStatus = prvSPI_FLASH_SendReceiveByte(SPI_FLASH_DUMMY_BYTE);
	} while ((flashStatus & SPI_FLASH_WIP_FLAG) == SET); /* Write in progress */

	/* Deselect the FLASH */
	prvSPI_FLASH_CS_HIGH();
}

/* Interrupt Handlers --------------------------------------------------------*/
/**
  * @brief  This function handles DMA Tx interrupt request.
  * @param  None
  * @retval None
  */
void DMA1_Stream4_IRQHandler(void)
{
	HAL_DMA_IRQHandler(SPI_Handle.hdmatx);
}

/**
  * @brief  This function handles DMA Rx interrupt request.
  * @param  None
  * @retval None
  */
void DMA1_Stream3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(SPI_Handle.hdmarx);
}

/**
  * @brief  TxRx Transfer completed callback.
  * @param  hspi: SPI handle.
  * @note   This example shows a simple way to report end of DMA TxRx transfer, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	prvDmaTransferIsDone = true;
}

/**
  * @brief  SPI error callbacks.
  * @param  hspi: SPI handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
 void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	 prvDmaTransferIsDone = true;
	 /* TODO: Manage errors */
}
