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

#include "acc_driver.h"

static volatile bool_t g_Tmr3IsrRdy = FALSE; // primitive spin lock
static acc3_t g_AccValAsync = {0,0,0};

static void Tm3IsrCallback(void)
{
	if (!g_Tmr3IsrRdy)
	{	
		acc_read(&g_AccValAsync);	
		g_Tmr3IsrRdy = TRUE;
	}
}

void acc_init(bool_t async, uint32_t async_freq)
{
	SPIAcc_Init();
	SPIAcc_SendByte(REG__CTRL_REG1, VAL_REG1__SAMPLE_RATE_MAX);
	
	if (async)
		TMR_Init_ISR_ResolutionUsec(TMR_ISR__3, 1000 * 1000 / async_freq, &Tm3IsrCallback);
}

void acc_read(acc3_t *accv)
{
	accv->x = spi_read_reg(REG__OUT_X_H, REG__OUT_X_L);
	accv->y = spi_read_reg(REG__OUT_Y_H, REG__OUT_Y_L);
	accv->z = spi_read_reg(REG__OUT_Z_H, REG__OUT_Z_L);
}

bool_t acc_read_async(acc3_t *accv)
{
	if (!g_Tmr3IsrRdy)
		return FALSE;
	
	(*accv) = g_AccValAsync;	
	g_Tmr3IsrRdy = FALSE;
	
	return TRUE;
}

bool_t acc_read_buffer_async(acc_fifo_t *buf, UserTaskHandler user_cb, void *user_data)
{
	acc3_t accv;

	while (1)
	{	
		// allow some other tasks while waiting for the full buffer read
		user_cb(user_data);
		
		if (acc_read_async(&accv))
		{
			acc_fifo_push(buf, &accv);
			
			// filled buffer fully, return to user
			if (acc_fifo_get_wpos(buf) == 0)
				return TRUE;
		}
	}
}

void acc_set_pwm_driver2(const acc3_t *acc, int16_t center)
{
	if (acc->x > center)
	{
		pwm_driver2_set(PWM_CH__GREEN, acc_get_pwm_brightness(acc->x));
		pwm_driver2_set(PWM_CH__RED, 0);
	}
	else
	if (acc->x < -center)
	{
		pwm_driver2_set(PWM_CH__RED, acc_get_pwm_brightness(acc->x));
		pwm_driver2_set(PWM_CH__GREEN, 0);
	}
	else
	{
		pwm_driver2_set(PWM_CH__RED, 0);
		pwm_driver2_set(PWM_CH__GREEN, 0);
	}
	
	if (acc->y > center)
	{
		pwm_driver2_set(PWM_CH__BLUE, acc_get_pwm_brightness(acc->y));
		pwm_driver2_set(PWM_CH__ORANGE, 0);
	}
	else
	if (acc->y < -center)
	{
		pwm_driver2_set(PWM_CH__ORANGE, acc_get_pwm_brightness(acc->y));
		pwm_driver2_set(PWM_CH__BLUE, 0);
	}
	else
	{
		pwm_driver2_set(PWM_CH__BLUE, 0);
		pwm_driver2_set(PWM_CH__ORANGE, 0);
	}
}

void acc_fifo_init(acc_fifo_t *buf)
{
	memset(buf->buf, 0, sizeof(buf->buf));
	buf->pos = 0;
}

void acc_fifo_clear(acc_fifo_t *buf)
{
	buf->pos = 0;
}

void acc_fifo_push(acc_fifo_t *buf, const acc3_t *val)
{
	buf->buf[buf->pos] = (*val);
	
	if (++buf->pos >= ACC_FIFO_SIZE)
		buf->pos = 0;
}

void acc_fifo_get_last(const acc_fifo_t *buf, acc3_t *val)
{
	if (buf->pos > 0)
		(*val) = buf->buf[buf->pos - 1];
	else
		(*val) = buf->buf[ACC_FIFO_SIZE - 1];
}

uint32_t acc_fifo_get_wpos(const acc_fifo_t *buf)
{
	return buf->pos;
}

void acc_fifo_get_average(const acc_fifo_t *buf, acc3_t *val)
{
	int32_t i;
	int64_t ax = 0, ay = 0, az = 0;
	
	for (i = 0; i < ACC_FIFO_SIZE; ++i)
	{
		ax += buf->buf[i].x;
		ay += buf->buf[i].y;
		az += buf->buf[i].z;
	}
	
	val->x = ax / ACC_FIFO_SIZE;
	val->y = ay / ACC_FIFO_SIZE;
	val->z = az / ACC_FIFO_SIZE;
}

void acc_fifo_merge(acc_fifo_t *to, const acc_fifo_t *from)
{
	int32_t i;
	
	for (i = 0; i < ACC_FIFO_SIZE; ++i)
	{
		to->buf[i].x = ((int32_t)to->buf[i].x + from->buf[i].x) / 2;
		to->buf[i].y = ((int32_t)to->buf[i].y + from->buf[i].y) / 2;
		to->buf[i].z = ((int32_t)to->buf[i].z + from->buf[i].z) / 2;
	}
}
