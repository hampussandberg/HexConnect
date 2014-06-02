/**
 ******************************************************************************
 * @file	lcd_ra8875_registers.h
 * @author	Hampus Sandberg
 * @version	0.1
 * @date	2014-06-02
 * @brief
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_RA8875_REGISTERS_H_
#define LCD_RA8875_REGISTERS_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Defines -------------------------------------------------------------------*/
/* System and Configuration Registers */
#define LCD_PWRR	(0x01)		/* Power and Display Control Register */
#define LCD_MRWC	(0x02)		/* Memory Read/Write Command */
#define LCD_PCSR	(0x04)		/* Pixel Clock Setting Register */
#define LCD_SROC	(0x05)		/* Serial Flash/ROM Configuration Register */
#define LCD_SFCLR	(0x06)		/* Serial Flash/ROM CLK Setting Register */
#define LCD_SYSR	(0x10)		/* System Configuration Register */
#define LCD_GPI		(0x12)		/* GPI */
#define LCD_GPO		(0x13)		/* GPO */
#define LCD_HDWR	(0x14)		/* LCD Horizontal Display Width Register */
#define LCD_HNDFTR	(0x15)		/* Horizontal Non-Display Period Fine Tuning Option Register */
#define LCD_HNDR	(0x16)		/* LCD Horizontal Non-Display Period Register */
#define LCD_HSTR	(0x17)		/* HSYNC Start Position Register */
#define LCD_HPWR	(0x18)		/* HSYNC Pulse Width Register */
#define LCD_VDHR0	(0x19)		/* LCD Vertical Display Height Register */
#define LCD_VDHR1	(0x1A)		/* LCD Vertical Display Height Register0 */
#define LCD_VNDR0	(0x1B)		/* LCD Vertical Non-Display Period Register */
#define LCD_VNDR1	(0x1C)		/* LCD Vertical Non-Display Period Register */
#define LCD_VSTR0	(0x1D)		/* VSYNC Start Position Register */
#define LCD_VSTR1	(0x1E)		/* VSYNC Start Position Register */
#define LCD_VPWR	(0x1F)		/* VSYNC Pulse Width Register */

/* LCD Display Control Registers */
#define LCD_DPCR	(0x20)		/* Display Configuration Register */
#define LCD_FNCR0	(0x21)		/* Font Control Register 0 */
#define LCD_FNCR1	(0x22)		/* Font Control Register 1 */
#define LCD_CGSR	(0x23)		/* CGRAM Select Register */
#define LCD_HOFS0	(0x24)		/* Horizontal Scroll Offset Register 0 */
#define LCD_HOFS1	(0x25)		/* Horizontal Scroll Offset Register 1 */
#define LCD_VOFS0	(0x26)		/* Vertical Scroll Offset Register 0 */
#define LCD_VOFS1	(0x27)		/* Vertical Scroll Offset Register 1 */
#define LCD_FLDR	(0x29)		/* Font Line Distance Setting Register */
#define LCD_F_CURXL	(0x2A)		/* Font Write Cursor Horizontal Position Register 0 */
#define LCD_F_CURXH	(0x2B)		/* Font Write Cursor Horizontal Position Register 1 */
#define LCD_F_CURYL	(0x2C)		/* Font Write Cursor Vertical Position Register 0 */
#define LCD_F_CURYH	(0x2D)		/* Font Write Cursor Vertical Position Register 1 */
#define LCD_FWTSR	(0x2E)		/* Font Write Type Setting Register */
#define LCD_SFRS	(0x2F)		/* Serial Font ROM Setting */

/* Active Window Setting Registers */
#define LCD_HSAW0	(0x30)		/* Horizontal Start Point 0 of Active Window */
#define LCD_HSAW1	(0x31)		/* Horizontal Start Point 1 of Active Window */
#define LCD_VSAW0	(0x32)		/* Vertical Start Point 0 of Active Window */
#define LCD_VSAW1	(0x33)		/* Vertical Start Point 1 of Active Window */
#define LCD_HEAW0	(0x34)		/* Horizontal End Point 0 of Active Window */
#define LCD_HEAW1	(0x35)		/* Horizontal End Point 1 of Active Window */
#define LCD_VEAW0	(0x36)		/* Vertical End Point 0 of Active Window */
#define LCD_VEAW1	(0x37)		/* Vertical End Point 1 of Active Window */
#define LCD_HSSW0	(0x38)		/* Horizontal Start Point 0 of Scroll Window */
#define LCD_HSSW1	(0x39)		/* Horizontal Start Point 1 of Scroll Window */
#define LCD_VSSW0	(0x3A)		/* Vertical Start Point 0 of Scroll Window */
#define LCD_VSSW1	(0x3B)		/* Vertical Start Point 1 of Scroll Window */
#define LCD_HESW0	(0x3C)		/* Horizontal End Point 0 of Scroll Window */
#define LCD_HESW1	(0x3D)		/* Horizontal End Point 1 of Scroll Window */
#define LCD_VESW0	(0x3E)		/* Vertical End Point 0 of Scroll Window */
#define LCD_VESW1	(0x3F)		/* Vertical End Point 1 of Scroll Window */

/* Cursor Setting Registers */
#define LCD_MWCR0	(0x40)		/* Memory Write Control Register 0 */
#define LCD_MWCR1	(0x41)		/* Memory Write Control Register 1 */
#define LCD_BTCR	(0x44)		/* Blink Time Control Register */
#define LCD_MRCD	(0x45)		/* Memory Read Cursor Direction */
#define LCD_CURH0	(0x46)		/* Memory Write Cursor Horizontal Position Register 0 */
#define LCD_CURH1	(0x47)		/* Memory Write Cursor Horizontal Position Register 1 */
#define LCD_CURV0	(0x48)		/* Memory Write Cursor Vertical Position Register 0 */
#define LCD_CURV1	(0x49)		/* Memory Write Cursor Vertical Position Register 1 */
#define LCD_RCURH0	(0x4A)		/* Memory Read Cursor Horizontal Position Register 0 */
#define LCD_RCURH1	(0x4B)		/* Memory Read Cursor Horizontal Position Register 1 */
#define LCD_RCURV0	(0x4C)		/* Memory Read Cursor Vertical Position Register 0 */
#define LCD_RCURV1	(0x4D)		/* Memory Read Cursor Vertical Position Register 1 */
#define LCD_CURHS	(0x4E)		/* Font Write Cursor and Memory Write Cursor Horizontal Size Register */
#define LCD_CURVS	(0x4F)		/* Font Write Cursor Vertical Size Register */

/* BTE Control Registers */
#define LCD_BECR0	(0x50)		/* BTE Function Control Register 0 */
#define LCD_BECR1	(0x51)		/* BTE Function Control Register 1 */
#define LCD_LTPR0	(0x52)		/* Layer Transparency Register0 */
#define LCD_LTPR1	(0x53)		/* Layer Transparency Register1 */
#define LCD_HSBE0	(0x54)		/* Horizontal Source Point 0 of BTE */
#define LCD_HSBE1	(0x55)		/* Horizontal Source Point 1 of BTE */
#define LCD_VSBE0	(0x56)		/* Vertical Source Point 0 of BTE */
#define LCD_VSBE1	(0x57)		/* Vertical Source Point 1 of BTE */
#define LCD_HDBE0	(0x58)		/* Horizontal Destination Point 0 of BTE */
#define LCD_HDBE1	(0x59)		/* Horizontal Destination Point 1 of BTE */
#define LCD_VDBE0	(0x5A)		/* Vertical Destination Point 0 of BTE */
#define LCD_VDBE1	(0x5B)		/* Vertical Destination Point 1 of BTE */
#define LCD_BEWR0	(0x5C)		/* BTE Width Register 0 */
#define LCD_BEWR1	(0x5D)		/* BTE Width Register 1 */
#define LCD_BEHR0	(0x5E)		/* BTE Height Register 0 */
#define LCD_BEHR1	(0x5F)		/* BTE Height Register 1 */

#define LCD_BGCR0	(0x60)		/* Background Color Register 0 */
#define LCD_BGCR1	(0x61)		/* Background Color Register 1 */
#define LCD_BGCR2	(0x62)		/* Background Color Register 2 */
#define LCD_FGCR0	(0x63)		/* Foreground Color Register 0 */
#define LCD_FGCR1	(0x64)		/* Foreground Color Register 1 */
#define LCD_FGCR2	(0x65)		/* Foreground Color Register 2 */
#define LCD_PTNO	(0x66)		/* Pattern Set No for BTE */
#define LCD_BGTR0	(0x67)		/* Background Color Register for Transparent 0 */
#define LCD_BGTR1	(0x68)		/* Background Color Register for Transparent 1 */
#define LCD_BGTR2	(0x69)		/* Background Color Register for Transparent 2 */

/* Touch Panel Control Registers */
#define LCD_TPCR0	(0x70)		/* Touch Panel Control Register 0 */
#define LCD_TPCR1	(0x71)		/* Touch Panel Control Register 1 */
#define LCD_TPXH	(0x72)		/* Touch Panel X High Byte Data Register */
#define LCD_TPYH	(0x73)		/* Touch Panel Y High Byte Data Register */
#define LCD_TPXYL	(0x74)		/* Touch Panel X/Y Low Byte Data Register */

/* Graphic Cursor Setting Registers */
#define LCD_GCHP0	(0x80)		/* Graphic Cursor Horizontal Position Register 0 */
#define LCD_GCHP1	(0x81)		/* Graphic Cursor Horizontal Position Register 1 */
#define LCD_GCVP0	(0x82)		/* Graphic Cursor Vertical Position Register 0 */
#define LCD_GCVP1	(0x83)		/* Graphic Cursor Vertical Position Register 1 */
#define LCD_GCC0	(0x84)		/* Graphic Cursor Color 0 */
#define LCD_GCC1	(0x85)		/* Graphic Cursor Color 1 */

/* PLL Setting Registers */
#define LCD_PLLC1	(0x88)		/* PLL Control Register 1 */
#define LCD_PLLC2	(0x89)		/* PLL Control Register 2 */

/* PWM Control Registers */
#define LCD_P1CR	(0x8A)		/* PWM1 Control Register */
#define LCD_P1DCR	(0x8B)		/* PWM1 Duty Cycle Register */
#define LCD_P2CR	(0x8C)		/* PWM2 Control Register */
#define LCD_P2DCR	(0x8D)		/* PWM2 Duty Cycle Register */
#define LCD_MCLR	(0x8E)		/* Memory Clear Control Register */

/* Drawing Control Registers */
#define LCD_DCR		(0x90)		/* Draw Line/Circle/Square Control Register */
#define LCD_DLHSR0	(0x91)		/* Draw Line/Square Horizontal Start Address Register0 */
#define LCD_DLHSR1	(0x92)		/* Draw Line/Square Horizontal Start Address Register1 */
#define LCD_DLVSR0	(0x93)		/* Draw Line/Square Vertical Start Address Register0 */
#define LCD_DLVSR1	(0x94)		/* Draw Line/Square Vertical Start Address Register1 */
#define LCD_DLHER0	(0x95)		/* Draw Line/Square Horizontal End Address Register0 */
#define LCD_DLHER1	(0x96)		/* Draw Line/Square Horizontal End Address Register1 */
#define LCD_DLVER0	(0x97)		/* Draw Line/Square Vertical End Address Register0 */
#define LCD_DLVER1	(0x98)		/* Draw Line/Square Vertical End Address Register1 */

#define LCD_DCHR0	(0x99)		/* Draw Circle Center Horizontal Address Register0 */
#define LCD_DCHR1	(0x9A)		/* Draw Circle Center Horizontal Address Register1 */
#define LCD_DCVR0	(0x9B)		/* Draw Circle Center Vertical Address Register0 */
#define LCD_DCVR1	(0x9C)		/* Draw Circle Center Vertical Address Register1 */
#define LCD_DCRR	(0x9D)		/* Draw Circle Radius Register */

#define LCD_ELLCR	(0xA0)		/* Draw Ellipse/Ellipse Curve/Circle Square Control Register */
#define LCD_ELL_A0	(0xA1)		/* Draw Ellipse/Circle Square Long axis Setting Register0 */
#define LCD_ELL_A1	(0xA2)		/* Draw Ellipse/Circle Square Long axis Setting Register1 */
#define LCD_ELL_B0	(0xA3)		/* Draw Ellipse/Circle Square Short axis Setting Register0 */
#define LCD_ELL_B1	(0xA4)		/* Draw Ellipse/Circle Square Short axis Setting Register1 */
#define LCD_DEHR0	(0xA5)		/* Draw Ellipse/Circle Square Center Horizontal Address Register0 */
#define LCD_DEHR1	(0xA6)		/* Draw Ellipse/Circle Square Center Horizontal Address Register1 */
#define LCD_DEVR0	(0xA7)		/* Draw Ellipse/Circle Square Center Vertical Address Register0 */
#define LCD_DEVR1	(0xA8)		/* Draw Ellipse/Circle Square Center Vertical Address Register1 */

#define LCD_DTPH0	(0xA9)		/* Draw Triangle Point 2 Horizontal Address Register0 */
#define LCD_DTPH1	(0xAA)		/* Draw Triangle Point 2 Horizontal Address Register1 */
#define LCD_DTPV0	(0xAB)		/* Draw Triangle Point 2 Vertical Address Register0 */
#define LCD_DTPV1	(0xAC)		/* Draw Triangle Point 2 Vertical Address Register1 */

/* DMA Control Registers */
#define LCD_SSAR0		(0xB0)		/* Source Starting Address REG0 */
#define LCD_SSAR1		(0xB1)		/* Source Starting Address REG1 */
#define LCD_SSAR2		(0xB2)		/* Source Starting Address REG2 */
#define LCD_BWR0_DTNR0	(0xB4)		/* Block Width REG 0(BWR0) / DMA Transfer Number REG 0 */
#define LCD_BWR1		(0xB5)		/* Block Width REG 1 */
#define LCD_BHR0_DTNR1	(0xB6)		/* Block Height REG 0(BHR0) /DMA Transfer Number REG 1 */
#define LCD_BHR1		(0xB7)		/* Block Height REG 1 */
#define LCD_SPWR0_DTNR2	(0xB8)		/* Source Picture Width REG 0(SPWR0) / DMA Transfer Number REG 2 */
#define LCD_SPWR1		(0xB9)		/* Source Picture Width REG 1 */
#define LCD_DMACR		(0xBF)		/* DMA Configuration REG  */

/* KEY & IO Control Registers */
#define LCD_KSCR1	(0xC0)		/* Key-Scan Control Register 1 */
#define LCD_KSCR2	(0xC1)		/* Key-Scan Control Register 2 */
#define LCD_KSDR0	(0xC2)		/* Key-Scan Data Register 0 */
#define LCD_KSDR1	(0xC3)		/* Key-Scan Data Register 1 */
#define LCD_KSDR2	(0xC4)		/* Key-Scan Data Register 2 */
#define LCD_GPIOX	(0xC7)		/* Extra General Purpose IO Register */

/* Floating Window Control Registers */
#define LCD_FWSAXA0	(0xD0)		/* Floating Windows Start Address XA 0 */
#define LCD_FWSAXA1	(0xD1)		/* Floating Windows Start Address XA 1 */
#define LCD_FWSAYA0	(0xD2)		/* Floating Windows Start Address YA 0 */
#define LCD_FWSAYA1	(0xD3)		/* Floating Windows Start Address YA 1 */
#define LCD_FWW0	(0xD4)		/* Floating Windows Width 0 */
#define LCD_FWW1	(0xD5)		/* Floating Windows Width 1 */
#define LCD_FWH0	(0xD6)		/* Floating Windows Height 0 */
#define LCD_FWH1	(0xD7)		/* Floating Windows Height 1 */
#define LCD_FWDXA0	(0xD8)		/* Floating Windows Display X Address 0 */
#define LCD_FWDXA1	(0xD9)		/* Floating Windows Display X Address 1 */
#define LCD_FWDYA0	(0xDA)		/* Floating Windows Display Y Address 0 */
#define LCD_FWDYA1	(0xDB)		/* Floating Windows Display Y Address 1 */

/* Serial Flash Control Registers */
#define LCD_SFRDAM	(0xE0)		/* Serial Flash/ROM Direct Access Mode */
#define LCD_SFRDAMA	(0xE1)		/* Serial Flash/ROM Direct Access Mode Address */
#define LCD_SFRDAMR	(0xE2)		/* Serial Flash/ROM Direct Access Data Read */

/* Interrupt Control Registers */
#define LCD_INTC1	(0xF0)		/* Interrupt Control Register1 */
#define LCD_INTC2	(0xF1)		/* Interrupt Control Register2 */


/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

#endif /* _REGISTERS */
