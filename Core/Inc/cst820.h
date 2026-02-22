#ifndef __CST820_H
#define __CST820_H

#include "stm32u5xx_hal.h"   // replace xxxx with your MCU family

#define CST820_I2C_ADDR      (0x15 << 1)   // Adjust if needed

/* Gesture values (same as Arduino lib) */
typedef enum {
    CST820_GESTURE_NONE = 0,
    CST820_GESTURE_SLIDE_UP,
    CST820_GESTURE_SLIDE_DOWN,
    CST820_GESTURE_SLIDE_LEFT,
    CST820_GESTURE_SLIDE_RIGHT,
    CST820_GESTURE_SINGLE_TAP,
    CST820_GESTURE_DOUBLE_TAP,
    CST820_GESTURE_LONG_PRESS
} CST820_Gesture_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;

    GPIO_TypeDef *rst_port;
    uint16_t rst_pin;

    GPIO_TypeDef *int_port;
    uint16_t int_pin;

} CST820_HandleTypeDef;

HAL_StatusTypeDef CST820_Init(CST820_HandleTypeDef *hcst);
HAL_StatusTypeDef CST820_Reset(CST820_HandleTypeDef *hcst);

uint8_t CST820_GetTouch(CST820_HandleTypeDef *hcst,
                        uint16_t *x,
                        uint16_t *y,
                        uint8_t *gesture);

#endif
