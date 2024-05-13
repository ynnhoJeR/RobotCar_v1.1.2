/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : VL53L4CD_RANGING.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application
  *                   of all TOF_Sensors.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 Reitberger J.
  * All rights reserved.
  *
  *
  ******************************************************************************
  */

#ifndef INC_VL53L4CD_RANGING_H_
#define INC_VL53L4CD_RANGING_H_

//#ifdef __cplusplus
//extern "C" {
//#endif

/* Private includes ----------------------------------------------------------*/
#include "VL53L4CD_api.h" //beinhaltet Funktionen, die zum Initialisieren und Auslesen des Sensors benötigt werden

/* Private define ------------------------------------------------------------*/
#define TOF_COUNT	7U
//#define TOF_COUNT	9U

/* Private variables ---------------------------------------------------------*/
Dev_t deviceTOF[TOF_COUNT];
uint8_t status[TOF_COUNT],loop,isReady[TOF_COUNT];
uint16_t sensor_id[TOF_COUNT];
VL53L4CD_ResultsData_t result[TOF_COUNT];
uint16_t distance_TOF[TOF_COUNT];

static const char *TofDevStr[] =
{
  [1] 	= 	"CENTER_LEFT",
  [2] 	= 	"FRONTSIDE_LEFT",
  [3] 	= 	"FRONTSIDE_CENTER",
  [4] 	= 	"FRONTSIDE_RIGHT",
  [5] 	= 	"BACKSIDE_CENTER",
  [6] 	= 	"BACKSIDE_LEFT",
//  [7] 	= 	"CENTER_RIGHT",
//  [8] 	= 	"BACKSIDE_RIGHT"

};

/* Private function prototypes -----------------------------------------------*/
static void GET_TOF_DATA(uint8_t SensorNR);
static void TOF_INIT(void);
static void SET_TOF_PIN(uint8_t device);
static void RESET_ALL_TOF_SEN(void);
static void SET_OFFSET(void);

void usDelay(uint32_t uSec);

/* Private user code ---------------------------------------------------------*/

/**
 * Auswertung der TOF-Sensoren
 *
 */
static void GET_TOF_DATA(uint8_t SensorNR)
{
	uint32_t delayTOF = 10;
	printf("\n");

		status[SensorNR] = VL53L4CD_StartRanging(deviceTOF[SensorNR]);

		if(status[SensorNR] == 0)
		{
			uint8_t messungen = 0;
			// Jeden Messung zwei mal um Genauigkeit zu erhöhen
			while(messungen < 2)
			{
				//Polling um zu pruefen ob eine neue Messung abgeschlossen ist
					HAL_Delay(delayTOF);
					status[SensorNR] = VL53L4CD_CheckForDataReady(deviceTOF[SensorNR], &isReady[SensorNR]);

					if(isReady[SensorNR])
					{
						HAL_Delay(delayTOF);

						//Hardwareinterrupt des Sensors löschen, sonst kann keine weitere Messung erfolgen
						VL53L4CD_ClearInterrupt(deviceTOF[SensorNR]);

						//Entfernung auslesen
						//Die Entfernung wird immer direkt nach dem auslesen wieder gespeichert!
						HAL_Delay(delayTOF);
						VL53L4CD_GetResult(deviceTOF[SensorNR], &result[SensorNR]);
						if(result[SensorNR].range_status == 0)
						{
							distance_TOF[SensorNR] = result[SensorNR].distance_mm;
							printf("%s	-> Distance = %5d mm\n",TofDevStr[SensorNR], distance_TOF[SensorNR]);
						}
					}
					messungen++;
					WaitMs(deviceTOF[SensorNR], delayTOF);
			}
		}
		HAL_Delay(delayTOF);
		status[SensorNR] = VL53L4CD_StopRanging(deviceTOF[SensorNR]);
}

/**
 *
 * Initialisierung der TOF-Sensoren (I2C-Adressen)
 *
 * Die Initialisierung funktioniert.
 * Allerdings wird der Sensor 0 (0x52) zwar richtig initialisiert und eine ID zugewiesen, dennoch kann die erste I2C Adresse nicht genutzt werden, da es sonst zu Problemen bei folgenden Sensoren kommt.
 * Lösung: Variable TOF_COUNT um eins erhöhen um bei 6 Sensoren 7 I2C Adressen zu generieren, dabei die erste nicht nutzen und auswertnen. Sensoren belgen den Bus auf Device[1-7], Device [0] wird nicht genutzt.
 *
 */
static void TOF_INIT(void)
{
	 uint8_t i;

	RESET_ALL_TOF_SEN();

	  for (i = 0; i < TOF_COUNT; i++)
	  {
		  SET_TOF_PIN(i);

		  Dev_t i2cAddr = 0x52;		// !!! Wichtig !!! defaultAdress nicht ändern, führt zu I2C Problemen
		  deviceTOF[i]  = (i2cAddr + i*2);

		  // Setzen der neuen I2C Adressen und auslesen der Sensor ID (0xEBAA)
		  VL53L4CD_SetI2CAddress(i2cAddr, deviceTOF[i]);
		  VL53L4CD_GetSensorId(deviceTOF[i], &sensor_id[i]);

		  if (deviceTOF[i] != 0x52)
		  {
			  printf("Init [ToF: %d]: Device -> %s 	ID: %04lX\n", deviceTOF[i], TofDevStr[i], (unsigned long)sensor_id[i]);
		  }

		  if((status[i] || (sensor_id[i] != 0xEBAA)) && (i != 0))
		  	{
		  		printf("VL53L4CD not detected at requested address\n");
		  	}

		  //Sensor initialisieren
		  if (deviceTOF[i] != 0x52)
		  		  {
			  	  	  status[i] = VL53L4CD_SensorInit(deviceTOF[i]);
		  		  }

		  	if(status[i])
		  	{
		  		printf("VL53L4CD ULD Loading failed\n");
		  	}
	  	}

	  	printf("\n");
	  	printf("VL53L4CD: Ultra Light Driver ready!\n");
}

/**
 *	PIN Set der ToF Sensoren (jeden Sonsor einzeln)
 *	Set der Pins über Register (ohne HAL-Funktion)
 */
static void SET_TOF_PIN(uint8_t device)
{
	switch (device)
	{
		case 0:	//CENTER_LEFT		(PC04)
			GPIOC->BSRR = (uint32_t)GPIO_PIN_4;
			break;
		case 1:	//FRONTSIDE_LEFT	(PC05)
			GPIOC->BSRR = (uint32_t)GPIO_PIN_5;
			break;
		case 2:	//FRONTSIDE_CENTER	(PC06)
			GPIOC->BSRR = (uint32_t)GPIO_PIN_6;
			break;
		case 3:	//FRONTSIDE_RIGHT	(PC07)
			GPIOC->BSRR = (uint32_t)GPIO_PIN_7;
			break;
		case 4:	//BACKSIDE_CENTER 	(PC08)
			GPIOC->BSRR = (uint32_t)GPIO_PIN_8;
			break;
		case 5:	//BACKSIDE_LEFT 	(PC09)
			GPIOC->BSRR = (uint32_t)GPIO_PIN_9;
			break;
//		case 6:	//CENTER_RIGHT 		(PC10)
//			GPIOC->BSRR = (uint32_t)GPIO_PIN_10;
//			break;
//		case 7:	//BACKSIDE_RIGHT 	(PC11)
//			GPIOC->BSRR = (uint32_t)GPIO_PIN_11;
//			break;

		default:
			break;
	}
	usDelay(3);
}

/**
 *	PIN Reset aller ToF Sensoren (GPIOC)
 *	Reset der Pins über Register (ohne HAL-Funktion)
 */
static void RESET_ALL_TOF_SEN(void)
{
	  GPIOC->BRR = (uint32_t)GPIO_PIN_4;
	  GPIOC->BRR = (uint32_t)GPIO_PIN_5;
	  GPIOC->BRR = (uint32_t)GPIO_PIN_6;
	  GPIOC->BRR = (uint32_t)GPIO_PIN_7;
	  GPIOC->BRR = (uint32_t)GPIO_PIN_8;
	  GPIOC->BRR = (uint32_t)GPIO_PIN_9;
//	  GPIOC->BRR = (uint32_t)GPIO_PIN_10;
//	  GPIOC->BRR = (uint32_t)GPIO_PIN_11;
	  usDelay(3);
}

/**
 * Kalibrierfunktion: Offsetvalues der Sensoren
 * Das Offsetvalue korrigiert die Distanz zwischen gemessenem und angezeigtem Wert
 *
 * Falls eine Glasabdeckung für die Sensoren benutzt wird, muss zusätzlich eine Crosstalk kalibrierung erfolgen. Glas reflektiert eventuell Licht, was zu falschen Messungen führen kann
 *
 */
static void SET_OFFSET(void)
{
	int16_t offsetvalue;

	for (int i = 1; i < TOF_COUNT; i++)	// i = 1 um die erste I2C Adresse zu überspringen, da keiner Sensor zugewiesen
	{
		switch (i) /* Offsetparameter in mm für jeden Sensor */
		{
			case 1:	//CENTER_LEFT		(PC04)
				offsetvalue = -10;
				break;
			case 2:	//FRONTSIDE_LEFT	(PC05)
				offsetvalue = -12;
				break;
			case 3:	//FRONTSIDE_RIGHT	(PC06)
				offsetvalue = -10;
				break;
			case 4:	//FRONTSIDE_RIGHT	(PC07)
				offsetvalue = -10;
				break;
			case 5:	//BACKSIDE_CENTER	(PC08)
				offsetvalue = -8;
				break;
			case 6:	//BACKSIDE_LEFT		(PC09)
				offsetvalue = -15;
				break;
//			case 7:	//CENTER_RIGHT		(PC10)
//				offsetvalue = -10;
//				break;
//			case 8:	//BACKSIDE_RIGHT	(PC11)
//				offsetvalue = -8;
//				break;

			default:
				break;
		}

		VL53L4CD_SetOffset(deviceTOF[i], offsetvalue);
		usDelay(3);
	}
}

#endif /* INC_HC_SR04_RANGING_H_ */
