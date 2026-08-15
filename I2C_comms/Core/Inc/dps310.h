// Include guard: "Has it already been defined? If not, define it now."
#ifndef DPS310_H
#define DPS310_H

#ifdef __cplusplus // Allow a C header to be safely included from C++
extern "C" {
#endif

// Public API goes here

#include <stdint.h>
#include "stm32h7xx_hal.h"

void dps310_init(I2C_HandleTypeDef *hi2c);
float dps310_get_temperature(I2C_HandleTypeDef *hi2c);
float dps310_get_pressure(I2C_HandleTypeDef *hi2c);


#ifdef __cplusplus
}
#endif

#endif
