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

#ifndef __ACC_DRIVER_H
#define __ACC_DRIVER_H

#include "hw_helper.h"
#include "pwm_driver.h"

#ifdef __cplusplus
extern C {
#endif

#define ACC_VAL_CENTER    1024
#define ACC_VAL_MAX       0x7FFF
#define ACC_VAL_TO_PWM(V) ((int32_t)(V) * PWM_MAX / (ACC_VAL_MAX - ACC_VAL_CENTER))

#define ACC_UPDATE_FREQ   32
#define ACC_FIFO_SIZE     (ACC_UPDATE_FREQ / 2)

typedef vector3_s16 acc3_t;

typedef struct acc_fifo_t
{	
	acc3_t   buf[ACC_FIFO_SIZE];
	uint32_t pos;
}
acc_fifo_t;

// Generic delay function prototype
typedef void (* AccIdleCallback)(void *user_data);

// Accelerator
void acc_init(bool_t async, uint32_t async_freq);
void acc_read(acc3_t *accv);
bool_t acc_read_async(acc3_t *accv);
bool_t acc_read_buffer_async(acc_fifo_t *buf, AccIdleCallback idle_cb, void *user_data);

// Convert accelerometer value to PWM brightensess
static __hw_forceinline int16_t acc_get_pwm_brightness(int16_t v)
{
	v = MATH_ABS(v);	
	return (v <= ACC_VAL_CENTER ? 0 : ACC_VAL_TO_PWM(v - ACC_VAL_CENTER));
}

// Update PWM2 with accelerometer's value
void acc_set_pwm_driver2(const acc3_t *acc, int16_t center);

// Accelerator FIFO
void acc_fifo_init(acc_fifo_t *buf);
void acc_fifo_clear(acc_fifo_t *buf);
void acc_fifo_push(acc_fifo_t *buf, const acc3_t *val);
void acc_fifo_get_last(const acc_fifo_t *buf, acc3_t *val);
uint32_t acc_fifo_get_wpos(const acc_fifo_t *buf);
void acc_fifo_merge(acc_fifo_t *to, const acc_fifo_t *from);
void acc_fifo_get_average(const acc_fifo_t *buf, acc3_t *val);

#ifdef __cplusplus
}
#endif

#endif // __ACC_DRIVER_H
