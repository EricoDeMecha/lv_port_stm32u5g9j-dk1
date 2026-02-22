/*********************
 *      INCLUDES
 *********************/

#include "lvgl_port.h"
#include "lvgl/lvgl.h"
#include "stm32u5xx_hal.h"
#include "main.h"
#include "cst820.h"

extern I2C_HandleTypeDef hi2c5;

/*********************
 *      DEFINES
 *********************/

#define DISP_WIDTH   466
#define DISP_HEIGHT  466
#define DISP_BPP     2    /* RGB565 = 2 bytes per pixel */

/**********************
 *  STATIC PROTOTYPES
 **********************/

void touch_read(lv_indev_t * indev, lv_indev_data_t * data);

/**********************
 *  STATIC VARIABLES
 **********************/

static volatile bool do_sample_touch = false;
static lv_indev_state_t last_state = LV_INDEV_STATE_RELEASED;
static CST820_HandleTypeDef hcst;

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lvgl_port_init(void)
{
    lv_init();

    lv_tick_set_cb(HAL_GetTick);

    /* CST820 touch controller on I2C5.
     * No dedicated touch reset pin on the DK1 adapter board —
     * the CST820 gets power-on reset from the VCI_EN power cycle. */
    hcst.hi2c = &hi2c5;
    hcst.rst_port = NULL;
    hcst.rst_pin = 0;
    hcst.int_port = TP_IRQ_GPIO_Port;
    hcst.int_pin = TP_IRQ_Pin;
    CST820_Init(&hcst);

    /* Direct double-buffered rendering:
     * Buffer A = LTDC framebuffer at 0x20000000 (RAM2)
     * Buffer B = static array (placed by linker in RAM) */
    static __attribute__((aligned(32))) uint8_t buf_direct_2[DISP_WIDTH * DISP_HEIGHT * DISP_BPP];
    lv_st_ltdc_create_direct((void *)0x20000000, buf_direct_2, 0);

    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == TP_IRQ_Pin)
    {
        do_sample_touch = true;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

void touch_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    NVIC_DisableIRQ(EXTI8_IRQn);

    if (do_sample_touch)
    {
        do_sample_touch = false;

        uint16_t x, y;
        uint8_t gesture;

        if (CST820_GetTouch(&hcst, &x, &y, &gesture))
        {
            last_state = LV_INDEV_STATE_PRESSED;
            data->point.x = x;
            data->point.y = y;
        }
        else
        {
            last_state = LV_INDEV_STATE_RELEASED;
        }
    }

    NVIC_EnableIRQ(EXTI8_IRQn);

    data->state = last_state;
}
