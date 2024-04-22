/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
	DIST_FL, DIST_FR, DIST_BR, DIST_BL
} dist_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* USER CODE BEGIN PV */
uint16_t dis_FL;
uint16_t dis_FR;
uint16_t dis_BL;
uint16_t dis_BR;
/* USER CODE END PV */



/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM4_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t ir_dists_raw[5] = { 0, 0, 0, 0, 0 };
uint16_t ir_dists_norm[5] = { 0, 0, 0, 0, 0 };

//Nominal value is our "between what X1-X2 should the mouse be between before re-adjusting"
const uint16_t BL_nominal = 100;
const uint16_t BR_nominal = 100;
const uint16_t FL_nominal = 200;
const uint16_t FR_nominal = 200;

//Calibration value is the perfect/ideal value that we would want the mouse to be
//BL_calib = 210 would mean that the mouse should ideally be 210 units away from left wall
//Values depend on individual mice and should be tested beforehand (watch video)
const uint16_t BL_calib = 210;
const uint16_t BR_calib = 210;
const uint16_t FL_calib = 1900;
const uint16_t FR_calib = 1900;

float BL_scale = (float) BL_nominal / BL_calib;
float BR_scale = (float) BR_nominal / BR_calib;
float FL_scale = (float) FL_nominal / FL_calib;
float FR_scale = (float) FR_nominal / FR_calib;

uint16_t motorRightSpeed;
uint16_t motorLeftSpeed;

bool lWallpresent = 0;
bool rWallpresent = 0;
bool fWallpresent = 0;

uint16_t enc_left = 0;
uint16_t enc_right = 0;

//Allows for reading of encoder values
void HAL_TIM_CaptureCallback(TIM_HandleTypeDef *htim) {
	// this is the left and right encoder timer
	if (htim->Instance == TIM3) {
		enc_left = __HAL_TIM_GET_COUNTER(htim);
	}
	if (htim->Instance == TIM4) {
		enc_right = __HAL_TIM_GET_COUNTER(htim);
	}
}

static void ADC1_Select_CH4(void) {
	ADC_ChannelConfTypeDef sConfig = { 0 };

	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
}

static void ADC1_Select_CH9(void) {
	ADC_ChannelConfTypeDef sConfig = { 0 };

	sConfig.Channel = ADC_CHANNEL_9;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
}

static void ADC1_Select_CH5(void) {
	ADC_ChannelConfTypeDef sConfig = { 0 };

	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
}

static void ADC1_Select_CH8(void) {
	ADC_ChannelConfTypeDef sConfig = { 0 };

	sConfig.Channel = ADC_CHANNEL_8;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_7CYCLES_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
}

uint16_t measure_dist(uint16_t dist_t) {
	GPIO_TypeDef *emitter_port;
	uint16_t emitter_pin;
	GPIO_TypeDef *receiver_port;
	uint16_t receiver_pin;

	switch (dist_t) {
	case DIST_FL:
		emitter_port = EMIT_FL_GPIO_Port;
		emitter_pin = EMIT_FL_Pin;
		receiver_port = RECIV_FL_GPIO_Port;
		receiver_pin = RECIV_FL_Pin;
		ADC1_Select_CH9();
		break;
	case DIST_FR:
		emitter_port = GPIOB;
		emitter_pin = GPIO_PIN_5;
		receiver_port = GPIOA;
		receiver_pin = GPIO_PIN_4;
		ADC1_Select_CH4();
		break;
	case DIST_BL:
		emitter_port = GPIOB;
		emitter_pin = GPIO_PIN_11;
		receiver_port = GPIOB;
		receiver_pin = GPIO_PIN_0;
		ADC1_Select_CH8();
		break;
	case DIST_BR:
		emitter_port = GPIOB;
		emitter_pin = GPIO_PIN_10;
		receiver_port = GPIOA;
		receiver_pin = GPIO_PIN_5;
		ADC1_Select_CH5();
		break;
	default:
		break;
	}
	//Turns on IR Emitter
	HAL_GPIO_WritePin(emitter_port, emitter_pin, GPIO_PIN_SET);
	HAL_Delay(5);

	//Starts ADC Conversion
	HAL_ADC_Start(&hadc1);
	//Checks if conversion is complete
	HAL_ADC_PollForConversion(&hadc1, 10);
	//Reads ADC Value
	uint16_t adc_val = HAL_ADC_GetValue(&hadc1);
	//Stops ADC Conversion
	HAL_ADC_Stop(&hadc1);

	//Turns off IR Emitter
	HAL_GPIO_WritePin(emitter_port, emitter_pin, GPIO_PIN_RESET);

	//ir_dists_raw[(uint8_t)ir_counter] = adc_val;

	return adc_val;
}

uint16_t updateIR(uint16_t rawValues) {
	int pollSumFR = 0;
	int pollSumFL = 0;
	int pollSumBR = 0;
	int pollSumBL = 0;

	int pollAvgFR = 0;
	int pollAvgFL = 0;
	int pollAvgBR = 0;
	int pollAvgBL = 0;

	float err = 0;
	//float left_err = 0;
	//float right_err = 0;
	int target = 0;
	int position = 0;

	for (int i = 0; i < 5; i++) {
		dis_FR = measure_dist(DIST_FR);
		dis_FL = measure_dist(DIST_FL);
		dis_BR = measure_dist(DIST_BR);
		dis_BL = measure_dist(DIST_BL);

		//include the subtraction portion?
		pollSumFR -= dis_FR;
		pollSumFR += dis_FR(FR_scale);
		pollSumFL -= dis_FL;
		pollSumFL += dis_FL(FL_scale);
		pollSumBR -= dis_BR;
		pollSumBR += dis_BR(BR_scale);
		pollSumBL -= dis_BL;
		pollSumBL += dis_BL(BL_scale);

	}
	pollAvgFR = pollSumFR / 5;
	pollAvgFL = pollSumFL / 5;
	pollAvgBR = pollSumBR / 5;
	pollAvgBL = pollSumBL / 5;

	ir_dists_norm[0] = pollAvgFR;
	ir_dists_norm[1] = pollAvgFL;
	ir_dists_norm[2] = pollAvgBR;
	ir_dists_norm[3] = pollAvgBL;

	//Save these for if the mouse actually detects a wall (i.e. this is not PID Control)
	lWallpresent = ir_dists_norm[3] > 50;
	rWallpresent = ir_dists_norm[2] > 50;
	fWallpresent = ir_dists_norm[0] > 50 && ir_dists_norm[1] > 50;

	//CHANGE THIS CODE SO lWallpresent is just the ir_dists_nom value?
	/*if(lWallpresent > 60 && (rWallpresent < 30 && rWallpresent > 10)){
		left_err = BL_nominal - ir_dist_norm[3];
	}
	if(rWallpresent > 60 && (lWallpresent < 30 && lWallpresent > 10)){
		right_err = BR_nominal - ir_dist_norm[2];
	}
	if(fWallpresent > 60){
		err = FR_nominal - ir_dist_norm[0];
	}*/

	if(ir_dist_norm[3] > 60 && (ir_dist_norm[2] < 30 && ir_dist_norm[2] > 10)){
		position = ir_dists_norm[3];
	}
	if(ir_dist_norm[2] > 60 && (ir_dist_norm[3] < 30 && ir_dist_norm[3] > 10)){
		position = -(ir_dists_norm[2]);
	}

	err = target - position;
	position += err * 0.5;

	return ir_dists_norm;
	//return array of avg polled valued
}

void AddWall() //Adds cell wall in pos coord + direction. A cell wall is also added to the neighbor facing in the same direction as the mouse but in the opposite direction
{
    /*maze.cellWalls[pos.y][pos.x] |= dir_mask[direction];
    Coord neighbor = FindNeighborCoord(pos, direction);
    if (neighbor.x>=0 && neighbor.x<16 && neighbor.y>=0 && neighbor.y<16)
        maze.cellWalls[neighbor.y][neighbor.x] |= dir_mask[(direction + 2) % 4];*/
}

bool ScanWalls() //Scans wall around mouse after every iteration and adds walls based on if a wall is scanned in front, to the right, or to the left of the mouse. The function also returns if any walls were found (true/false)
{
    bool found = false;

    if (fWallpresent & err < 10) {
        //AddWall(maze, maze.mouse_pos, maze.mouse_dir);
        found = true;
    }
    if (rWallpresent & right_err < 10) {
        //AddWall(maze, maze.mouse_pos, (Direction)((maze.mouse_dir + 1) % 4));
        found = true;
    }
    if (lWallpresent & left_err < 10) {
        //AddWall(maze, maze.mouse_pos, (Direction)((maze.mouse_dir + 3) % 4));
        found = true;
    }

    return found;
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */

	//first parameter is encoder handle interface?
	//second parameter is source channel for which first parameter will receive ticks
	//parameters might be switched though
	HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_2);
	HAL_TIM_Encoder_Start_IT(&htim4, TIM_CHANNEL_2);

	//Sets FR and FL motor to ON
	//Sets BR and BL motor to OFF
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1);

	//Sets timer frequencies (1000/2047 -> ~50% duty cycle / half motor speed)
	//See if you can initialize motor speeds like this?
	//Also see if you initialized it the right way (i.e. motorRightSpeed not with left motor)
	motorRightSpeed = TIM2->CCR3;
	motorLeftSpeed = TIM2->CCR4;

	motorRightSpeed = 1200;
	motorLeftSpeed = 1200;
	HAL_Delay(100);

	//Are these needed? They're supposed to actually start the timers once set
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	while (1) {
		/* USER CODE END WHILE */

		//Create angleChecker program which retains raw encoder value between -360-360 for
		//easy turning purposes and resetting encoder values
		//Poll sensors (DO AN AVERAGE FOR MULTIPLE POLLS)
		updateIR(rawValues);
		/*dis_FR = measure_dist(DIST_FR);
		dis_FL = measure_dist(DIST_FL);
		dis_BR = measure_dist(DIST_BR);
		dis_BL = measure_dist(DIST_BL);*/
		HAL_Delay(1000);

		//Go Straight
		if(position < 15 && position > -15){
			motorRightSpeed = 1200;
			motorLeftSpeed = 1200;
		}
		//Go slight right
		if(position > 15){
			motorRightSpeed = 1100;
			motorLeftSpeed = 1200;
		}
		//Go slight left
		if(position < -15){
			motorRightSpeed = 1200;
			motorLeftSpeed = 1100;
		}

		//Scan walls

		//Run Flood fill
		//Figure out best cell
		//Move to best cell
		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
	PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void) {

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Common config
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	if (HAL_ADC_Init(&hadc1) != HAL_OK) {
		Error_Handler();
	}

	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_9;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 2047;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 25;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_3)
			!= HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4)
			!= HAL_OK) {
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
static void MX_TIM3_Init(void) {

	/* USER CODE BEGIN TIM3_Init 0 */

	/* USER CODE END TIM3_Init 0 */

	TIM_Encoder_InitTypeDef sConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM3_Init 1 */

	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 65535;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_ICPOLARITY_FALLING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 0;
	sConfig.IC2Polarity = TIM_ICPOLARITY_FALLING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 0;
	if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */

	/* USER CODE END TIM3_Init 2 */

}

/**
 * @brief TIM4 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM4_Init(void) {

	/* USER CODE BEGIN TIM4_Init 0 */

	/* USER CODE END TIM4_Init 0 */

	TIM_Encoder_InitTypeDef sConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };

	/* USER CODE BEGIN TIM4_Init 1 */

	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 0;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 65535;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_ICPOLARITY_FALLING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = 0;
	sConfig.IC2Polarity = TIM_ICPOLARITY_FALLING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = 0;
	if (HAL_TIM_Encoder_Init(&htim4, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */

	/* USER CODE END TIM4_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB,
			EMIT_BR_Pin | EMIT_BL_Pin | EMIT_FL_Pin | MR_FWD_Pin | ML_BWD_Pin
					| MR_BWD_Pin | EMIT_FR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(ML_FWD_GPIO_Port, ML_FWD_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : EMIT_BR_Pin EMIT_BL_Pin EMIT_FL_Pin MR_FWD_Pin
	 ML_BWD_Pin MR_BWD_Pin EMIT_FR_Pin */
	GPIO_InitStruct.Pin = EMIT_BR_Pin | EMIT_BL_Pin | EMIT_FL_Pin | MR_FWD_Pin
			| ML_BWD_Pin | MR_BWD_Pin | EMIT_FR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : ML_FWD_Pin */
	GPIO_InitStruct.Pin = ML_FWD_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(ML_FWD_GPIO_Port, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
