/**
 * @file BH1750_example.c
 * @brief BH1750 驱动使用示例 (基于 STM32 HAL 库)
 * @note  本文件仅供参考，展示如何适配接口和调用API
 */

#include "bh1750_driver.h"
#include "stm32f4xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

static driver_status_t my_i2c_write(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)p_data, len, 100);
    return (ret == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

static driver_status_t my_i2c_read(void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, p_data, len, 100);
    return (ret == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

static void my_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

static bh1750_dev_t g_bh1750;

void BH1750_Demo_Init(void)
{
    driver_i2c_ops_t i2c_ops = {
        .write_reg = my_i2c_write,
        .read_reg  = my_i2c_read
    };

    driver_time_ops_t time_ops = {
        .delay_ms = my_delay_ms
    };

    bh1750_config_t config = {
        .mt_reg = 69
    };

    driver_status_t ret = bh1750_init(&g_bh1750,
                                       &i2c_ops,
                                       &time_ops,
                                       &hi2c1,
                                       BH1750_ADDR_DEFAULT,
                                       &config);

    if (ret != DRV_OK) {
        printf("BH1750 Init Failed: %d\n", ret);
    } else {
        printf("BH1750 Init Success!\n");
    }
}

void BH1750_Demo_Continuous_Mode(void)
{
    float lux;

    if (bh1750_read_lux(&g_bh1750, &lux) == DRV_OK) {
        printf("Light Intensity: %.2f lux\n", lux);
    } else {
        printf("Read Failed\n");
    }

    HAL_Delay(100);
}

void BH1750_Demo_One_Shot_Mode(void)
{
    float lux;

    if (bh1750_measure_lux(&g_bh1750, &lux) == DRV_OK) {
        printf("Light Intensity (One Shot): %.2f lux\n", lux);
    } else {
        printf("One Shot Measurement Failed\n");
    }

    HAL_Delay(1000);
}

void BH1750_Demo_Change_Mode(void)
{
    bh1750_set_mode(&g_bh1750, BH1750_MODE_CON_H_RES);
    printf("Switched to Continuous High Resolution Mode (1 lux)\n");

    HAL_Delay(5000);

    bh1750_set_mode(&g_bh1750, BH1750_MODE_CON_L_RES);
    printf("Switched to Continuous Low Resolution Mode (4 lux)\n");

    HAL_Delay(5000);

    bh1750_set_mode(&g_bh1750, BH1750_MODE_CON_H_RES2);
    printf("Switched back to Continuous High Resolution Mode 2 (0.5 lux)\n");
}

void BH1750_Demo_Adjust_Sensitivity(void)
{
    float lux;

    printf("Default Sensitivity (MTReg=69):\n");
    bh1750_read_lux(&g_bh1750, &lux);
    printf("Light Intensity: %.2f lux\n", lux);

    HAL_Delay(1000);

    printf("High Sensitivity (MTReg=254):\n");
    bh1750_set_sensitivity(&g_bh1750, 254);
    bh1750_read_lux(&g_bh1750, &lux);
    printf("Light Intensity: %.2f lux\n", lux);

    HAL_Delay(1000);

    printf("Low Sensitivity (MTReg=31):\n");
    bh1750_set_sensitivity(&g_bh1750, 31);
    bh1750_read_lux(&g_bh1750, &lux);
    printf("Light Intensity: %.2f lux\n", lux);

    HAL_Delay(1000);

    bh1750_set_sensitivity(&g_bh1750, 69);
    printf("Sensitivity restored to default\n");
}

void BH1750_Demo_Power_Management(void)
{
    float lux;

    printf("Reading light...\n");
    bh1750_read_lux(&g_bh1750, &lux);
    printf("Light Intensity: %.2f lux\n", lux);

    HAL_Delay(1000);

    printf("Powering down...\n");
    bh1750_power_down(&g_bh1750);

    HAL_Delay(2000);

    printf("Powering on...\n");
    bh1750_power_on(&g_bh1750);

    HAL_Delay(100);

    printf("Reading light again...\n");
    bh1750_read_lux(&g_bh1750, &lux);
    printf("Light Intensity: %.2f lux\n", lux);
}
