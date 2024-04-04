/**
  ******************************************************************************
  * @file           : HCSR04_RANGING.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application
  *                   of all US_Sensors.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Reitberger J.
  * All rights reserved.
  *
  *
  ******************************************************************************
  */

#ifndef INC_HCSR04_RANGING_H_
#define INC_HCSR04_RANGING_H_

//#ifdef __cplusplus
//extern "C" {
//#endif

/* Private includes ----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/
#define US_COUNT	2
#define usTIM		TIM4

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim15;

Dev_t deviceUS[US_COUNT];
uint8_t icFlag = 0;
uint8_t captureIdx = 0;
uint32_t edge1Time = 0, edge2Time = 0;
uint32_t startTick;

uint16_t distance_US[US_COUNT];

const float speedOfSound = 0.343/2; // [mm/us]
static const char *USDevStr[] =
{
  [0] 	= 	"FRONT_US",
  [1] 	= 	"BACK_US	",
};

/* Private function prototypes -----------------------------------------------*/
static void GET_US_DATA(void);
static void SET_US_PULSE(uint8_t device);
static void RESET_ALL_US_PIN(void);
static void START_IC_TIMER(TIM_HandleTypeDef *htim, uint32_t Channel);
void usDelay(uint32_t uSec);

/* Private user code ---------------------------------------------------------*/

/**
 *	Funktion sendet einen 10 us Puls auf den Trigger_Pin des US-Sensors
 *	Die Timer messen die Ticks der steigenden und fallenden Flanke der Internal-Clock. (= Distanz)
 *	Flankenzeit * speedofsound/2 = Distance
 *
 */
static void GET_US_DATA(void)
{
	    /* USER CODE END WHILE */
	printf("\n");

	for(int i = 0; i < US_COUNT; i++)
		{
			uint8_t messungen = 0;

			while(messungen < 2)
			{
				RESET_ALL_US_PIN();
				SET_US_PULSE(deviceUS[i]);	//Trigger-Signal 10us

				if(i == 0)
				{
					START_IC_TIMER(&htim5, TIM_CHANNEL_1);
				}
				else
				{
					START_IC_TIMER(&htim15, TIM_CHANNEL_2);
				}

				if(edge2Time > edge1Time)
				{
					distance_US[i] = ((edge2Time - edge1Time) + 0.0f)*speedOfSound;
					printf("%s	-> Distance =	%5d mm\n", USDevStr[i], distance_US[i]);
					messungen++;
					HAL_Delay(25);
				}
				else
				{
					;
				}
			}
		}
		HAL_Delay(25);
}

/**
 * Zählt die Tick zwischen steigende und fellender Flanke
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	uint32_t Channel;

	if(htim == &htim5)
	{
		Channel = TIM_CHANNEL_1;
	}
	else
	{
		Channel = TIM_CHANNEL_2;
	}

	if(captureIdx == 0 ) //First edge
		{
			edge1Time = HAL_TIM_ReadCapturedValue(htim, Channel); //__HAL_TIM_GetCounter;//

			captureIdx = 1;
		}
		else if(captureIdx == 1) //Second edge
		{
			edge2Time = HAL_TIM_ReadCapturedValue(htim, Channel);

			captureIdx = 0;
			icFlag = 1;
		}
}

/**
 * Funktion senden einen 10 us Puls auf den Trigger-Pin der US-Sensoren
 */
static void SET_US_PULSE(uint8_t device)
{
	switch (device)
	{
		case 0:
			//1. Output 10 usec TRIGGER1
			GPIOB->BSRR = (uint32_t)GPIO_PIN_12;	//Pin_Set
			usDelay(10);							//10us Pulse
			GPIOB->BRR = (uint32_t)GPIO_PIN_12;		//Pin_Reset

		case 1:
			//1. Output 10 usec TRIGGER2
			GPIOB->BSRR = (uint32_t)GPIO_PIN_13;
			usDelay(10);
			GPIOB->BRR = (uint32_t)GPIO_PIN_13;
	}
}

/**
 * TRIGGER_PIN Reset
 *
 */
static void RESET_ALL_US_PIN(void)
{
	GPIOB->BRR = (uint32_t)GPIO_PIN_12;	//Pin_Reset
	GPIOB->BRR = (uint32_t)GPIO_PIN_13;	//Pin_Reset
	usDelay(3);
}

/**
 * Funktion startet den Timer und wartet bis icFlag = 1 (fallende Flanke) um Timer zu stoppen
 */
static void START_IC_TIMER(TIM_HandleTypeDef *htim, uint32_t Channel)
{
	//Start IC timer
	HAL_TIM_IC_Start_IT(htim, Channel);

	//Wait for IC flag
	startTick = HAL_GetTick();
	do
	{
		if(icFlag) break;
	}
	while((HAL_GetTick() - startTick) < 500);  //500ms
	icFlag = 0;
	HAL_TIM_IC_Stop_IT(htim, Channel);
}

/**
 *	Funktion stellt einen us Delay über die Internal Clock von TIM4 bereit
 *	us Delay wird für zum senden des Pulses an GPIO Pins der US-Sensoren benötigt
 */
void usDelay(uint32_t uSec)
{
	if(uSec < 2) uSec = 2;
		usTIM->ARR = uSec - 1; 	/*sets the value in the auto-reload register*/
		usTIM->EGR = 1; 		/*Re-initialises the timer*/
		usTIM->SR &= ~1; 		//Resets the flag
		usTIM->CR1 |= 1; 		//Enables the counter
		while((usTIM->SR&0x0001) != 1);
		usTIM->SR &= ~(0x0001);
}

#endif /* INC_HCSR04_RANGING_H_ */
