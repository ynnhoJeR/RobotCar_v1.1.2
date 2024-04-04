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

/* Private variables ---------------------------------------------------------*/
Dev_t deviceTOF[TOF_COUNT];
uint8_t status[TOF_COUNT],loop,isReady[TOF_COUNT];
uint16_t sensor_id[TOF_COUNT];
VL53L4CD_ResultsData_t result[TOF_COUNT];
uint16_t distance_TOF[TOF_COUNT];

static const char *TofDevStr[] =
{
  [1] 	= 	"CENTER_LEFT",
  [2] 	= 	"FRONT_LEFT",
  [3] 	= 	"FRONT_RIGHT",
  [4] 	= 	"BACK_LEFT",
  [5] 	= 	"BACK_RIGHT",
  [6] 	= 	"CENTER_RIGHT"

};

/* Private function prototypes -----------------------------------------------*/
static void GET_TOF_DATA(void);
static void TOF_INIT(void);
static void SET_TOF_PIN(uint8_t device);
static void RESET_ALL_TOF_SEN(void);
static void SET_OFFSET(void);

void usDelay(uint32_t uSec);

/* Private user code ---------------------------------------------------------*/

/**
 * Auswertung der TOF-Sensoren
 */
static void GET_TOF_DATA(void)
{
	uint32_t delayTOF = 10;
	printf("\n");

	for(int i = 1; i < TOF_COUNT; i++)		// i = 1 um die erste I2C Adresse zu überspringen, da keiner Sensor vorhanden
	{

		status[i] = VL53L4CD_StartRanging(deviceTOF[i]);

		if(status[i] == 0)
		{
			uint8_t messungen = 0;
			// Jeden Messung zwei mal um Genauigkeit zu erhöhen
			while(messungen < 2)
			{
				//Polling um zu pruefen ob eine neue Messung abgeschlossen ist
					HAL_Delay(delayTOF);
					status[i] = VL53L4CD_CheckForDataReady(deviceTOF[i], &isReady[i]);

					if(isReady[i])
					{
						HAL_Delay(delayTOF);

						//Hardwareinterrupt des Sensors löschen, sonst kann keine weitere Messung erfolgen
						VL53L4CD_ClearInterrupt(deviceTOF[i]);

						//Entfernung auslesen
						//Die Entfernung wird immer direkt nach dem auslesen wieder gespeichert!
						HAL_Delay(delayTOF);
						VL53L4CD_GetResult(deviceTOF[i], &result[i]);
						if(result[i].range_status == 0)
						{
							distance_TOF[i] = result[i].distance_mm;
							printf("%s	-> Distance = %5d mm\n",TofDevStr[i], distance_TOF[i]);
						}
					}
					messungen++;
					WaitMs(deviceTOF[i], delayTOF);
			}
		}
		HAL_Delay(delayTOF);
		status[i] = VL53L4CD_StopRanging(deviceTOF[i]);
	}
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
		case 0:
			GPIOC->BSRR = (uint32_t)GPIO_PIN_4;
			break;
		case 1:
			GPIOC->BSRR = (uint32_t)GPIO_PIN_5;
			break;
		case 2:
			GPIOC->BSRR = (uint32_t)GPIO_PIN_6;
			break;
		case 3:
			GPIOC->BSRR = (uint32_t)GPIO_PIN_7;
			break;
		case 4:
			GPIOC->BSRR = (uint32_t)GPIO_PIN_8;
			break;
		case 5:
			GPIOC->BSRR = (uint32_t)GPIO_PIN_9;
			break;

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
			case 1:	//CENTER_LEFT	(PC04)
				offsetvalue = -10;
				break;
			case 2:	//FRONT_LEFT	(PC05)
				offsetvalue = -12;
				break;
			case 3:	//FRONT_RIGHT	(PC06)
				offsetvalue = -10;
				break;
			case 4:	//BACK_LEFT		(PC07)
				offsetvalue = -10;
				break;
			case 5:	//BACK_RIGHT	(PC08)
				offsetvalue = -8;
				break;
			case 6:	//CENTER_RIGHT	(PC09)
				offsetvalue = -15;
				break;
		}

		VL53L4CD_SetOffset(deviceTOF[i], offsetvalue);
		usDelay(3);
	}
}

#endif /* INC_HC_SR04_RANGING_H_ */
