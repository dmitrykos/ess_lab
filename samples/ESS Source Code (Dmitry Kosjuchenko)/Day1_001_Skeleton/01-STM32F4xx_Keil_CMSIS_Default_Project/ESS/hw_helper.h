/* Copyright (c) 2019 by Dmitry Kostjuchenko (dmitrykos@neutroncode.com)
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef __HW_HELPER_H
#define __HW_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
 
#include "spi_driver.h"

#ifdef __cplusplus
extern C {
#endif

#define __hw_inline __inline
#define __hw_forceinline __forceinline

// Application base (must be the first member of the application structure)
typedef struct hw_app_base_t
{
	uint32_t __pad;
}
hw_app_base_t;

// Generic delay function prototype
typedef void (* DelayFunc)(uint32_t tm);

// Convert msec to usec
#define TIME_MSEC_TO_USEC(X) ((X) * 1000)

// Math helpers
#define MATH_ABS(V)          ((V) < 0 ? -(V) : (V))
#define MATH_FADEOUT_SINT(V) do { if ((V) != 0) (V) = ((V) > 0 ? (V) - 1 : (V) + 1); } while (0)

// Boolean
#define TRUE 1
#define FALSE 0
typedef uint8_t bool_t;
	
// User task handler
typedef void (* UserTaskHandler)(void *user_data);

// Vector3
typedef struct vector3_s16
{
	int16_t x,y,z;
}
vector3_s16;
typedef struct vector3_s32
{
	int32_t x,y,z;
}
vector3_s32;

#define VEC_MUL_SINGLE(VEC, M)   do { (VEC).x *= (M); (VEC).y *= (M); (VEC).z *= (M); } while (0)
#define VEC_DIV_SINGLE(VEC, D)   do { (VEC).x /= (D); (VEC).y /= (D); (VEC).z /= (D); } while (0)
#define VEC_SET(V1, V2)          do { (V1).x  = (V2).x; (V1).y  = (V2).y; (V1).z  = (V2).z; } while (0)
#define VEC_ADD(V1, V2)          do { (V1).x += (V2).x; (V1).y += (V2).y; (V1).z += (V2).z; } while (0)	
#define VEC_SUB(V1, V2)          do { (V1).x -= (V2).x; (V1).y -= (V2).y; (V1).z -= (V2).z; } while (0)
#define VEC_MAGNITUDE_SQARED(V1) ((V1).x * (V1).x + (V1).y * (V1).y + (V1).z * (V1).z)	
static __hw_forceinline vector3_s16 VEC_AVERAGE(vector3_s16 V1, vector3_s16 V2)
{
	(V1).x = ((int32_t)(V1).x + (V2).x) / 2;
	(V1).y = ((int32_t)(V1).y + (V2).y) / 2;
	(V1).z = ((int32_t)(V1).z + (V2).z) / 2;
	return V1;
}

// Buttons (GPIO)
#define GPIO__BUTTON_0 (1 << 0)

// Registers
#define REG__WHO_AM_I  0x0F
#define REG__CTRL_REG1 0x20
#define REG__OUT_X_L   0x28
#define REG__OUT_X_H   0x29
#define REG__OUT_Y_L   0x2A
#define REG__OUT_Y_H   0x2B
#define REG__OUT_Z_L   0x2C
#define REG__OUT_Z_H   0x2D

// Register values
#define VAL_REG1__SAMPLE_RATE_MAX 0x87

// Get register value from hi/lo
#define GET_REG_VAL_HILO(HI, LO) ((uint16_t)(LO) | ((uint16_t)(HI) << 8))

// SW delay
void TMR_SW_DelayUSec(uint32_t usec);
void TMR_SW_DelayMSec(uint32_t msec);

#define delay_usec TMR_SW_DelayUSec
#define delay_msec TMR_SW_DelayMSec

// HW delay
void TMR_HW_DelayUSec(uint32_t usec);
void TMR_HW_DelayMSec(uint32_t msec);

#define delay_hw_usec TMR_HW_DelayUSec
#define delay_hw_msec TMR_HW_DelayMSec

// TMR4 HW
#define TMR4_FREQUENCY 84000000
void TMR4_Init(uint16_t prescaler, uint16_t period);
void TMR4_InitResolutionUsec(uint32_t usec);
void TMR4_WaitForExpiry(void);

// TMR4 ISR
typedef enum ETmrIsrId
{
	TMR_ISR__3 = 3,
	TMR_ISR__4 = 4,
	
	TMR_ISR_MAX_ID
}
ETmrIsrId;

typedef void (* TMR_ISR_Callback)(void);

bool_t TMR_Init_ISR(ETmrIsrId id, uint16_t prescaler, uint16_t period, TMR_ISR_Callback cb);
void TMR_Init_ISR_ResolutionUsec(ETmrIsrId id, uint32_t usec, TMR_ISR_Callback cb);

// Read register (hi/lo) from SPI
static __hw_forceinline uint16_t spi_read_reg(uint8_t hi_reg, uint8_t lo_reg)
{
	return GET_REG_VAL_HILO(SPIAcc_GetByte(hi_reg), SPIAcc_GetByte(lo_reg));
}

// Temperature
void TEMP_Enable(void);
uint32_t TEMP_Read(void);
float TEMP_ValueToDeg(uint32_t amp);

#ifdef __cplusplus
}
#endif

#endif // __HW_HELPER_H
