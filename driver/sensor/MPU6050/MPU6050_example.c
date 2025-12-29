/**
 * @file MPU6050_example.c
 * @brief MPU6050 驱动使用示例 (基于 STM32 HAL 库)
 * @note  本文件仅供参考，展示如何适配接口和调用API
 */

#include "mpu6050_driver.h"
#include "stm32f4xx_hal.h" // 假设使用STM32F4 HAL库

/* 假设已定义的全局变量 */
extern I2C_HandleTypeDef hi2c1;

/* --- 1. 接口适配 (Interface Adapters) --- */

/* I2C 写适配器 */
static driver_status_t my_i2c_write(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)p_data, len, 100);
    return (ret == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

/* I2C 读适配器 */
static driver_status_t my_i2c_read(void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len)
{
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, p_data, len, 100);
    return (ret == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

/* 毫秒延时适配器 */
static void my_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/* --- 2. 使用示例 (Usage Example) --- */

/* 定义设备句柄 (通常放在全局或静态区) */
static mpu6050_dev_t g_mpu6050;

void MPU6050_Demo_Init(void)
{
    /* 1. 准备接口操作集 */
    driver_i2c_ops_t i2c_ops = {
        .write_reg = my_i2c_write,
        .read_reg  = my_i2c_read
    };
    
    driver_time_ops_t time_ops = {
        .delay_ms = my_delay_ms
    };

    /* 2. 准备配置参数 */
    mpu6050_config_t config = {
        .gyro_fs         = MPU6050_GYRO_FS_2000,  // 陀螺仪量程 ±2000dps
        .accel_fs        = MPU6050_ACCEL_FS_8,    // 加速度计量程 ±8g
        .dlpf_cfg        = MPU6050_DLPF_BW_44,    // 带宽 44Hz
        .sample_rate_div = 9                      // 采样率 = 1kHz / (1+9) = 100Hz
    };

    /* 3. 初始化驱动 */
    driver_status_t ret = mpu6050_init(&g_mpu6050, 
                                       &i2c_ops, 
                                       &time_ops, 
                                       &hi2c1,             // 传入HAL库I2C句柄
                                       MPU6050_ADDR_AD0_LOW, 
                                       &config);

    if (ret != DRV_OK) {
        // 初始化失败处理...
        printf("MPU6050 Init Failed: %d\n", ret);
    } else {
        printf("MPU6050 Init Success!\n");
    }
}

void MPU6050_Demo_Loop(void)
{
    mpu6050_float_data_t accel, gyro;
    float temp;

    /* 读取所有数据 */
    if (mpu6050_read_all(&g_mpu6050, &accel, &gyro, &temp) == DRV_OK) {
        /* 打印数据 */
        printf("Accel: %.2f, %.2f, %.2f g\n", accel.x, accel.y, accel.z);
        printf("Gyro:  %.2f, %.2f, %.2f dps\n", gyro.x, gyro.y, gyro.z);
        printf("Temp:  %.2f C\n", temp);
    } else {
        printf("Read Failed\n");
    }

    HAL_Delay(100); // 10Hz 打印频率
}
