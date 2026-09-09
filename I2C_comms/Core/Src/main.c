/* USER CODE BEGIN Header */
/* big time baby*/
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "dps310.h"
#include <stdint.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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

COM_InitTypeDef BspCOMInit;
__IO uint32_t BspButtonState = BUTTON_RELEASED;

I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include "dps310.h"


#define imu_user_ctrl 0x03
#define REG_BANK_SEL 0X7F
#define IMU_I2C_ADDR 0x69
#define PWR_MGMT_1 0x06
#define PWR_MGMT_2 0x07
#define GYRO_CFG_1 0x01
#define ACCEL_CFG 0x14
#define GYRO_SMPLRT_DIV 0x00
#define ACCEL_SMPLRT_DIV_1 0x10
#define ACCEL_SMPLRT_DIV_2 0x11

#define GYRO_XOUT_H 0x33
#define GYRO_XOUT_L 0x34
#define GYRO_YOUT_H 0x35
#define GYRO_YOUT_L 0x36
#define GYRO_ZOUT_H 0x37
#define GYRO_ZOUT_L 0x38

#define ACCEL_XOUT_H 0x2D
#define ACCEL_XOUT_L 0x2E
#define ACCEL_YOUT_H 0x2F
#define ACCEL_YOUT_L 0x30
#define ACCEL_ZOUT_H 0x31
#define ACCEL_ZOUT_L 0x32

void w_reg(uint8_t reg, uint8_t value, I2C_HandleTypeDef *hi2c) {
  HAL_I2C_Mem_Write(hi2c, IMU_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

uint8_t r_reg(uint8_t reg, I2C_HandleTypeDef *hi2c) {
  uint8_t stuff = 0;
  if (HAL_I2C_Mem_Read(hi2c, IMU_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &stuff, 1, HAL_MAX_DELAY) == HAL_OK) {
    // Successfully read the register
  } else {
    // Handle error (e.g., print an error message, retry, etc.)
  };
  return stuff;
}



void select_bank(uint8_t bank, I2C_HandleTypeDef *hi2c) {
    // Write the bank number to the REG_BANK_SEL register
    //write_register(REG_BANK_SEL, 0x69, bank << 4, hi2c);
    uint8_t val = (bank & 0x03) << 4;
    w_reg(REG_BANK_SEL, val, hi2c);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

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
  MX_I2C1_Init();
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */
  dps310_init(&hi2c1);

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN BSP */

  /* -- Sample board code to send message over COM1 port ---- */

  /* -- Sample board code to switch on leds ---- */
  BSP_LED_On(LED_GREEN);
  BSP_LED_On(LED_YELLOW);
  BSP_LED_On(LED_RED);

  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */



  // Initialize the IMU
  uint8_t data = 0;

  select_bank(0, &hi2c1); // Select the desired bank
  
  
  //  Ensure IMU is working properly (should returen 0xEA)
  if (HAL_I2C_Mem_Read(&hi2c1, IMU_I2C_ADDR << 1, 0x00, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY) == HAL_OK) {
    // Successfully read the register
    printf("WHO_AM_I: 0x%02X\n\r", data);
  } 
  else {
    // Handle error (e.g., print an error message, retry, etc.)
    printf("Error reading WHO_AM_I register\n\r");
  }

  // Reset all registers to default values
  w_reg(PWR_MGMT_1, 0x80, &hi2c1); // Set the reset bit in USER_CTRL register
  HAL_Delay(100); // Wait for the reset to complete

  // Wake up the device and set a clock source
  w_reg(PWR_MGMT_1, 0x01, &hi2c1); // Clear the reset bit and set clock source to PLL
  HAL_Delay(50);

  // Enable the Accelerometer and Gyroscope
  w_reg(PWR_MGMT_2, 0x00, &hi2c1); // Clear the disable bits for both accelerometer and gyroscope
  HAL_Delay(50);
  
  // Configure the gyro
  select_bank(2, &hi2c1); // Select the desired bank
  HAL_Delay(50);

  w_reg(GYRO_CFG_1, 0x03, &hi2c1); // Set Gyroscope configuration (e.g., 250 dps full scale)
  w_reg(GYRO_SMPLRT_DIV, 0x03, &hi2c1); // Set Gyroscope sample rate divider

  // Configure the accelerometer
  w_reg(ACCEL_CFG, 0x03, &hi2c1); // Set Accelerometer configuration (e.g., 2g full scale)
  w_reg(ACCEL_SMPLRT_DIV_1, 0x00, &hi2c1); // Set Accelerometer sample rate divider 1
  w_reg(ACCEL_SMPLRT_DIV_2, 0x04, &hi2c1); // Set Accelerometer sample rate divider 2

  // Read the gyro
  select_bank(0, &hi2c1); // Select the desired bank
  int16_t gyro_x, gyro_y, gyro_z;
  int x_angular_rate, y_angular_rate, z_angular_rate;

  // Read the accelerometer
  int16_t accel_x, accel_y, accel_z;
  int x_acceleration, y_acceleration, z_acceleration;

  while (1)
  {
    /* USER CODE BEGIN 3 */
    // Read Gyro
    gyro_x = (r_reg(GYRO_XOUT_H, &hi2c1) << 8) | r_reg(GYRO_XOUT_L, &hi2c1);
    gyro_y = (r_reg(GYRO_YOUT_H, &hi2c1) << 8) | r_reg(GYRO_YOUT_L, &hi2c1);
    gyro_z = (r_reg(GYRO_ZOUT_H, &hi2c1) << 8) | r_reg(GYRO_ZOUT_L, &hi2c1);

    x_angular_rate = gyro_x / 65.5;
    y_angular_rate = gyro_y / 65.5;
    z_angular_rate = gyro_z / 65.5;

    printf("Gyro X: %d dps, Gyro Y: %d dps, Gyro Z: %d dps\n\r", x_angular_rate, y_angular_rate, z_angular_rate);

    // Read Accel
    accel_x = (r_reg(ACCEL_XOUT_H, &hi2c1) << 8) | r_reg(ACCEL_XOUT_L, &hi2c1);
    accel_y = (r_reg(ACCEL_YOUT_H, &hi2c1) << 8) | r_reg(ACCEL_YOUT_L, &hi2c1);
    accel_z = (r_reg(ACCEL_ZOUT_H, &hi2c1) << 8) | r_reg(ACCEL_ZOUT_L, &hi2c1);

    x_acceleration = (accel_x * 1000) / 8192.0;
    y_acceleration = (accel_y * 1000) / 8192.0;
    z_acceleration = (accel_z * 1000) / 8192.0;

    printf("Accelerometer X: %d mg, Accelerometer Y: %d mg, Accelerometer Z: %d mg\n\r", x_acceleration, y_acceleration, z_acceleration);

    /* -- Sample board code for User push-button in interrupt mode ---- */
    // if (BspButtonState == BUTTON_PRESSED)
    // {
    //   /* Update button state */
    //   BspButtonState = BUTTON_RELEASED;
    //   /* -- Sample board code to toggle leds ---- */
    //   BSP_LED_Toggle(LED_GREEN);
    //   BSP_LED_Toggle(LED_YELLOW);
    //   BSP_LED_Toggle(LED_RED);

    //   /* ..... Perform your action ..... */
    // }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    /* -- Sample board code for User push-button in interrupt mode ---- */
    // if (BspButtonState == BUTTON_PRESSED)
    // {
    //   /* Update button state */
    //   BspButtonState = BUTTON_RELEASED;
    //   /* -- Sample board code to toggle leds ---- */
    //   BSP_LED_Toggle(LED_GREEN);
    //   BSP_LED_Toggle(LED_YELLOW);
    //   BSP_LED_Toggle(LED_RED);

    //   /* ..... Perform your action ..... */
    // }
    /* USER CODE END WHILE */


    printf("Temperature: %d C\n\r", (int)dps310_get_temperature(&hi2c1));
    printf("Pressure: %d Pa\n\r", (int)dps310_get_pressure(&hi2c1));
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
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
  hi2c1.Init.Timing = 0x10707DBC;
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
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x10707DBC;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

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
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LD1_Pin */
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);

  // /*Configure GPIO pin : interrupt1_Pin */
  // GPIO_InitStruct.Pin = interrupt1_Pin;
  // GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  // GPIO_InitStruct.Pull = GPIO_NOPULL;
  // HAL_GPIO_Init(interrupt1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief BSP Push Button callback
  * @param Button Specifies the pressed button
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_USER)
  {
    BspButtonState = BUTTON_PRESSED;
  }
}

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
#ifdef USE_FULL_ASSERT
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
