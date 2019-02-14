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

// Lab 3

#include "string.h"
#include "pwm_driver.h"
#include "acc_driver.h"

void lab3_task_1_a()
{	
	LED_t led;
	volatile uint8_t rval;
	
	SPIAcc_Init();
	led_init(&led, PORTD, LED_GREEN);
		
	rval = SPIAcc_GetByte(REG__WHO_AM_I);
	
	if (rval == 0x3F) // bug in handout, not 0x33
		led_on(&led);
	
	while (1)
	{ }	
}

void lab3_task_1_bcde()
{
	acc3_t accv;
	uint32_t time_fade = 0;
	
	// init accelerometer
	acc_init(FALSE, 0);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	while (1)
	{
		// read accelerator data inside main loop
		acc_read(&accv);
		
		// compensate position jitter for 50 msecs
		if ((time_fade += PWM_RESOLUTION_USEC) >= TIME_MSEC_TO_USEC(50))
		{	
			// visualize
			acc_set_pwm_driver2(&accv, ACC_VAL_CENTER);			
			time_fade = 0;
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

void lab3_task_1_i()
{	
	acc3_t accv;
	acc_fifo_t fifo;
	
	// init async accelerometer with 32 Hz update rate
	acc_init(TRUE, ACC_UPDATE_FREQ);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	// init fifo
	acc_fifo_init(&fifo);
	
	while (1)
	{
		// wait for accelerator's data (HW reads offloaded in IRQ's thread)
		if (acc_read_async(&accv))
		{	
			// visualize
			acc_set_pwm_driver2(&accv, ACC_VAL_CENTER);
			
			// store data of 1/2 seconds (ACC_UPDATE_FREQ / 2) in a fifo buffer
			acc_fifo_push(&fifo, &accv);
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

void lab3_task_1_j()
{
	acc3_t accv, tilt;
	acc_fifo_t fifo;
	
	// init async accelerometer with 32 Hz (ACC_UPDATE_FREQ) update rate
	acc_init(TRUE, ACC_UPDATE_FREQ);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	// init fifo
	acc_fifo_init(&fifo);
	
	while (1)
	{
		// wait for a fifo buffer (filled with 1/2 seconds of accelerator's data sampled at 32 Hz, e.g. 16 values,
		// HW reads offloaded in IRQ's thread)
		if (acc_read_async(&accv))
		{
			acc_fifo_push(&fifo, &accv);
			
			if (acc_fifo_get_wpos(&fifo) == 0)
			{
				// get average power (impact)
				acc_fifo_get_average(&fifo, &tilt);
				
				// visualize
				acc_set_pwm_driver2(&tilt, ACC_VAL_CENTER);
			}
		}
				
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

typedef struct object_state_t
{
	int64_t     timestamp;  // seconds
	vector3_s16 impact;     // average power of impacts on x,y,z axis
	vector3_s16 tilt;       // tilt angle on x,y,z axis
	vector3_s16 vibration;  // vibratio on x,y,z axis in Hz
}
object_state_t;

void object_state_init(object_state_t *state)
{
	memset(state, 0, sizeof(*state));
}

typedef void (* ObjectStateCb)(const object_state_t *state);

typedef struct object_state_tracer_t
{
	uint32_t       time_now;  // milliseconds
	uint32_t       time_last; // milliseconds
	object_state_t state_cur; // current state
	ObjectStateCb  logger;    // logger
}
object_state_tracer_t;

void object_tracer_init(object_state_tracer_t *tracer, ObjectStateCb cb)
{
	tracer->time_now  = 0;
	tracer->time_last = 0;
	tracer->logger    = cb;
	
	object_state_init(&tracer->state_cur);
}

static __hw_forceinline void _object_tracer_to_angle(vector3_s16 *v)
{
	v->x = (int32_t)180 * v->x / 0x7FFF;
	v->y = (int32_t)180 * v->y / 0x7FFF;
	v->z = (int32_t)180 * v->z / 0x7FFF;
}

void object_tracer_get_tilt(const acc_fifo_t *fifo, vector3_s16 *tilt)
{	
	acc_fifo_get_average(fifo, tilt);
	_object_tracer_to_angle(tilt);
}

void object_tracer_update(object_state_tracer_t *tracer, const acc_fifo_t fifo[2], uint32_t time_msec)
{
	if (!tracer->logger)
		return;	
	
	tracer->time_now += time_msec;	
	if ((tracer->time_now - tracer->time_last) < 1000)
		return;
	
	{
		acc3_t tilt[2], impact[2], vibration = {0,0,0}, tmp;
	
		tracer->time_last = tracer->time_now;
		
		// snapshot timestamp
		tracer->state_cur.timestamp = tracer->time_last;
		
		// tilt: average angle per whole detection period
		object_tracer_get_tilt(&fifo[0], &tilt[0]);
		object_tracer_get_tilt(&fifo[1], &tilt[1]);		
		tracer->state_cur.tilt = VEC_AVERAGE(tilt[0], tilt[1]);
		
		// impact: diff between of average angles of both detection periods
		impact[0] = tilt[0];
		impact[1] = tilt[1];		
		VEC_SUB(impact[0], impact[1]);	
		tracer->state_cur.impact = impact[0];
		
		// vibration: accumulation of detected differences between successive angles
		for (int32_t t = 0; t < 2; ++t)
		{
			for (int32_t i = 0; i < (ACC_FIFO_SIZE - 1); ++i)
			{
				tmp.x = (fifo[t].buf[i + 1].x - fifo[t].buf[i].x);
				tmp.y = (fifo[t].buf[i + 1].y - fifo[t].buf[i].y);
				tmp.z = (fifo[t].buf[i + 1].z - fifo[t].buf[i].z);
				
				_object_tracer_to_angle(&tmp);
				
				vibration.x += (tmp.x != 0);
				vibration.y += (tmp.y != 0);
				vibration.z += (tmp.z != 0);
			}
		}
		tracer->state_cur.vibration = vibration;
		
		tracer->logger(&tracer->state_cur);
	}
}

void ObjectStateLogger(const object_state_t *state)
{
	printf("t %ds |  tilt[ %d | %d | %d ]  impact[ %d | %d | %d ]  vibration[ %d | %d | %d ]\n", (uint32_t)(state->timestamp / 1000), 
		state->tilt.x, state->tilt.y, state->tilt.z,
		state->impact.x, state->impact.y, state->impact.z,
		state->vibration.x, state->vibration.y, state->vibration.z);
}

void lab3_task_2_abc()
{
	acc3_t accv, tilt_raw;
	uint8_t toggle = 0;
	acc_fifo_t fifo[2];
	object_state_tracer_t tracer;
	
	// init async accelerometer with 32 Hz (ACC_UPDATE_FREQ) update rate
	acc_init(TRUE, ACC_UPDATE_FREQ);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);	
	PWM_HW_TIMER_INIT;
	
	// init fifo buffers (2 x 30 sec each)
	acc_fifo_init(&fifo[0]);
	acc_fifo_init(&fifo[1]);
	
	// init tracer
	object_tracer_init(&tracer, &ObjectStateLogger);
	
	while (1)
	{
		// fill 2 x 30 sec buffers with accelerator's data
		if (acc_read_async(&accv))
		{
			acc_fifo_push(&fifo[toggle], &accv);
			
			// detect buffer's rollover and switch to the next 2nd or process data
			if (acc_fifo_get_wpos(&fifo[toggle]) == 0)
			{
				// visualize (debug with LEDs)
				{
					acc_fifo_get_average(&fifo[toggle], &tilt_raw);					
					acc_set_pwm_driver2(&tilt_raw, ACC_VAL_CENTER);
				}
				
				// switch buffer or update tracer
				if (toggle == 0)
				{
					toggle = 1;
				}
				else
				{
					// assume that 2 buffers were filled within 1000 msec
					object_tracer_update(&tracer, fifo, 1000);
					toggle = 0;
				}
			}
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

typedef struct lab3_task_2_abc_fancy_app
{
	hw_app_base_t base;
	
	PWMFade_t     pwm_fade;
	bool_t        idle;
}
lab3_task_2_abc_fancy_app;

static void AccIdleHandler(void *user_data)
{
	lab3_task_2_abc_fancy_app *ctx = (lab3_task_2_abc_fancy_app *)user_data;
	
	pwm_driver2_update();
	
	if (ctx->idle)
		pwm_driver2_fade_update(&ctx->pwm_fade);
	
	PWM_HW_TIMER_DELAY;
}

void lab3_task_2_abc_fancy()
{
	lab3_task_2_abc_fancy_app app;
	acc3_t tilt_raw;
	uint8_t toggle = 0;
	acc_fifo_t fifo[2];
	object_state_tracer_t tracer;
	
	app.idle = FALSE;
		
	// init async accelerometer with 32 Hz (ACC_UPDATE_FREQ) update rate
	acc_init(TRUE, ACC_UPDATE_FREQ);
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);
	PWM_HW_TIMER_INIT;
	
	// init PWM's fader
	pwm_driver2_fade_init(&app.pwm_fade, TIME_MSEC_TO_USEC(100), 5);
	
	// init fifo buffers (2 x 30 sec each)
	acc_fifo_init(&fifo[0]);
	acc_fifo_init(&fifo[1]);
	
	// init tracer
	object_tracer_init(&tracer, &ObjectStateLogger);
	
	while (1)
	{	
		// fill 2 x 30 sec buffers with accelerator's data
		if (acc_read_buffer_async(&fifo[toggle], &AccIdleHandler, &app))
		{		
			// visualize (debug with LEDs)
			{			
				acc_fifo_get_average(&fifo[toggle], &tilt_raw);
				
				app.idle = (acc_get_pwm_brightness(tilt_raw.x) == 0) && 
				           (acc_get_pwm_brightness(tilt_raw.y) == 0);
				
				if (!app.idle)
					acc_set_pwm_driver2(&tilt_raw, ACC_VAL_CENTER);
			}
			
			// switch buffer or update tracer (assume that 2 buffers were filled within 1000 msec)
			if ((toggle ^= 1) == 0)
				object_tracer_update(&tracer, fifo, 1000);
		}
	}
}

void lab3_task_3()
{
	enum
	{
		DETECT_PERIOD        = TIME_MSEC_TO_USEC(1000),              // time period for temperature detection
		DETECT_AVERAGE_COUNT = (DETECT_PERIOD / PWM_RESOLUTION_USEC) // average count of accumulated temperature values
	};
	
	uint32_t timer = 0;
	int32_t temp_avg = 0;
	float tempf;
	
	// init PWM
	pwm_driver2_init(LED_GREEN, LED_ORANGE, LED_RED, LED_BLUE);
	PWM_HW_TIMER_INIT;
	
	// init Temperature
	TEMP_Enable();
	
	while (1)
	{
		// accumulate temperature value
		temp_avg += TEMP_Read();
		
		if ((timer += PWM_RESOLUTION_USEC) >= DETECT_PERIOD)
		{			
			// use equation to calculate real temp
			tempf = TEMP_ValueToDeg(temp_avg /= DETECT_AVERAGE_COUNT);
			
			printf("temperature: %.04f raw[%d/%d]\n", tempf, temp_avg, DETECT_AVERAGE_COUNT);
			
			// off all leds
			for (int32_t i = 0; i < LED_COUNT; ++i)
				pwm_driver2_set(LED_START + i, 0);
			
			// display
			if (tempf < 20)
				pwm_driver2_set(PWM_CH__BLUE, 100);
			else
			if (tempf > 20 && tempf < 35)
				pwm_driver2_set(PWM_CH__GREEN, 100);
			else
			if (tempf >= 35 && tempf < 45)
				pwm_driver2_set(PWM_CH__ORANGE, 100);
			else
			if (tempf >= 45)
				pwm_driver2_set(PWM_CH__RED, 100);
			
			timer = 0;
			temp_avg = 0;
		}
		
		pwm_driver2_update();
		PWM_HW_TIMER_DELAY;
	}
}

void execute_lab3()
{
	//lab3_task_1_a();
	//lab3_task_1_bcde();
	//lab3_task_1_i();
	//lab3_task_1_j();
	//lab3_task_2();
	//lab3_task_2_abc();
	lab3_task_2_abc_fancy();
	//lab3_task_3();
}
