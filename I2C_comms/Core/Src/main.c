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
#include <stdint.h>

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define baro_I2C_ADDR 0x77
#define baro_PSR_B2 0x00
#define baro_PSR_B1 0x01
#define baro_PSR_B0 0x02
#define baro_TMP_B2 0x03
#define baro_TMP_B1 0x04
#define baro_TMP_B0 0x05
#define baro_PRS_CFG 0x06
#define baro_TMP_CFG 0x07
#define baro_MEAS_CFG 0x08
#define baro_CFG_REG 0x09
#define baro_reset 0x0C


// Calibration Coefficients
#define baro_c0 0x10
#define baro_c0_c1 0x11
#define baro_c1 0x12
#define baro_c00_a 0x13
#define baro_c00_b 0x14
#define baro_c00_c10 0x15
#define baro_c10_a 0x16
#define baro_c10_b 0x17
#define baro_c01_a 0x18
#define baro_c01_b 0x19
#define baro_c11_a 0x1A
#define baro_c11_b 0x1B
#define baro_c20_a 0x1C
#define baro_c20_b 0x1D
#define baro_c21_a 0x1E
#define baro_c21_b 0x1F
#define baro_c30_a 0x20
#define baro_c30_b 0x21

// Calibration Coefficients
int16_t c0_val;
int16_t c1_val;
int32_t c00_val;
int32_t c10_val;
int16_t c01_val;
int16_t c11_val;
int16_t c20_val;
int16_t c21_val;
int16_t c30_val;
float pres_scale_factor = 253952.0f;
float temp_scale_factor = 524288.0f;
int32_t t_raw;
int32_t p_raw;
float t_sc;
float p_sc;
float temp_scaled;
float pres_scaled;

// Read register function
uint8_t read_register(uint8_t reg) {
  uint8_t data = 0;
  if (HAL_I2C_Mem_Read(&hi2c1, baro_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY) == HAL_OK) {
    // Successfully read the register
  } else {
    // Handle error (e.g., print an error message, retry, etc.)
  };
  return data;
}

// Write register function
void write_register(uint8_t reg, uint8_t value) {
  HAL_I2C_Mem_Write(&hi2c1, baro_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

// Delay Function
void delay(uint32_t milliseconds) {
  HAL_Delay(milliseconds);
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
  /* USER CODE BEGIN 2 */
 
  // Read write variable declarations
  uint8_t tmp_cfg = 0x00;
  uint8_t psr_cfg = 0x14;
  uint8_t psr_cfg_read = 0x00;
  uint8_t cfg_reg = 0x04;

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

  // Calculate coefficients

  c0_val = (read_register(baro_c0) << 4) | (read_register(baro_c0_c1) >> 4); 
  if (c0_val & 0x0800) { // Check if the sign bit is set
    c0_val |= 0xF000; // Sign extend to 16 bits
  }

  c1_val = ((read_register(baro_c0_c1) & 0x0F) << 8) | read_register(baro_c1);
  if (c1_val & 0x0800) { // Check if the sign bit is set
    c1_val |= 0xF000; // Sign extend to 16 bits
  }

  c00_val = (read_register(baro_c00_a) << 12) | (read_register(baro_c00_b) << 4) | (read_register(baro_c00_c10) >> 4);
  if (c00_val & 0x80000) { // Check if the sign bit is set
    c00_val |= 0xFFF00000; // Sign extend to 32 bits
  }

  c10_val = ((read_register(baro_c00_c10) & 0x0F) << 16) | (read_register(baro_c10_a) << 8) | read_register(baro_c10_b);
  if (c10_val & 0x80000) { // Check if the sign bit is set
      c10_val |= 0xFFF00000; // Sign extend to 32 bits
    }

  c01_val = (read_register(baro_c01_a) << 8) | read_register(baro_c01_b);

  c11_val = ((read_register(baro_c11_a) << 8) | read_register(baro_c11_b));

  c20_val = (read_register(baro_c20_a) << 8) | read_register(baro_c20_b);

  c21_val = (read_register(baro_c21_a) << 8) | read_register(baro_c21_b);

  c30_val = (read_register(baro_c30_a) << 8) | read_register(baro_c30_b);

  printf("c0_val: %d\n\r", c0_val);
  printf("c1_val: %d\n\r", c1_val);
  printf("c00_val: %d\n\r", c00_val);
  printf("c10_val: %d\n\r", c10_val);
  printf("c01_val: %d\n\r", c01_val);
  printf("c11_val: %d\n\r", c11_val);
  printf("c20_val: %d\n\r", c20_val);
  printf("c21_val: %d\n\r", c21_val);
  printf("c30_val: %d\n\r", c30_val);


  // after coefficient reads, before configuring PRS_CFG/TMP_CFG:

  // Read COEF_SRCE (0x28) - tells us which sensor the factory coefficients were derived from
  uint8_t coef_srce = read_register(0x28);
  uint8_t tmp_ext = coef_srce & 0x80;   // bit 7 = TMP_COEF_SRCE
  printf("COEF_SRCE: 0x%02X, TMP_EXT to use: 0x%02X\n\r", coef_srce, tmp_ext);

  // Temperature Configuration: TMP_EXT MUST match COEF_SRCE bit 7. 1x oversampling.
  tmp_cfg = tmp_ext | 0x00;   // rate bits don't matter in command mode; PRC=0000=1x

  write_register(baro_TMP_CFG, tmp_cfg);
  delay(70);

  // Pressure Configuration
  //Write 7-bits to pressure register. Bit mask in order to preserve the original bits that aren't being written to.
  psr_cfg_read = read_register(baro_PRS_CFG);
  psr_cfg = (psr_cfg_read & 0x80) | (psr_cfg & 0x7F); //Preserve the original bit 7, write to bits 6-0
  write_register(baro_PRS_CFG, psr_cfg);

  // Temperature Configuration
  write_register(baro_TMP_CFG, tmp_cfg);
  delay(70);

  // Configuration Register
  write_register(baro_CFG_REG, cfg_reg);
  delay(70);

  // meas_cfg_read = read_register(baro_MEAS_CFG);
  // meas_cfg = (meas_cfg_read & 0xF8) | (meas_cfg & 0x07); //Preserve the original bits 7-3, write to bits 2-0

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Measurement Register Configuration
    write_register(baro_MEAS_CFG, 0x02);
    delay(70); // Wait for 100 ms for the measurement to complete

    // Calculate temperature registers
    t_raw = (read_register(baro_TMP_B2) << 16 | read_register(baro_TMP_B1) << 8 | read_register(baro_TMP_B0));
    if(t_raw & 0x800000) { // Check if the sign bit is set
      t_raw |= 0xFF000000; // Sign extend to 32 bits
    }
    // Measurement Register Configuration
    write_register(baro_MEAS_CFG, 0x01);
    delay(70); // Wait for 100 ms for the measurement to complete
  
    // Calculate pressure registers
    p_raw = (read_register(baro_PSR_B2) << 16 | read_register(baro_PSR_B1) << 8 | read_register(baro_PSR_B0));
    if(p_raw & 0x800000) { // Check if the sign bit is set
      p_raw |= 0xFF000000; // Sign extend to 32 bits
    }

    t_sc = (float)t_raw / temp_scale_factor;
    p_sc = (float)p_raw / pres_scale_factor;

    printf("Raw Temperature: %d, Raw Pressure: %d\n\r", (int)(t_raw), (int)p_raw);

   
    pres_scaled = c00_val + p_sc*(c10_val + p_sc *(c20_val+ p_sc *c30_val)) + t_sc *c01_val + t_sc * p_sc *(c11_val+p_sc*c21_val);
    temp_scaled = c0_val * 0.5 + c1_val * t_sc;

    printf("Temperature: %d C, Pressure: %d Pa\n\r", (int)(temp_scaled), (int)pres_scaled);

    // Both values are going up and down when cahnged but their most signifcant values aren't changing which
    // makes it seem like the values are not changing. Figure out why this is, our coefficients might be wrong.


    
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
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB__Button__Pin */
  GPIO_InitStruct.Pin = PB__Button__Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(PB__Button__GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD1_Pin */
  GPIO_InitStruct.Pin = LD1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);

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
