/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "VL53L4CD_RANGING.h"	// Beinhaltet alle Funktionen für Verwendung des VL53L4CD Sensors	[TOF]
#include "HCSR04_RANGING.h"		// Beinhaltet alle Funktionen für Verwendung des HCSR04 Sensors	 	[US]

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define PUTCHAR int __io_putchar(int ch)	// Set printf to COM3 port || serielle Schnittstelle

enum STATE{
	STOP, DRIVE, PARK
};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart2_rx;
DMA_HandleTypeDef hdma_usart2_tx;

/* USER CODE BEGIN PV */

//Abstaende:
enum STATE state = STOP;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM15_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */

static void TOGGLE_PIN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
// static void SET_PWM_MOTOR(uint8_t value);
// static void SET_PWM_LENKUNG(uint8_t value);
static void ENCODE_SIGN(uint8_t value);
static void START_PARKING(void);
static void CHECK_STATE(void);

static void CHECK_STATE(void);
GPIO_PinState CHECK_BUTTON(void);
static void START_SIGNAL(uint16_t MODE);

// unsigned long SysTickGetTickcount(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM5_Init();
  MX_TIM15_Init();
  MX_TIM4_Init();

  TOF_INIT();
  SET_OFFSET();

  /* USER CODE BEGIN 2 */

  //UART
//  uint8_t tx_buff[10]={11,12,13,14,15,16,17,18,19};
  uint8_t rx_buff[10]={0};
//  uint8_t rx_buff2[10]={0};
  state = STOP;

  HAL_UART_Receive_DMA(&huart1,rx_buff,10);
  HAL_UART_Transmit_DMA(&huart2,"System Startup from Reset State\n",32);
  int UART_Tick = 0;


  //PWM Servo Lenkung
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2); //Clock is at 45MHZ Prescaler 900 Counter Period 1000 =>50Hz for Servo PWM
  rx_buff[8]=0;
  //50Hz==20ms (PA1)

  //PWM Motor Antrieb
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); //Clock is at 45MHZ Prescaler 900 Counter Period 1000 =>50Hz for Servo PWM
  //PA6

  //Encoder
//  uint32_t timer_counter=0;
//  HAL_TIM_Encoder_Start(&htim1,TIM_CHANNEL_1);

  //Hier wird das Neutralsignal für den Fahrtregler ausgegeben.
//  SET_PWM_MOTOR(75); 	// Set zero position of motor driver (TIM3 PWM Motor)
  htim3.Instance->CCR1 = 75;
  HAL_Delay(2000);		// Wait for motor driver to get zero position

  //Callback at USER CODE 4

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  switch (state) {

		case STOP:
//			SET_PWM_LENKUNG(83); //Set Duty Cycle of TIM2 PWM Lenkung
			htim2.Instance->CCR2 = 83;
//			SET_PWM_MOTOR(75); //Set Duty Cycle of TIM3 PWM Motor
			htim3.Instance->CCR1 = 75;
			HAL_Delay(1000);

			CHECK_STATE();	//Checkt den aktuellen Fahrmodus (durch blauen Knopf änderbar) | einmal drücken DRIVE, kurz gedrückt halten PARK

		case DRIVE:
			HAL_UART_Receive_DMA(&huart1,rx_buff,10); //Achtung Uart1 bei L476 TX(D8/PA9) RX(D2/PA10) nach beschriftung entsprechen diese Pins Uart0!
				 if((SysTickGetTickcount()-UART_Tick)>=1000)
				 {

//					timer_counter=TIM1->CNT;
					//HAL_UART_Transmit_DMA(&huart2,&timer_counter,1);

					UART_Tick=SysTickGetTickcount();
//					if(rx_buff[9]==88){HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_5);}
					if(rx_buff[9]==88){TOGGLE_PIN(GPIOA,GPIO_PIN_5);}

				//WICHTIG! im Fehlerfall via Debugger die Positionen der übertagenen UART Werte prüfen im rx_buff!
//					SET_PWM_LENKUNG(rx_buff[3]); //Set Duty Cycle of TIM2 PWM Lenkung
					htim2.Instance->CCR2 = rx_buff[3];
//					SET_PWM_MOTOR(rx_buff[4]); //Set Duty Cycle of TIM3 PWM Motor
					htim3.Instance->CCR1 = rx_buff[4];

					//TODO: Hier Funktion ergänzen die nach Schilderkennung in den passenden Modus wechselt

					int8_t STOP_CAR = 0x01;

					STOP_CAR = CHECK_BUTTON();
					if(STOP_CAR == 0x00)
					{
						printf("\nDRIVE-MODE:	END\n");
						state = STOP;
						HAL_Delay(1500);
					}

					//ENCODE_SIGN(rx_buff[????]);

				 }
			break;

		case PARK:
			START_PARKING();
			break;

		default:
			break;
	}

  }
  /* USER CODE END 3 */
}

/* Private user code ---------------------------------------------------------*/

/* Private function  ---------------------------------------------------------*/
/* USER CODE BEGIN PFP */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){__NOP();} //check if all Data is received

/**
 *	Toggelt Pin zwischen HIGH-LOW
 *
 */
static void TOGGLE_PIN(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	 uint32_t odr;

	  /* Check the parameters */
	  assert_param(IS_GPIO_PIN(GPIO_Pin));

	  /* get current Output Data Register value */
	  odr = GPIOx->ODR;

	  /* Set selected pins that were at low level, and reset ones that were high */
	  GPIOx->BSRR = ((odr & GPIO_Pin) << 16U) | (~odr & GPIO_Pin);
}

/**
 *
 * Steuerung des Motors, value:
 * 75...100 Entspricht 0 bis 100% Geschwindigkeit
 * PWM wird erzeugt mit TIM3
 *
static void SET_PWM_MOTOR(uint8_t value)
{

	htim3.Instance->CCR1 = value;
}
**/

/**
 * Steuerung Lenkung: value:
 *  rechts max = 45, null = 83, links max = 105
 *  PWM wird erzeugt mit TIM2
 *
static void SET_PWM_LENKUNG(uint8_t value)
{
	htim2.Instance->CCR2 = value;
}
**/

/**
 * Auswerten welches Schild erkannt wurde und entsprechend den Modus ändern
 */
static void ENCODE_SIGN(uint8_t value)
{
	switch (value) {
		//Value für Einparkschild
		case 47:
			state = PARK;
			break;
		//Value für Stop:
		case 14:
			state = STOP;
			break;
		default:
			break;
	}
}

/**
 * Funktion die Auto losfahren/parken lässt: Reaktion auf Button Betätigung
 * Blaue Taste einmal drücken = DRIVE, Taste kurz gedrückt halten = PARK
 * LD2 leuchtet solange in STOP-MODE
 *
 * DRIVE-MODE: Bei Start schnelleres blinken der LED
 * PARK-MODE: Bei Start langsameres blinken der LED
*/
static void CHECK_STATE(void)
{
	uint8_t B1_VALUE = 0x01;

	TOGGLE_PIN(GPIOA,GPIO_PIN_5);

	do
	{
		B1_VALUE = CHECK_BUTTON();
	}
	while(B1_VALUE != 0x00U);

	printf("\nDRIVE-MODE:	START\n");
	state = DRIVE;
	HAL_Delay(750);	//Zeit die Taster gedrückt werden muss um in DRIVE zu schalten

	B1_VALUE = CHECK_BUTTON();

	if(B1_VALUE == 0x00U)
	{
		printf("\nPARK-MODE:	START\n");
		state = PARK;
	}

	TOGGLE_PIN(GPIOA,GPIO_PIN_5);

	if(state == DRIVE)
	{
		START_SIGNAL(100);
	}
	else
	{
		START_SIGNAL(300);
	}
}

/**
 * Funktion erkennt ob der Blaue Knopf auf dem STM32 Board gedrückt wurde
 * (PC13)
 */
GPIO_PinState CHECK_BUTTON(void)
{
	GPIO_PinState bitstatus;
	  /* Check the parameters */
	  assert_param(IS_GPIO_PIN(GPIO_PIN_13));

	  if ((GPIOC->IDR & GPIO_PIN_13) != 0x00U)
	  {
	    bitstatus = 0x01U;
	  }
	  else
	  {
	    bitstatus = 0x00U;
	  }
	  return bitstatus;
}

static void START_SIGNAL(uint16_t MODE)
{
	for(int i = 0; i < 5; i++)
	{
		HAL_Delay(MODE);
		TOGGLE_PIN(GPIOA,GPIO_PIN_5);
		HAL_Delay(MODE);
		TOGGLE_PIN(GPIOA,GPIO_PIN_5);

	}
}

/**
 * Einparkalgorythmus V1.1.2
 *
 * distance_TOF[1] = CENTER_LEFT, distance_TOF[2] = FRONTSIDE_LEFT, distance_TOF[3] = FRONTSIDE_CENTER, distance_TOF[4] = FRONTSIDE_RIGHT, distance_TOF[5] = BACKSIDE_CENTER, distance_TOF[6] = BACKSIDE_LEFT
 * distance_US[0] = FRONT_CENTER_US, distance_US[1] = BACK_CENTER_US
 *
 * htim3 = MOTOR, htim2 = LENKUNG
 *
 * MOTOR: Rückwärts 50....75....100 Vorwärts
 * LENKUNG: Rechts 45....83....105 Links
 */
static void START_PARKING(void)
{
	int RESET_LENKUNG = 83U;	//Nullstellung
	int RESET_MOTOR = 75U;		//Nullstellung
	int GW_PARKLÜCKE = 220U, GW_BACK = 375U, GW_PARALLEL_FRONT = 200U, GW_PARALLEL_CENTER = 250U, GW_KORREKTUR = 300U;	//GW = Grenzwert

	htim3.Instance->CCR1 = 83;

	while(1)
	{
	//Fahren bis Lücke erkannt wird
		do
		{
			GET_TOF_DATA();
		}
		while(distance_TOF[2] < GW_PARKLÜCKE || distance_TOF[2] == 0U);	// TOF FRONTSIDE_LEFT

	//Fahren bis Lücke Zuende ist
		do
		{
			GET_TOF_DATA();
		}
		while(distance_TOF[2] > GW_PARKLÜCKE|| distance_TOF[2] == 0U);	//TOF FRONTSIDE_LEFT

	//Fahren bis Auto in Position
		do
		{
			GET_TOF_DATA();
		}
		while(distance_TOF[1] > GW_PARKLÜCKE || distance_TOF[1] == 0U);	//TOF CENTER_LEFT

		htim3.Instance->CCR1 = RESET_MOTOR;

	//Rückwärts und voll links einlenken
		htim2.Instance->CCR2 = 105;
		HAL_Delay(1000);
		htim3.Instance->CCR1 = 67;
		HAL_Delay(1000);
		htim3.Instance->CCR1 = RESET_MOTOR;
		HAL_Delay(1000);
		htim3.Instance->CCR1 = 70;

	//Rückwärts bis BACK_CENTER_US Mindestabstand erkannt, dann voll rechts einschlagen
		do
		{
			GET_TOF_DATA();
		}
		while(distance_TOF[6] > GW_BACK || distance_TOF[6] == 0U);	//TOF BACKSIDE_LEFT

		htim3.Instance->CCR1 = RESET_MOTOR;
		htim2.Instance->CCR2 = RESET_LENKUNG;
		HAL_Delay(1000);
		htim2.Instance->CCR2 = 45;
		HAL_Delay(1000);
		htim3.Instance->CCR1 = 70;

		//Rückwärts bis Auto wieder paralell ist
			while(1)
			{
				GET_TOF_DATA();

				if (distance_TOF[1] < GW_PARALLEL_CENTER && distance_TOF[2] < GW_PARALLEL_FRONT)	//TOF CENTER_LEFT | TOF FRONTSIDE_LEFT
				{
					htim3.Instance->CCR1 = RESET_MOTOR;
					htim2.Instance->CCR2 = RESET_LENKUNG;
					HAL_Delay(1000);
					break;
				}
			}

		htim3.Instance->CCR1 = 82;

	//Geradeaus bis Auto korrekt steht
		do
		{
			GET_TOF_DATA();
		}
		while(distance_TOF[3] > GW_KORREKTUR|| distance_TOF[3] == 0U);	//TOF FRONTSIDE_CENTER

		htim3.Instance->CCR1 = RESET_MOTOR;
		state = STOP;
		break;

	}

}

/**
 *	Funktion dient zur printf Ausgabe auf der seriellen Schnittstelle
 */
	PUTCHAR
	{
		/* e.g. write a character to the USART2 and Loop until the end of transmission */
		HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);

		return ch;
	}

/* USER CODE END PFP */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 25;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00200B2B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 255;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI1;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 10;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 899;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 899;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */
  HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);					///*******MODIFIED

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 50-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 0;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 50-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 10000;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 4;
  if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(void)
{

  /* USER CODE BEGIN TIM15_Init 0 */

  /* USER CODE END TIM15_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 50-1;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 10000;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim15, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 4;
  if (HAL_TIM_IC_ConfigChannel(&htim15, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA1_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
  /* DMA1_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, CENTER_LEFT_TOF_Pin|FRONT_LEFT_TOF_Pin|FRONT_RIGHT_TOF_Pin|BACK_LEFT_TOF_Pin
                          |BACK_RIGHT_TOF_Pin|CENTER_RIGHT_TOF_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TRIGGER_FRONT_US_Pin|TRIGGER_BACK_US_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_BLUE_Pin */
  GPIO_InitStruct.Pin = B1_BLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_BLUE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CENTER_LEFT_TOF_Pin FRONT_LEFT_TOF_Pin FRONT_RIGHT_TOF_Pin BACK_LEFT_TOF_Pin
                           BACK_RIGHT_TOF_Pin CENTER_RIGHT_TOF_Pin */
  GPIO_InitStruct.Pin = CENTER_LEFT_TOF_Pin|FRONT_LEFT_TOF_Pin|FRONT_RIGHT_TOF_Pin|BACK_LEFT_TOF_Pin
                          |BACK_RIGHT_TOF_Pin|CENTER_RIGHT_TOF_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : TRIGGER_FRONT_US_Pin TRIGGER_BACK_US_Pin */
  GPIO_InitStruct.Pin = TRIGGER_FRONT_US_Pin|TRIGGER_BACK_US_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
