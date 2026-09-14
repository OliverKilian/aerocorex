/**
  ******************************************************************************
  * @file           : dps310.c
  * @brief          : dps310 barometer code
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
#include "dps310.h"
#include <stdint.h>
#include <stdio.h>

// DPS310 Register Addresses
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
#define baro_INT_STS 0x0A


// DPS 310 Calibration Coefficient Locations
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

// Calibration Coefficients Variables
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
uint8_t read_register(uint8_t reg, I2C_HandleTypeDef *hi2c) {
  uint8_t data = 0;
  if (HAL_I2C_Mem_Read(hi2c, baro_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY) == HAL_OK) {
    // Successfully read the register
  } else {
    // Handle error (e.g., print an error message, retry, etc.)
  };
  return data;
}

// Write register function
void write_register(uint8_t reg, uint8_t value, I2C_HandleTypeDef *hi2c) {
  HAL_I2C_Mem_Write(hi2c, baro_I2C_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}


void dps310_init(I2C_HandleTypeDef *hi2c) {
    // Calculate coefficients
    c0_val = (read_register(baro_c0, hi2c) << 4) | (read_register(baro_c0_c1, hi2c) >> 4); 
    if (c0_val & 0x0800) { // Check if the sign bit is set
        c0_val |= 0xF000; // Sign extend to 16 bits
    }

    c1_val = ((read_register(baro_c0_c1, hi2c) & 0x0F) << 8) | read_register(baro_c1, hi2c);
    if (c1_val & 0x0800) { // Check if the sign bit is set
        c1_val |= 0xF000; // Sign extend to 16 bits
    }

    c00_val = (read_register(baro_c00_a, hi2c) << 12) | (read_register(baro_c00_b, hi2c) << 4) | (read_register(baro_c00_c10, hi2c) >> 4);
    if (c00_val & 0x80000) { // Check if the sign bit is set
        c00_val |= 0xFFF00000; // Sign extend to 32 bits
    }

    c10_val = ((read_register(baro_c00_c10, hi2c) & 0x0F) << 16) | (read_register(baro_c10_a, hi2c) << 8) | read_register(baro_c10_b, hi2c);
    if (c10_val & 0x80000) { // Check if the sign bit is set
        c10_val |= 0xFFF00000; // Sign extend to 32 bits
        }

    c01_val = (read_register(baro_c01_a, hi2c) << 8) | read_register(baro_c01_b, hi2c);

    c11_val = ((read_register(baro_c11_a, hi2c) << 8) | read_register(baro_c11_b, hi2c));

    c20_val = (read_register(baro_c20_a, hi2c) << 8) | read_register(baro_c20_b, hi2c);

    c21_val = (read_register(baro_c21_a, hi2c) << 8) | read_register(baro_c21_b, hi2c);

    c30_val = (read_register(baro_c30_a, hi2c) << 8) | read_register(baro_c30_b, hi2c);

    printf("c0_val: %d\n\r", c0_val);
    printf("c1_val: %d\n\r", c1_val);
    printf("c00_val: %d\n\r", c00_val);
    printf("c10_val: %d\n\r", c10_val);
    printf("c01_val: %d\n\r", c01_val);
    printf("c11_val: %d\n\r", c11_val);
    printf("c20_val: %d\n\r", c20_val);
    printf("c21_val: %d\n\r", c21_val);
    printf("c30_val: %d\n\r", c30_val);

    // Read write variable declarations
    uint8_t tmp_cfg = 0x00;
    uint8_t psr_cfg = 0x14;
    uint8_t psr_cfg_read = 0x00;
    uint8_t cfg_reg = 0x04;

    // after coefficient reads, before configuring PRS_CFG/TMP_CFG:

    // Read COEF_SRCE (0x28) - tells us which sensor the factory coefficients were derived from
    uint8_t coef_srce = read_register(0x28, hi2c);
    uint8_t tmp_ext = coef_srce & 0x80;   // bit 7 = TMP_COEF_SRCE
    printf("COEF_SRCE: 0x%02X, TMP_EXT to use: 0x%02X\n\r", coef_srce, tmp_ext);

    // Temperature Configuration: TMP_EXT MUST match COEF_SRCE bit 7. 1x oversampling.
    tmp_cfg = tmp_ext | 0x00;   // rate bits don't matter in command mode; PRC=0000=1x

    write_register(baro_TMP_CFG, tmp_cfg, hi2c);
    HAL_Delay(70);

    // Pressure Configuration
    //Write 7-bits to pressure register. Bit mask in order to preserve the original bits that aren't being written to.
    psr_cfg_read = read_register(baro_PRS_CFG, hi2c);
    psr_cfg = (psr_cfg_read & 0x80) | (psr_cfg & 0x7F); //Preserve the original bit 7, write to bits 6-0
    write_register(baro_PRS_CFG, psr_cfg, hi2c);

    // Temperature Configuration
    write_register(baro_TMP_CFG, tmp_cfg, hi2c);
    HAL_Delay(70);

    // Configuration Register
    write_register(baro_CFG_REG, cfg_reg, hi2c);
    HAL_Delay(70);
}

float dps310_get_temperature(I2C_HandleTypeDef *hi2c) {
/* USER CODE BEGIN 3 */
    // Measurement Register Configuration
    write_register(baro_MEAS_CFG, 0x02, hi2c);
    HAL_Delay(70); // Wait for 100 ms for the measurement to complete

    // Calculate temperature registers
    t_raw = (read_register(baro_TMP_B2, hi2c) << 16 | read_register(baro_TMP_B1, hi2c) << 8 | read_register(baro_TMP_B0, hi2c));
    if(t_raw & 0x800000) { // Check if the sign bit is set
      t_raw |= 0xFF000000; // Sign extend to 32 bits
    }

    t_sc = (float)t_raw / temp_scale_factor;
    temp_scaled = c0_val * 0.5 + c1_val * t_sc;
    HAL_Delay(70); // ensure any sensor timing
    return temp_scaled;
  }


float dps310_get_pressure(I2C_HandleTypeDef *hi2c) {
    // Measurement Register Configuration
    // Measurement Register Configuration
    write_register(baro_MEAS_CFG, 0x01, hi2c);
    HAL_Delay(70); // Wait for 100 ms for the measurement to complete
  
    // Calculate pressure registers
    p_raw = (read_register(baro_PSR_B2, hi2c) << 16 | read_register(baro_PSR_B1, hi2c) << 8 | read_register(baro_PSR_B0, hi2c));
    if(p_raw & 0x800000) { // Check if the sign bit is set
      p_raw |= 0xFF000000; // Sign extend to 32 bits
    }

    p_sc = (float)p_raw / pres_scale_factor;
    pres_scaled = c00_val + p_sc*(c10_val + p_sc *(c20_val+ p_sc *c30_val)) + t_sc *c01_val + t_sc * p_sc *(c11_val+p_sc*c21_val);
    return pres_scaled;
  }