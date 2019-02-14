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

#include "stm32f4xx_tim.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_adc.c" // hack: source amalgamation
#include "core_cm4.h"
#include "hw_helper.h"

static uint32_t g_TMR4_ResolutionUsec = 0;
static TMR_ISR_Callback g_TMR_Callback[TMR_ISR_MAX_ID];

void TMR_SW_DelayUSec(volatile uint32_t usec)
{
	static const uint32_t freq = 168 * 1000 * 1000;
	usec *= (freq / (1000 * 1000)) / 8; // 3 instructions, 4 cyles per 1 instruction (speculate = 8 cycles per 3 ins)
	
	while (usec--)
	{ }
}

void TMR_SW_DelayMSec(volatile uint32_t msec)
{
	static const uint32_t freq = 168 * 1000 * 1000;
	msec *= (freq / 1000) / 8; // 3 instructions, 4 cyles per 1 instruction (speculate = 8 cycles per 3 ins)
	
	while (msec--)
	{ }
}

void TMR_HW_DelayUSec(volatile uint32_t usec)
{
	uint32_t chunks = usec / g_TMR4_ResolutionUsec;
	
	while (chunks--)
	{
		TMR4_WaitForExpiry();
	}
}

void TMR_HW_DelayMSec(volatile uint32_t msec)
{
	uint32_t chunks = msec * 1000 / g_TMR4_ResolutionUsec;
	
	while (chunks--)
	{
		TMR4_WaitForExpiry();
	}
}

// Initialize Timer 4
void TMR4_Init(uint16_t prescaler, uint16_t period)
{
	// copyright notice: below implementation is sourced from the Lab's handouts
	
	TIM_TimeBaseInitTypeDef TIM_BaseStruct;
	/* Enable clock for TIM4 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	// timer_tick_frequency = 84000000 / (0 + 1) = 84000000
	TIM_BaseStruct.TIM_Prescaler = prescaler;
	/* Count up */
	TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	/*
	Set timer period when it must reset
	First you have to know max value for timer
	In our case it is 16bit = 65535
	Frequency = timer_tick_frequency / (TIM_Period + 1)
	If you get TIM_Period larger than max timer value (in our case 65535),
	you have to choose larger prescaler and slow down timer tick frequency
	*/
	TIM_BaseStruct.TIM_Period = period; // <your value here>
	TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_BaseStruct.TIM_RepetitionCounter = 0;
	/* Initialize TIM4 */
	TIM_TimeBaseInit(TIM4, &TIM_BaseStruct);
	/* Start count on TIM4 */
	TIM_Cmd(TIM4, ENABLE);
}

// Loops until the timer has expired
void TMR4_WaitForExpiry(void)
{
	// copyright notice: below implementation is sourced from the Lab's handouts
	
	// Check the flag. When the timer is expired, the flag is SET.
	while(TIM_GetFlagStatus(TIM4, TIM_FLAG_Update) == RESET)
	{
	}
	// Reset flag for next expiry
	TIM_ClearFlag(TIM4, TIM_IT_Update);
}

void TMR_GetPrescalerAndPeriod(uint32_t usec, uint32_t *_prescaler, uint32_t *_period)
{
#define TMR_PERIOD(FREQ, TIME_USEC) (uint32_t)(((uint64_t)FREQ * TIME_USEC) / (1000 * 1000))	
	
	uint32_t prescaler = 0, period;
	
	period = TMR_PERIOD(TMR4_FREQUENCY, usec);
	if (period > 0xFFFF)
	{
		uint32_t freq;
		for (uint32_t i = 1; (i < 16) && (period > 0xFFFF); ++i)
		{
			prescaler = (1 << i);
			freq      = TMR4_FREQUENCY / prescaler;
			period    = TMR_PERIOD(freq, usec);
		}			
	}
	
	(*_prescaler) = (prescaler ? prescaler - 1 : prescaler);
	(*_period)    = (period ? period - 1 : period);
	
#undef TMR_PERIOD
}

void TMR4_InitResolutionUsec(uint32_t usec)
{
	uint32_t prescaler, period;
	TMR_GetPrescalerAndPeriod(usec, &prescaler, &period);
		
	TMR4_Init(prescaler, period);
	
	g_TMR4_ResolutionUsec = usec;
}

// Initialize Timer 4 for interrupts
bool_t TMR_Init_ISR(ETmrIsrId id, uint16_t prescaler, uint16_t period, TMR_ISR_Callback cb)
{
	// copyright notice: below implementation is sourced from the Lab's handouts
	
	TIM_TypeDef *TIMx;
	switch (id)
	{
	case TMR_ISR__3: TIMx = TIM3; break;
	case TMR_ISR__4: TIMx = TIM4; break;
	default:
		return FALSE;
	}
	
	g_TMR_Callback[id] = cb;
	
	// Setup the nested vector interrupt controller
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_BaseStruct;
	/* Enable the TIMx gloabal Interrupt */
	switch (id)
	{
	case TMR_ISR__3: NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; break;
	case TMR_ISR__4: NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn; break;
	}
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Enable clock for TIMx */
	switch (id)
	{
	case TMR_ISR__3: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); break;
	case TMR_ISR__4: RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); break;
	}
	/*
	timer_tick_frequency = 84000000 / (prescaler + 1) = 84000000
	*/
	TIM_BaseStruct.TIM_Prescaler = prescaler;
	/* Count up */
	TIM_BaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	/*
	Set timer period when it must reset
	First you have to know max value for timer
	In our case it is 16bit = 65535
	Frequency = timer_tick_frequency / (TIM_Period + 1)
	If you get TIM_Period larger than max timer value (in our case 65535),
	you have to choose larger prescaler and slow down timer tick frequency
	*/
	TIM_BaseStruct.TIM_Period = period; // <your value>
	TIM_BaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_BaseStruct.TIM_RepetitionCounter = 0;
	/* Initialize TIM4 */
	TIM_TimeBaseInit(TIMx, &TIM_BaseStruct);
	/* TIM Interrupt enable */
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
	/* Start count on TIM4 */
	TIM_Cmd(TIMx, ENABLE);
	
	return TRUE;
}

void TMR_Init_ISR_ResolutionUsec(ETmrIsrId id, uint32_t usec, TMR_ISR_Callback cb)
{
	uint32_t prescaler, period;
	TMR_GetPrescalerAndPeriod(usec, &prescaler, &period);
			
	TMR_Init_ISR(id, prescaler, period, cb);
	
	if (id == TMR_ISR__4)
		g_TMR4_ResolutionUsec = usec;
}

// This is triggered when the counter overflows
void TIM4_IRQHandler(void)
{
	// copyright notice: below implementation is sourced from the Lab's handouts
	
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
	
		if (g_TMR_Callback[TMR_ISR__4])
			g_TMR_Callback[TMR_ISR__4]();
	}
}

// This is triggered when the counter overflows
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	
		if (g_TMR_Callback[TMR_ISR__3])
			g_TMR_Callback[TMR_ISR__3]();
	}
}

// printf() support
int itm_debug(int ch)
{
	return ITM_SendChar(ch);
}
int fputc(int ch, FILE *f) 
{
    return itm_debug(ch);
}

void TEMP_Enable()
{
	// copyright notice: below implementation is sourced from the Lab's handouts
	
	/**********************************************************************************
	* This enables the A/D converter for sampling the on board
	* temperature sensor.
	* You must first enable the CommonInitStructure
	* then enable the specific InitStructure for AD1, AD2 or AD3
	* Review reference manual RM0090 for register details.
	* **********************************************************************************/
	ADC_InitTypeDef ADC_InitStruct;
	ADC_CommonInitTypeDef ADC_CommonInitStruct;
	ADC_DeInit();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	//
	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStruct);
	//
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_CC1;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_Init(ADC1, &ADC_InitStruct);
	// ADC1 Configuration, ADC_Channel_TempSensor is actual channel 16
	ADC_RegularChannelConfig(ADC1, ADC_Channel_TempSensor, 1,
	ADC_SampleTime_144Cycles);
	// Enable internal temperature sensor
	ADC_TempSensorVrefintCmd(ENABLE);
	// Enable ADC conversion
	ADC_Cmd(ADC1, ENABLE);
}

uint32_t TEMP_Read()
{
	// copyright notice: below implementation is sourced from the Lab's handouts
	
	uint32_t temp_value;
	ADC_SoftwareStartConv(ADC1); //Start the conversion
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET); //Processing the conversion
	temp_value = ADC_GetConversionValue(ADC1); //Return the converted dat
	return temp_value;
}

float TEMP_ValueToDeg(uint32_t sens)
{
	static const float to_V  = 3.3f / 4095;             // max resolution: V per step
	static const float slope = 2.5f / 1000;             // slope in V
	static const float v_cal = 1060.0f * (3.3f / 4095); // 26 deg == 1060
	static const float t_cal = 26;                      // calibration T
		
	float temp = (sens * to_V - v_cal) / slope + t_cal;

	return temp;
}
