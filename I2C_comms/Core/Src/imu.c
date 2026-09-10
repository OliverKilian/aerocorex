/**
  ******************************************************************************
  * @file           : imu.c
  * @brief          : ICM-20948 IMU code
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
#include "imu.h"
#include <stdint.h>
#include <stdio.h>


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

void imu_init(I2C_HandleTypeDef *hi2c) {
  // Select Bank 0
  select_bank(0, hi2c);

  // Reset the IMU
  w_reg(PWR_MGMT_1, 0x80, hi2c); // Set the reset bit in PWR_MGMT_1 register
  HAL_Delay(100); // Wait for the reset to complete

  // Wake up the IMU and set clock source
  w_reg(PWR_MGMT_1, 0x01, hi2c); // Clear the reset bit and set clock source to PLL
  HAL_Delay(50);

  // Enable the Accelerometer and Gyroscope
  w_reg(PWR_MGMT_2, 0x00, hi2c); // Clear the disable bits for both accelerometer and gyroscope
  HAL_Delay(50);
 // Initialize the IMU
  uint8_t data = 0;

  select_bank(0, hi2c); // Select the desired bank
  
  
  //  Ensure IMU is working properly (should returen 0xEA)
  if (HAL_I2C_Mem_Read(hi2c, IMU_I2C_ADDR << 1, 0x00, I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY) == HAL_OK) {
    // Successfully read the register
    printf("WHO_AM_I: 0x%02X\n\r", data);
  } 
  else {
    // Handle error (e.g., print an error message, retry, etc.)
    printf("Error reading WHO_AM_I register\n\r");
  }

  // Reset all registers to default values
  w_reg(PWR_MGMT_1, 0x80, hi2c); // Set the reset bit in USER_CTRL register
  HAL_Delay(100); // Wait for the reset to complete

  // Wake up the device and set a clock source
  w_reg(PWR_MGMT_1, 0x01, hi2c); // Clear the reset bit and set clock source to PLL
  HAL_Delay(50);

  // Enable the Accelerometer and Gyroscope
  w_reg(PWR_MGMT_2, 0x00, hi2c); // Clear the disable bits for both accelerometer and gyroscope
  HAL_Delay(50);
  
  // Configure the gyro
  select_bank(2, hi2c); // Select the desired bank
  HAL_Delay(50);

  w_reg(GYRO_CFG_1, 0x03, hi2c); // Set Gyroscope configuration (e.g., 250 dps full scale)
  w_reg(GYRO_SMPLRT_DIV, 0x03, hi2c); // Set Gyroscope sample rate divider

  // Configure the accelerometer
  w_reg(ACCEL_CFG, 0x03, hi2c); // Set Accelerometer configuration (e.g., 2g full scale)
  w_reg(ACCEL_SMPLRT_DIV_1, 0x00, hi2c); // Set Accelerometer sample rate divider 1
  w_reg(ACCEL_SMPLRT_DIV_2, 0x04, hi2c); // Set Accelerometer sample rate divider 2

}

void get_gyro(I2C_HandleTypeDef *hi2c) {
    // Read the gyro
    select_bank(0, hi2c); // Select the desired bank
    int16_t gyro_x, gyro_y, gyro_z;
    int x_angular_rate, y_angular_rate, z_angular_rate;

    gyro_x = (r_reg(GYRO_XOUT_H, hi2c) << 8) | r_reg(GYRO_XOUT_L, hi2c);
    gyro_y = (r_reg(GYRO_YOUT_H, hi2c) << 8) | r_reg(GYRO_YOUT_L, hi2c);
    gyro_z = (r_reg(GYRO_ZOUT_H, hi2c) << 8) | r_reg(GYRO_ZOUT_L, hi2c);

    x_angular_rate = gyro_x / 65.5;
    y_angular_rate = gyro_y / 65.5;
    z_angular_rate = gyro_z / 65.5;

    printf("Gyro X: %d dps, Gyro Y: %d dps, Gyro Z: %d dps\n\r", x_angular_rate, y_angular_rate, z_angular_rate);
}

void get_accel(I2C_HandleTypeDef *hi2c) {
    // Read the accelerometer
    int16_t accel_x, accel_y, accel_z;
    int x_acceleration, y_acceleration, z_acceleration;

    // Read Accel
    accel_x = (r_reg(ACCEL_XOUT_H, hi2c) << 8) | r_reg(ACCEL_XOUT_L, hi2c);
    accel_y = (r_reg(ACCEL_YOUT_H, hi2c) << 8) | r_reg(ACCEL_YOUT_L, hi2c);
    accel_z = (r_reg(ACCEL_ZOUT_H, hi2c) << 8) | r_reg(ACCEL_ZOUT_L, hi2c);

    x_acceleration = (accel_x * 1000) / 8192.0;
    y_acceleration = (accel_y * 1000) / 8192.0;
    z_acceleration = (accel_z * 1000) / 8192.0;

    printf("Accelerometer X: %d mg, Accelerometer Y: %d mg, Accelerometer Z: %d mg\n\r", x_acceleration, y_acceleration, z_acceleration);
}