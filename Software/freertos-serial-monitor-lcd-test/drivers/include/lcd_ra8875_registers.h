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
#define PWRR	(0x01)		/* Power and Display Control Register */
#define MRWC	(0x02)		/* Memory Read/Write Command */
#define PCSR	(0x04)		/* Pixel Clock Setting Register */
#define SROC	(0x05)		/* Serial Flash/ROM Configuration Register */
#define SFCLR	(0x06)		/* Serial Flash/ROM CLK Setting Register */
#define SYSR	(0x10)		/* System Configuration Register */
#define GPI		(0x12)		/* GPI */
#define GPO		(0x13)		/* GPO */
#define HDWR	(0x14)		/* LCD Horizontal Display Width Register */
#define HNDFTR	(0x15)		/* Horizontal Non-Display Period Fine Tuning Option Register */
#define HNDR	(0x16)		/* LCD Horizontal Non-Display Period Register */
#define HSTR	(0x17)		/* HSYNC Start Position Register */
#define HPWR	(0x18)		/* HSYNC Pulse Width Register */
#define VDHR0	(0x19)		/* LCD Vertical Display Height Register */
#define VDHR1	(0x1A)		/* LCD Vertical Display Height Register0 */
#define VNDR0	(0x1B)		/* LCD Vertical Non-Display Period Register */
#define VNDR1	(0x1C)		/* LCD Vertical Non-Display Period Register */
#define VSTR0	(0x1D)		/* VSYNC Start Position Register */
#define VSTR1	(0x1E)		/* VSYNC Start Position Register */
#define VPWR	(0x1F)		/* VSYNC Pulse Width Register */

/* LCD Display Control Registers */
#define DPCR	(0x20)		/* Display Configuration Register */
#define FNCR0	(0x21)		/* Font Control Register 0 */
#define FNCR1	(0x22)		/* Font Control Register 1 */
#define CGSR	(0x23)		/* CGRAM Select Register */
#define HOFS0	(0x24)		/* Horizontal Scroll Offset Register 0 */
#define HOFS1	(0x25)		/* Horizontal Scroll Offset Register 1 */
#define VOFS0	(0x26)		/* Vertical Scroll Offset Register 0 */
#define VOFS1	(0x27)		/* Vertical Scroll Offset Register 1 */
#define FLDR	(0x29)		/* Font Line Distance Setting Register */
#define F_CURXL	(0x2A)		/* Font Write Cursor Horizontal Position Register 0 */
#define F_CURXH	(0x2B)		/* Font Write Cursor Horizontal Position Register 1 */
#define F_CURYL	(0x2C)		/* Font Write Cursor Vertical Position Register 0 */
#define F_CURYH	(0x2D)		/* Font Write Cursor Vertical Position Register 1 */
#define FWTSR	(0x2E)		/* Font Write Type Setting Register */
#define SFRS	(0x2F)		/* Serial Font ROM Setting */

/* Active Window Setting Registers */
#define HSAW0	(0x30)		/* Horizontal Start Point 0 of Active Window */
#define HSAW1	(0x31)		/* Horizontal Start Point 1 of Active Window */
#define VSAW0	(0x32)		/* Vertical Start Point 0 of Active Window */
#define VSAW1	(0x33)		/* Vertical Start Point 1 of Active Window */
#define HEAW0	(0x34)		/* Horizontal End Point 0 of Active Window */
#define HEAW1	(0x35)		/* Horizontal End Point 1 of Active Window */
#define VEAW0	(0x36)		/* Vertical End Point 0 of Active Window */
#define VEAW1	(0x37)		/* Vertical End Point 1 of Active Window */
#define HSSW0	(0x38)		/* Horizontal Start Point 0 of Scroll Window */
#define HSSW1	(0x39)		/* Horizontal Start Point 1 of Scroll Window */
#define VSSW0	(0x3A)		/* Vertical Start Point 0 of Scroll Window */
#define VSSW1	(0x3B)		/* Vertical Start Point 1 of Scroll Window */
#define HESW0	(0x3C)		/* Horizontal End Point 0 of Scroll Window */
#define HESW1	(0x3D)		/* Horizontal End Point 1 of Scroll Window */
#define VESW0	(0x3E)		/* Vertical End Point 0 of Scroll Window */
#define VESW1	(0x3F)		/* Vertical End Point 1 of Scroll Window */

/* Cursor Setting Registers */
#define MWCR0	(0x40)		/* Memory Write Control Register 0 */
#define MWCR1	(0x41)		/* Memory Write Control Register 1 */
#define BTCR	(0x44)		/* Blink Time Control Register */
#define MRCD	(0x45)		/* Memory Read Cursor Direction */
#define CURH0	(0x46)		/* Memory Write Cursor Horizontal Position Register 0 */
#define CURH1	(0x47)		/* Memory Write Cursor Horizontal Position Register 1 */
#define CURV0	(0x48)		/* Memory Write Cursor Vertical Position Register 0 */
#define CURV1	(0x49)		/* Memory Write Cursor Vertical Position Register 1 */
#define RCURH0	(0x4A)		/* Memory Read Cursor Horizontal Position Register 0 */
#define RCURH1	(0x4B)		/* Memory Read Cursor Horizontal Position Register 1 */
#define RCURV0	(0x4C)		/* Memory Read Cursor Vertical Position Register 0 */
#define RCURV1	(0x4D)		/* Memory Read Cursor Vertical Position Register 1 */
#define CURHS	(0x4E)		/* Font Write Cursor and Memory Write Cursor Horizontal Size Register */
#define CURVS	(0x4F)		/* Font Write Cursor Vertical Size Register */

/* BTE Control Registers */
#define BECR0	(0x50)		/* BTE Function Control Register 0 */
#define BECR1	(0x51)		/* BTE Function Control Register 1 */
#define LTPR0	(0x52)		/* Layer Transparency Register0 */
#define LTPR1	(0x53)		/* Layer Transparency Register1 */
#define HSBE0	(0x54)		/* Horizontal Source Point 0 of BTE */
#define HSBE1	(0x55)		/* Horizontal Source Point 1 of BTE */
#define VSBE0	(0x56)		/* Vertical Source Point 0 of BTE */
#define VSBE1	(0x57)		/* Vertical Source Point 1 of BTE */
#define HDBE0	(0x58)		/* Horizontal Destination Point 0 of BTE */
#define HDBE1	(0x59)		/* Horizontal Destination Point 1 of BTE */
#define VDBE0	(0x5A)		/* Vertical Destination Point 0 of BTE */
#define VDBE1	(0x5B)		/* Vertical Destination Point 1 of BTE */
#define BEWR0	(0x5C)		/* BTE Width Register 0 */
#define BEWR1	(0x5D)		/* BTE Width Register 1 */
#define BEHR0	(0x5E)		/* BTE Height Register 0 */
#define BEHR1	(0x5F)		/* BTE Height Register 1 */

#define BGCR0	(0x60)		/* Background Color Register 0 */
#define BGCR1	(0x61)		/* Background Color Register 1 */
#define BGCR2	(0x62)		/* Background Color Register 2 */
#define FGCR0	(0x63)		/* Foreground Color Register 0 */
#define FGCR1	(0x64)		/* Foreground Color Register 1 */
#define FGCR2	(0x65)		/* Foreground Color Register 2 */
#define PTNO	(0x66)		/* Pattern Set No for BTE */
#define BGTR0	(0x67)		/* Background Color Register for Transparent 0 */
#define BGTR1	(0x68)		/* Background Color Register for Transparent 1 */
#define BGTR2	(0x69)		/* Background Color Register for Transparent 2 */

/* Touch Panel Control Registers */
#define TPCR0	(0x70)		/* Touch Panel Control Register 0 */
#define TPCR1	(0x71)		/* Touch Panel Control Register 1 */
#define TPXH	(0x72)		/* Touch Panel X High Byte Data Register */
#define TPYH	(0x73)		/* Touch Panel Y High Byte Data Register */
#define TPXYL	(0x74)		/* Touch Panel X/Y Low Byte Data Register */

/* Graphic Cursor Setting Registers */
#define GCHP0	(0x80)		/* Graphic Cursor Horizontal Position Register 0 */
#define GCHP1	(0x81)		/* Graphic Cursor Horizontal Position Register 1 */
#define GCVP0	(0x82)		/* Graphic Cursor Vertical Position Register 0 */
#define GCVP1	(0x83)		/* Graphic Cursor Vertical Position Register 1 */
#define GCC0	(0x84)		/* Graphic Cursor Color 0 */
#define GCC1	(0x85)		/* Graphic Cursor Color 1 */

/* PLL Setting Registers */
#define PLLC1	(0x88)		/* PLL Control Register 1 */
#define PLLC2	(0x89)		/* PLL Control Register 2 */

/* PWM Control Registers */
#define P1CR	(0x8A)		/* PWM1 Control Register */
#define P1DCR	(0x8B)		/* PWM1 Duty Cycle Register */
#define P2CR	(0x8C)		/* PWM2 Control Register */
#define P2DCR	(0x8D)		/* PWM2 Duty Cycle Register */
#define MCLR	(0x8E)		/* Memory Clear Control Register */

/* Drawing Control Registers */
#define DCR		(0x90)		/* Draw Line/Circle/Square Control Register */
#define DLHSR0	(0x91)		/* Draw Line/Square Horizontal Start Address Register0 */
#define DLHSR1	(0x92)		/* Draw Line/Square Horizontal Start Address Register1 */
#define DLVSR0	(0x93)		/* Draw Line/Square Vertical Start Address Register0 */
#define DLVSR1	(0x94)		/* Draw Line/Square Vertical Start Address Register1 */
#define DLHER0	(0x95)		/* Draw Line/Square Horizontal End Address Register0 */
#define DLHER1	(0x96)		/* Draw Line/Square Horizontal End Address Register1 */
#define DLVER0	(0x97)		/* Draw Line/Square Vertical End Address Register0 */
#define DLVER1	(0x98)		/* Draw Line/Square Vertical End Address Register1 */

#define DCHR0	(0x99)		/* Draw Circle Center Horizontal Address Register0 */
#define DCHR1	(0x9A)		/* Draw Circle Center Horizontal Address Register1 */
#define DCVR0	(0x9B)		/* Draw Circle Center Vertical Address Register0 */
#define DCVR1	(0x9C)		/* Draw Circle Center Vertical Address Register1 */
#define DCRR	(0x9D)		/* Draw Circle Radius Register */

#define ELLCR	(0xA0)		/* Draw Ellipse/Ellipse Curve/Circle Square Control Register */
#define ELL_A0	(0xA1)		/* Draw Ellipse/Circle Square Long axis Setting Register0 */
#define ELL_A1	(0xA2)		/* Draw Ellipse/Circle Square Long axis Setting Register1 */
#define ELL_B0	(0xA3)		/* Draw Ellipse/Circle Square Short axis Setting Register0 */
#define ELL_B1	(0xA4)		/* Draw Ellipse/Circle Square Short axis Setting Register1 */
#define DEHR0	(0xA5)		/* Draw Ellipse/Circle Square Center Horizontal Address Register0 */
#define DEHR1	(0xA6)		/* Draw Ellipse/Circle Square Center Horizontal Address Register1 */
#define DEVR0	(0xA7)		/* Draw Ellipse/Circle Square Center Vertical Address Register0 */
#define DEVR1	(0xA8)		/* Draw Ellipse/Circle Square Center Vertical Address Register1 */

#define DTPH0	(0xA9)		/* Draw Triangle Point 2 Horizontal Address Register0 */
#define DTPH1	(0xAA)		/* Draw Triangle Point 2 Horizontal Address Register1 */
#define DTPV0	(0xAB)		/* Draw Triangle Point 2 Vertical Address Register0 */
#define DTPV1	(0xAC)		/* Draw Triangle Point 2 Vertical Address Register1 */

/* DMA Control Registers */
#define SSAR0		(0xB0)		/* Source Starting Address REG0 */
#define SSAR1		(0xB1)		/* Source Starting Address REG1 */
#define SSAR2		(0xB2)		/* Source Starting Address REG2 */
#define BWR0_DTNR0	(0xB4)		/* Block Width REG 0(BWR0) / DMA Transfer Number REG 0 */
#define BWR1		(0xB5)		/* Block Width REG 1 */
#define BHR0_DTNR1	(0xB6)		/* Block Height REG 0(BHR0) /DMA Transfer Number REG 1 */
#define BHR1		(0xB7)		/* Block Height REG 1 */
#define SPWR0_DTNR2	(0xB8)		/* Source Picture Width REG 0(SPWR0) / DMA Transfer Number REG 2 */
#define SPWR1		(0xB9)		/* Source Picture Width REG 1 */
#define DMACR		(0xBF)		/* DMA Configuration REG  */

/* KEY & IO Control Registers */
#define KSCR1	(0xC0)		/* Key-Scan Control Register 1 */
#define KSCR2	(0xC1)		/* Key-Scan Control Register 2 */
#define KSDR0	(0xC2)		/* Key-Scan Data Register 0 */
#define KSDR1	(0xC3)		/* Key-Scan Data Register 1 */
#define KSDR2	(0xC4)		/* Key-Scan Data Register 2 */
#define GPIOX	(0xC7)		/* Extra General Purpose IO Register */

/* Floating Window Control Registers */
#define FWSAXA0	(0xD0)		/* Floating Windows Start Address XA 0 */
#define FWSAXA1	(0xD1)		/* Floating Windows Start Address XA 1 */
#define FWSAYA0	(0xD2)		/* Floating Windows Start Address YA 0 */
#define FWSAYA1	(0xD3)		/* Floating Windows Start Address YA 1 */
#define FWW0	(0xD4)		/* Floating Windows Width 0 */
#define FWW1	(0xD5)		/* Floating Windows Width 1 */
#define FWH0	(0xD6)		/* Floating Windows Height 0 */
#define FWH1	(0xD7)		/* Floating Windows Height 1 */
#define FWDXA0	(0xD8)		/* Floating Windows Display X Address 0 */
#define FWDXA1	(0xD9)		/* Floating Windows Display X Address 1 */
#define FWDYA0	(0xDA)		/* Floating Windows Display Y Address 0 */
#define FWDYA1	(0xDB)		/* Floating Windows Display Y Address 1 */

/* Serial Flash Control Registers */
#define SFRDAM	(0xE0)		/* Serial Flash/ROM Direct Access Mode */
#define SFRDAMA	(0xE1)		/* Serial Flash/ROM Direct Access Mode Address */
#define SFRDAMR	(0xE2)		/* Serial Flash/ROM Direct Access Data Read */

/* Interrupt Control Registers */
#define INTC1	(0xF0)		/* Interrupt Control Register1 */
#define INTC2	(0xF1)		/* Interrupt Control Register2 */


/* Typedefs ------------------------------------------------------------------*/
/* Function prototypes -------------------------------------------------------*/

#endif /* _REGISTERS */
