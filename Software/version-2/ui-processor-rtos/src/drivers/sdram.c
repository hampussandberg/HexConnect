/**
 *******************************************************************************
 * @file    sdram.c
 * @author  Hampus Sandberg
 * @version 0.1
 * @date    2015-08-23
 * @brief   Parameters set for a AS4C16M16S-7TCN 256Mbit SDRAM
 *******************************************************************************
  Copyright (c) 2015 Hampus Sandberg.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 */

/** Includes -----------------------------------------------------------------*/
#include "sdram.h"

/** Private defines ----------------------------------------------------------*/
//#define SDRAM_MEMORY_WIDTH   FMC_SDRAM_MEM_BUS_WIDTH_8
#define SDRAM_MEMORY_WIDTH    FMC_SDRAM_MEM_BUS_WIDTH_16

// #define SDRAM_CAS_LATENCY   FMC_SDRAM_CAS_LATENCY_2
#define SDRAM_CAS_LATENCY    FMC_SDRAM_CAS_LATENCY_3

#define SDCLOCK_PERIOD    FMC_SDRAM_CLOCK_PERIOD_2
//#define SDCLOCK_PERIOD    FMC_SDRAM_CLOCK_PERIOD_3

#define SDRAM_READBURST    FMC_SDRAM_RBURST_DISABLE
//#define SDRAM_READBURST    FMC_SDRAM_RBURST_ENABLE

/* FMC SDRAM Mode definition register defines */
#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

/** Private typedefs ---------------------------------------------------------*/
/** Private variables --------------------------------------------------------*/

/** Private function prototypes ----------------------------------------------*/
static void prvGPIOConfig();

/** Functions ----------------------------------------------------------------*/
/**
 * @brief  Initializes the SDRAM
 * @param  None
 * @retval  None
 */
void SDRAM_Init()
{
  SDRAM_HandleTypeDef SDRAMHandle;
  FMC_SDRAM_TimingTypeDef FMC_SDRAMTiming;

  /* GPIO configuration for FMC SDRAM bank */
  prvGPIOConfig();

  /* Enable FMC clock */
  __HAL_RCC_FMC_CLK_ENABLE();

  /** FMC Configuration ------------------------------------------------------*/
  /* FMC SDRAM Bank configuration */
  /* Timing configuration for 90 Mhz of SD clock frequency (180Mhz/2) */
  FMC_SDRAMTiming.LoadToActiveDelay    = 2; /* TMRD: 2 Clock cycles */
  FMC_SDRAMTiming.ExitSelfRefreshDelay = 7; /* TXSR: min=64.5ns => 7x11.11ns */
  FMC_SDRAMTiming.SelfRefreshTime      = 5; /* TRAS: min=49ns => 5x11.11ns */
  FMC_SDRAMTiming.RowCycleDelay        = 7; /* TRC:  min=63ns => 7x11.11ns */
  FMC_SDRAMTiming.WriteRecoveryTime    = 2; /* TWR:  min=14ns => 2x11.11ns */
  FMC_SDRAMTiming.RPDelay              = 2; /* TRP:  min=21ns => 2x11.11ns */
  FMC_SDRAMTiming.RCDDelay             = 2; /* TRCD: min=21ns => 2x11.11ns */

  /* FMC SDRAM control configuration */
  SDRAMHandle.Instance = FMC_SDRAM_DEVICE;
  SDRAMHandle.Init.SDBank             = FMC_SDRAM_BANK2;              /* Bank 2 */
  SDRAMHandle.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;  /* Row addressing: [8:0] */
  SDRAMHandle.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_13;    /* Column addressing: [12:0] */
  SDRAMHandle.Init.MemoryDataWidth    = SDRAM_MEMORY_WIDTH;
  SDRAMHandle.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  SDRAMHandle.Init.CASLatency         = SDRAM_CAS_LATENCY;
  SDRAMHandle.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  SDRAMHandle.Init.SDClockPeriod      = SDCLOCK_PERIOD;
  SDRAMHandle.Init.ReadBurst          = SDRAM_READBURST;
  SDRAMHandle.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_1;  /* TODO: Check if 0 is better */

  /* FMC SDRAM bank initialization */
  HAL_SDRAM_Init(&SDRAMHandle, &FMC_SDRAMTiming);



  /* FMC SDRAM device initialization sequence */
  FMC_SDRAM_CommandTypeDef Command;
  uint32_t tmpr = 0;

  /** Step 3 -----------------------------------------------------------------*/
  /* Configure a clock configuration enable command */
  Command.CommandMode             = FMC_SDRAM_CMD_CLK_ENABLE;
  Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber       = 1;
  Command.ModeRegisterDefinition  = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&SDRAMHandle, &Command, 0x1000);

  /** Step 4 -----------------------------------------------------------------*/
  /* Insert 100 ms delay */
  HAL_Delay(100);

  /** Step 5 -----------------------------------------------------------------*/
  /* Configure a PALL (precharge all) command */
  Command.CommandMode             = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber       = 1;
  Command.ModeRegisterDefinition  = 0;

  /* Send the command */
  HAL_SDRAM_SendCommand(&SDRAMHandle, &Command, 0x1000);

  /** Step 6 -----------------------------------------------------------------*/
  /* Configure a Auto-Refresh command */
  Command.CommandMode             = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber       = 4;
  Command.ModeRegisterDefinition  = 0;

  /* Send the  first command */
  HAL_SDRAM_SendCommand(&SDRAMHandle, &Command, 0x1000);

  /** Step 7 -----------------------------------------------------------------*/
  /* Program the external memory mode register */
  tmpr = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_2           |
                   SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL    |
                   SDRAM_MODEREG_CAS_LATENCY_3            |
                   SDRAM_MODEREG_OPERATING_MODE_STANDARD  |
                   SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

  /* Configure a load Mode register command*/
  Command.CommandMode             = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget           = FMC_SDRAM_CMD_TARGET_BANK2;
  Command.AutoRefreshNumber       = 1;
  Command.ModeRegisterDefinition  = tmpr;

  /* Send the command */
  HAL_SDRAM_SendCommand(&SDRAMHandle, &Command, 0x1000);

  /** Step 8 -----------------------------------------------------------------*/
  /* Set the refresh rate counter */
  /* 8192 refresh cycles/64ms = 7.8125us between each refresh */
  /* 7.8125 us x 90MHz ≈ 703 */
  /* Refresh more often than this, let's say 20 less -> 683 */
  /* TODO: Is it the SDRAM clock (90MHz) or the main clock (180MHz)? */
  HAL_SDRAM_ProgramRefreshRate(&SDRAMHandle, 683);

//  /* Do some checks */
//  volatile bool status = false;
//  status = SDRAM_Test8bit(SDRAM_END);
//  status = SDRAM_Test16bit(SDRAM_END);
//  status = SDRAM_Test32bit(SDRAM_END);
}

/**
  * @brief
  * @param  EndAddress: The address to stop at
  * @retval None
  */
void SDRAM_EraseAll(uint32_t EndAddress)
{
  /* Erase SDRAM memory */
  for (uint32_t address = SDRAM_BANK_ADDR; address < EndAddress; address += 2)
  {
    *(__IO uint16_t*) (address) = (uint16_t)0x00;
  }
}

/**
  * @brief  Writes a Entire-word buffer to the SDRAM memory.
  * @param  pBuffer: pointer to buffer.
  * @param  WriteAddress: SDRAM memory internal address from which the data will be
  *         written.
  * @param  BufferSize: number of words to write.
  * @retval None
  */
void SDRAM_WriteBuffer(uint32_t* pBuffer, uint32_t WriteAddress, uint32_t BufferSize)
{
  __IO uint32_t write_pointer = (uint32_t)WriteAddress;

  /* Disable write protection */
//  FMC_SDRAMWriteProtectionConfig(FMC_Bank2_SDRAM, DISABLE); /* TODO: Needed? */

  /* Wait until the SDRAM controller is ready */
//  while (FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET) {} /* TODO: Needed? */

  /* While there is data to write */
  for (; BufferSize != 0; BufferSize--)
  {
    /* Transfer data to the memory */
    *(uint32_t *) (SDRAM_BANK_ADDR + write_pointer) = *pBuffer++;

    /* Increment the address*/
    write_pointer += 4;
  }
}

/**
  * @brief  Reads data buffer from the SDRAM memory.
  * @param  pBuffer: pointer to buffer.
  * @param  ReadAddress: SDRAM memory internal address from which the data will be
  *         read.
  * @param  BufferSize: number of words to write
  * @retval None
  */
void SDRAM_ReadBuffer(uint32_t* pBuffer, uint32_t ReadAddress, uint32_t BufferSize)
{
  __IO uint32_t write_pointer = (uint32_t)ReadAddress;

  /* Wait until the SDRAM controller is ready */
//  while (FMC_GetFlagStatus(FMC_Bank2_SDRAM, FMC_FLAG_Busy) != RESET) {} /* TODO: Needed? */

  /* Read data */
  for (; BufferSize != 0x00; BufferSize--)
  {
    *pBuffer++ = *(__IO uint32_t *)(SDRAM_BANK_ADDR + write_pointer );

    /* Increment the address*/
    write_pointer += 4;
  }
}

/**
  * @brief  Test the SDRAM by saving 8-bit data and reading it back
  * @param  EndAddress: The address to stop at
  * @retval true: if everything went ok
  * @retval false: if something went wrong
  */
bool SDRAM_Test8bit(uint32_t EndAddress)
{
  uint8_t uhWritedata_8b = 0xDE, uhReaddata_8b = 0;
  uint32_t uwReadwritestatus = 0;
  uint32_t testAddress = 0x0;

  /* ============================ 8-bit test ============================ */
  /* Erase SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 1)
  {
    *(__IO uint8_t*) (testAddress) = (uint8_t)0x00;
  }

  /* Write data value to all SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 1)
  {
    *(__IO uint8_t*) (testAddress) = (uint8_t)(uhWritedata_8b + testAddress);
  }

  /* Read back SDRAM memory and check content correctness*/
  testAddress = SDRAM_BANK_ADDR;
  uwReadwritestatus = 0;
  while ((testAddress < EndAddress) && (uwReadwritestatus == 0))
  {
    uhReaddata_8b = *(__IO uint8_t*)(testAddress);
    if (uhReaddata_8b != (uint8_t)(uhWritedata_8b + testAddress))
    {
      /* Error has occured */
      uwReadwritestatus = 1;
      return false;
    }
    testAddress += 1;
  }
  /* If we reach this point everything is fine */
  /* Erase SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 1)
  {
    *(__IO uint8_t*) (testAddress) = (uint8_t)0x00;
  }
  return true;
}

/**
  * @brief  Test the SDRAM by saving 16-bit data and reading it back
  * @param  EndAddress: The address to stop at
  * @retval true: if everything went ok
  * @retval false: if something went wrong
  */
bool SDRAM_Test16bit(uint32_t EndAddress)
{
  uint16_t uhWritedata_16b = 0x1E5A, uhReaddata_16b = 0;
  uint32_t uwReadwritestatus = 0;
  uint32_t testAddress = 0x0;

  /* ============================ 16-bit test ============================ */
  /* Erase SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 2)
  {
    *(__IO uint16_t*) (testAddress) = (uint16_t)0x00;
  }

  /* Write data value to all SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 2)
  {
    *(__IO uint16_t*) (testAddress) = (uint16_t)(uhWritedata_16b + testAddress);
  }

  /* Read back SDRAM memory and check content correctness*/
  testAddress = SDRAM_BANK_ADDR;
  uwReadwritestatus = 0;
  while ((testAddress < EndAddress) && (uwReadwritestatus == 0))
  {
    uhReaddata_16b = *(__IO uint16_t*)(testAddress);
    if (uhReaddata_16b != (uint16_t)(uhWritedata_16b + testAddress))
    {
      /* Error has occured */
      uwReadwritestatus = 1;
      return false;
    }
    testAddress += 2;
  }
  /* If we reach this point everything is fine */
  /* Erase SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 2)
  {
    *(__IO uint16_t*) (testAddress) = (uint16_t)0x00;
  }
  return true;
}

/**
  * @brief  Test the SDRAM by saving 32-bit data and reading it back
  * @param  EndAddress: The address to stop at
  * @retval true: if everything went ok
  * @retval false: if something went wrong
  */
bool SDRAM_Test32bit(uint32_t EndAddress)
{
  uint32_t uhWritedata_32b = 0xDE35EFA8, uhReaddata_32b = 0;
  uint32_t uwReadwritestatus = 0;
  uint32_t testAddress = 0x0;

  /* ============================ 32-bit test ============================ */
  /* Erase SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 4)
  {
    *(__IO uint32_t*) (testAddress) = (uint32_t)0x00;
  }

  /* Write data value to all SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 4)
  {
    *(__IO uint32_t*) (testAddress) = (uint32_t)(uhWritedata_32b + testAddress);
  }

  /* Read back SDRAM memory and check content correctness*/
  testAddress = SDRAM_BANK_ADDR;
  uwReadwritestatus = 0;
  while ((testAddress < EndAddress) && (uwReadwritestatus == 0))
  {
    uhReaddata_32b = *(__IO uint32_t*)(testAddress);
    if (uhReaddata_32b != (uint32_t)(uhWritedata_32b + testAddress))
    {
      /* Error has occured */
      uwReadwritestatus = 1;
      return false;
    }
    testAddress += 4;
  }
  /* If we reach this point everything is fine */
  /* Erase SDRAM memory */
  for (testAddress = SDRAM_BANK_ADDR; testAddress < EndAddress; testAddress += 4)
  {
    *(__IO uint32_t*) (testAddress) = (uint32_t)0x00;
  }
  return true;
}

/** Private functions .-------------------------------------------------------*/
/**
 * @brief  Configures the GPIO for the SDRAM
 * @param  None
 * @retval  None
 */
static void prvGPIOConfig()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIO clocks */
  __GPIOB_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();
  __GPIOD_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();
  __GPIOF_CLK_ENABLE();
  __GPIOG_CLK_ENABLE();

/**-- GPIO Configuration -----------------------------------------------------*/
/*
 +-------------------+--------------------+--------------------+--------------------+
 +                       SDRAM pins assignment                                      +
 +-------------------+--------------------+--------------------+--------------------+
 | PD0  <-> FMC_D2   | PE0  <-> FMC_NBL0  | PF0  <-> FMC_A0    | PG0  <-> FMC_A10   |
 | PD1  <-> FMC_D3   | PE1  <-> FMC_NBL1  | PF1  <-> FMC_A1    | PG1  <-> FMC_A11   |
 | PD8  <-> FMC_D13  | PE7  <-> FMC_D4    | PF2  <-> FMC_A2    | PG2  <-> FMC_A12   |
 | PD9  <-> FMC_D14  | PE8  <-> FMC_D5    | PF3  <-> FMC_A3    | PG4  <-> FMC_BA0   |
 | PD10 <-> FMC_D15  | PE9  <-> FMC_D6    | PF4  <-> FMC_A4    | PG5  <-> FMC_BA1   |
 | PD14 <-> FMC_D0   | PE10 <-> FMC_D7    | PF5  <-> FMC_A5    | PG8  <-> FMC_SDCLK |
 | PD15 <-> FMC_D1   | PE11 <-> FMC_D8    | PF11 <-> FMC_NRAS  | PG15 <-> FMC_NCAS  |
 +-------------------| PE12 <-> FMC_D9    | PF12 <-> FMC_A6    |--------------------+
                     | PE13 <-> FMC_D10   | PF13 <-> FMC_A7    |
                     | PE14 <-> FMC_D11   | PF14 <-> FMC_A8    |
                     | PE15 <-> FMC_D12   | PF15 <-> FMC_A9    |
 +-------------------+--------------------+--------------------+
 | PB5 <-> FMC_SDCKE1|
 | PB6 <-> FMC_SDNE1 |
 | PC0 <-> FMC_SDNWE |
 +-------------------+
*/

  /* Common GPIO configuration */
  GPIO_InitStructure.Mode       = GPIO_MODE_AF_PP;
  GPIO_InitStructure.Pull       = GPIO_NOPULL;
  GPIO_InitStructure.Speed      = GPIO_SPEED_HIGH;
  GPIO_InitStructure.Alternate  = GPIO_AF12_FMC;

  /* GPIOB configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_5 | GPIO_PIN_6;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

  /* GPIOC configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_0;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* GPIOD configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 |
                            GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 |
                            GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

  /* GPIOE configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_7 |
                            GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 |
                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                            GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

  /* GPIOF configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                            GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
                            GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 |
                            GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);

  /* GPIOG configuration */
  GPIO_InitStructure.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                            GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 |
                            GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStructure);
}

/** Interrupt Handlers -------------------------------------------------------*/
