// Include guard: "Has it already been defined? If not, define it now."
#ifndef IMU_H
#define IMU_H

#ifdef __cplusplus // Allow a C header to be safely included from C++
extern "C" {
#endif

// Public API goes here

#include <stdint.h>
#include "stm32h7xx_hal.h"

void get_accel(I2C_HandleTypeDef *hi2c);
void get_gyro(I2C_HandleTypeDef *hi2c);
void imu_init(I2C_HandleTypeDef *hi2c);
void select_bank(uint8_t bank, I2C_HandleTypeDef *hi2c);
uint8_t r_reg(uint8_t reg, I2C_HandleTypeDef *hi2c);
void w_reg(uint8_t reg, uint8_t value, I2C_HandleTypeDef *hi2c);


#ifdef __cplusplus
}
#endif

#endif
