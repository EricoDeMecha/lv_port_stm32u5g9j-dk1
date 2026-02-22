#include "cst820.h"

/* ================= I2C Helpers ================= */

static HAL_StatusTypeDef CST820_ReadReg(CST820_HandleTypeDef *hcst,
                                        uint8_t reg,
                                        uint8_t *data,
                                        uint16_t len)
{
    return HAL_I2C_Mem_Read(hcst->hi2c,
                            CST820_I2C_ADDR,
                            reg,
                            I2C_MEMADD_SIZE_8BIT,
                            data,
                            len,
                            HAL_MAX_DELAY);
}

static HAL_StatusTypeDef CST820_WriteReg(CST820_HandleTypeDef *hcst,
                                         uint8_t reg,
                                         uint8_t data)
{
    return HAL_I2C_Mem_Write(hcst->hi2c,
                             CST820_I2C_ADDR,
                             reg,
                             I2C_MEMADD_SIZE_8BIT,
                             &data,
                             1,
                             HAL_MAX_DELAY);
}

/* ================= Reset ================= */

HAL_StatusTypeDef CST820_Reset(CST820_HandleTypeDef *hcst)
{
    if (hcst->rst_port == NULL)
        return HAL_OK;

    HAL_GPIO_WritePin(hcst->rst_port, hcst->rst_pin, GPIO_PIN_RESET);
    HAL_Delay(10);        // Arduino delay(10)
    HAL_GPIO_WritePin(hcst->rst_port, hcst->rst_pin, GPIO_PIN_SET);
    HAL_Delay(300);       // Arduino delay(300)

    return HAL_OK;
}

/* ================= Init (Arduino begin()) ================= */

HAL_StatusTypeDef CST820_Init(CST820_HandleTypeDef *hcst)
{
    if (hcst == NULL || hcst->hi2c == NULL)
        return HAL_ERROR;

    /* Reset device */
    CST820_Reset(hcst);

    /* Check device ready */
    if (HAL_I2C_IsDeviceReady(hcst->hi2c,
                              CST820_I2C_ADDR,
                              3,
                              100) != HAL_OK)
    {
        return HAL_ERROR;
    }

    /* Disable auto low power mode
       i2c_write(0xFE, 0xFF);  */
    if (CST820_WriteReg(hcst, 0xFE, 0xFF) != HAL_OK)
        return HAL_ERROR;

    return HAL_OK;
}

/* ================= Equivalent to Arduino getTouch() ================= */

uint8_t CST820_GetTouch(CST820_HandleTypeDef *hcst,
                        uint16_t *x,
                        uint16_t *y,
                        uint8_t *gesture)
{
    uint8_t finger = 0;
    uint8_t data[4];

    /* FingerIndex = i2c_read(0x02); */
    if (CST820_ReadReg(hcst, 0x02, &finger, 1) != HAL_OK)
        return 0;

    /* Read gesture from 0x01 */
    if (CST820_ReadReg(hcst, 0x01, gesture, 1) != HAL_OK)
        return 0;

    /* Validate gesture (same logic as Arduino) */
    if (!(*gesture == CST820_GESTURE_SLIDE_UP ||
          *gesture == CST820_GESTURE_SLIDE_DOWN ||
          *gesture == CST820_GESTURE_SLIDE_LEFT ||
          *gesture == CST820_GESTURE_SLIDE_RIGHT ||
          *gesture == CST820_GESTURE_SINGLE_TAP ||
          *gesture == CST820_GESTURE_DOUBLE_TAP ||
          *gesture == CST820_GESTURE_LONG_PRESS))
    {
        *gesture = CST820_GESTURE_NONE;
    }

    /* Read 4 bytes from 0x03 */
    if (CST820_ReadReg(hcst, 0x03, data, 4) != HAL_OK)
        return 0;

    *x = ((data[0] & 0x0F) << 8) | data[1];
    *y = ((data[2] & 0x0F) << 8) | data[3];

    return finger ? 1 : 0;
}
