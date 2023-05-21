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
#include <string.h>
#include "ads1293.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN PV */

 uint8_t data[3];
 int timeout= 100;
 int size = 2;
 uint8_t tx_Data[20];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void cs_low_ads(){
	  HAL_GPIO_WritePin(GPIOA, ADS_CS_Pin, GPIO_PIN_RESET); //CS установлен в ноль
	  };
static void cs_high_ads(){
	 	  HAL_GPIO_WritePin(GPIOA, ADS_CS_Pin, GPIO_PIN_SET); //CS установлен в единицу
	 	  };


void ADS1293_write(uint8_t address, uint8_t value){
	uint8_t data[2];
	uint8_t command_field = address & ADS1293_WRITE_BIT;//формируем command field
	data[0] = command_field;
	data[1] = value; //байт, который запишем в регистр

	cs_low_ads();
	HAL_SPI_Transmit(&hspi1, data, 2, 100);
	//передача двух байт, первый - адрес регистра,
	//второй - то, что хотим записать
	cs_high_ads();
	HAL_Delay(10);
};
static uint8_t ADS1293_read(uint8_t address){
	uint8_t command_field = address | ADS1293_READ_BIT;
	uint8_t read_val;
	cs_low_ads();
	HAL_SPI_Transmit(&hspi1, &command_field, 1, 100);
	HAL_SPI_Receive(&hspi1, &read_val, 1, 100);
	cs_high_ads();
	return read_val;
};

void ADS1293_INIT(){
	//power-down - выкл, stand-by - выкл, start conversion выкл
	ADS1293_write(ADS1293_CONFIG_REG,0b000);
	//POS1=IN2, NEG1=IN1
	ADS1293_write(ADS1293_FLEX_CH1_CN_REG,0b00010001);
	//POS2=IN3, NEG2=IN1
	ADS1293_write(ADS1293_FLEX_CH2_CN_REG,0b00011001);
	// POS3=IN5, NEG3=IN6
	ADS1293_write(ADS1293_FLEX_CH3_CN_REG,0b00101110);
	//common-mode detector -вкл на IN1,IN2,IN3
	ADS1293_write(ADS1293_CMDET_EN_REG,0b00000111);
	//подкл IN4 к RLD control
	ADS1293_write(ADS1293_RLD_CN_REG,0b0000100);
	//подключаем Wilson control
	ADS1293_write(ADS1293_WILSON_EN1_REG,0b001);
	ADS1293_write(ADS1293_WILSON_EN2_REG,0b010);
	ADS1293_write(ADS1293_WILSON_EN3_REG,0b011);
	ADS1293_write(ADS1293_WILSON_CN_REG,0b01);
	
	//используем внешний кварцевый резонатор
	ADS1293_write(ADS1293_OSC_CN_REG,0b100);
	
	
	//настраиваем цифровой фильтр
	ADS1293_write(ADS1293_R2_RATE_REG,0b1000);
	
	ADS1293_write(ADS1293_R3_RATE1_REG,0b100);
	ADS1293_write(ADS1293_R3_RATE2_REG,0b100);
	ADS1293_write(ADS1293_R3_RATE3_REG,0b100);
	
	//data ready bar на channel 1 ECG
	ADS1293_write(ADS1293_DRDYB_SRC_REG,0b1000);
	
	//вкл чтение с CH1 CH2 CH3, вкл status data loop read-back
	ADS1293_write(ADS1293_CH_CNFG_REG,0b1110001);

};

void ADS1293_start_conv(){
	ADS1293_write(ADS1293_CH_CNFG_REG,0b001);
}


void ADS1293_stream_read(unsigned long int* databuffer){

	unsigned long int  result = 0x000000000000000000000000;
	  //////////// Read lead I
	  result=ADS1293_read(ADS1293_DATA_CH1_ECG_H_REG);
	  result=result<<8;
	  result|=ADS1293_read(ADS1293_DATA_CH1_ECG_M_REG);
	  result=result<<8;
	  result|=ADS1293_read(ADS1293_DATA_CH1_ECG_L_REG);
	  databuffer[0]=result;

	  /////////// Read lead II
	  result=ADS1293_read(ADS1293_DATA_CH2_ECG_H_REG);
	  result=result<<8;
	  result|=ADS1293_read(ADS1293_DATA_CH2_ECG_M_REG);
	  result=result<<8;
	  result|=ADS1293_read(ADS1293_DATA_CH2_ECG_L_REG);
	  databuffer[1]=result;

	  //////Read lead V1
	  result=ADS1293_read(ADS1293_DATA_CH3_ECG_H_REG);
	  result=result<<8;
	  result|=ADS1293_read(ADS1293_DATA_CH2_ECG_M_REG);
	  result=result<<8;
	  result|=ADS1293_read(ADS1293_DATA_CH2_ECG_L_REG);
	  databuffer[2]=result;


}




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

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  ADS1293_read(ADS1293_AFE_DITHER_EN_REG);



/*

	  void ADS_INIT(){

		  uint8_t tmpTXData[20];//объявили массив для хранения данных
		  memset(tmpTXData, 0, sizeof(tmpTXData));//Очистка массива
		  tmpTXData[0]=ADS1293_CONFIG_REG;//записали в первый элемент массива адрес конфигурационного регистра
		  tmpTXData[1]=0x010;//запустили режим ожидания - записав во второй элемент массива нужную комбинацию
		  ADS1293_write_read(tmpTXData, 2);//отправляем по spi два байта - адрес регистра + то что хотим записать

	  }
	 void ADS_CONFIG(){
			uint8_t tmpTXData[20];//объявили массив для хранения данных

			memset(tmpTXData, 0, sizeof(tmpTXData));//Очистка массива
			tmpTXData[0]=ADS1293_FLEX_CH1_CN_REG;//записали в первый элемент массива адрес конфигурационного регистра
			tmpTXData[1]=0b01001010;// конфигурация Flex Routing Switch Control for Channel 1
			ADS1293_write_read(tmpTXData, 2);//отправляем по spi два байта - адрес регистра + то что хотим записать

			memset(tmpTXData, 0, sizeof(tmpTXData));//Очистка массива
			tmpTXData[0]=ADS1293_FLEX_CH2_CN_REG;//записали в первый элемент массива адрес конфигурационного регистра
			tmpTXData[1]=0b01001010;// конфигурация Flex Routing Switch Control for Channel 2
			ADS1293_write_read(tmpTXData, 2);

			memset(tmpTXData, 0, sizeof(tmpTXData));//Очистка массива
			tmpTXData[0]=ADS1293_FLEX_CH3_CN_REG;//записали в первый элемент массива адрес конфигурационного регистра
			tmpTXData[1]=0b01001010;// конфигурация Flex Routing Switch Control for Channel 3
			ADS1293_write_read(tmpTXData, 2);

			memset(tmpTXData, 0, sizeof(tmpTXData));//Очистка массива
			tmpTXData[0]=ADS1293_FLEX_VBAT_CN_REG;//записали в первый элемент массива адрес конфигурационного регистра
			tmpTXData[1]=0b111;// Flex Routing Switch for Battery Monitoring
			ADS1293_write_read(tmpTXData, 2);


			//Lead-Off Detect Control Registers


			memset(tmpTXData, 0, sizeof(tmpTXData));//Очистка массива
			tmpTXData[0]=ADS1293_LOD_CN_REG;//записали в первый элемент массива адрес конфигурационного регистра
			tmpTXData[1]=0b10100;//Lead-Off Detect Control
			ADS1293_write_read(tmpTXData, 2);



			//Lead-Off Detect Enable

			memset(tmpTXData, 0, sizeof(tmpTXData));//Очистка массива
			tmpTXData[0]=ADS1293_LOD_CN_REG;//записали в первый элемент массива адрес конфигурационного регистра
			tmpTXData[1]=0b111111;//Lead-Off Detect Enable
			ADS1293_write_read(tmpTXData, 2);



	 	 }*/









  }
  /* USER CODE END 3 */
}

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_HARD_INPUT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_HARD_OUTPUT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

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
