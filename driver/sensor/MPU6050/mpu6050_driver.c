/**
 * @file mpu6050_driver.c
 * @brief MPU6050 6轴运动处理组件驱动实现文件
 * @note  遵循“无隐式状态”和“严格分层”原则
 */

#include "mpu6050_driver.h"

/* --- 1. 私有宏定义 (Private Macros) --- */

/* 寄存器地址 */
#define MPU6050_REG_SMPLRT_DIV      0x19
#define MPU6050_REG_CONFIG          0x1A
#define MPU6050_REG_GYRO_CONFIG     0x1B
#define MPU6050_REG_ACCEL_CONFIG    0x1C
#define MPU6050_REG_ACCEL_XOUT_H    0x3B
#define MPU6050_REG_TEMP_OUT_H      0x41
#define MPU6050_REG_GYRO_XOUT_H     0x43
#define MPU6050_REG_PWR_MGMT_1      0x6B
#define MPU6050_REG_WHO_AM_I        0x75

/* 寄存器位掩码 */
#define MPU6050_WHO_AM_I_VAL        0x68
#define MPU6050_PWR1_DEVICE_RESET   0x80
#define MPU6050_PWR1_SLEEP          0x40
#define MPU6050_PWR1_CLKSEL_PLL_X   0x01

/* --- 2. 私有辅助函数声明 (Private Function Prototypes) --- */
static driver_status_t static_write_reg(mpu6050_dev_t *p_dev, uint8_t reg, uint8_t data);
static driver_status_t static_read_regs(mpu6050_dev_t *p_dev, uint8_t reg, uint8_t *p_data, uint32_t len);

/* --- 3. API 实现 (API Implementation) --- */

driver_status_t mpu6050_init(mpu6050_dev_t *p_dev, 
                             const driver_i2c_ops_t *p_i2c_ops, 
                             const driver_time_ops_t *p_time_ops,
                             void *bus_ctx, 
                             uint8_t addr,
                             const mpu6050_config_t *p_cfg)
{
    /* 1. 参数检查 */
    if (p_dev == NULL || p_i2c_ops == NULL || p_time_ops == NULL || bus_ctx == NULL || p_cfg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    if (p_i2c_ops->write_reg == NULL || p_i2c_ops->read_reg == NULL || p_time_ops->delay_ms == NULL) {
        return DRV_ERR_NOT_SUPPORT;
    }

    /* 2. 对象初始化 */
    p_dev->i2c_ops = *p_i2c_ops;
    p_dev->time_ops = *p_time_ops;
    p_dev->bus_ctx = bus_ctx;
    p_dev->dev_addr = addr;
    p_dev->is_initialized = false;

    /* 3. 复位设备 */
    if (mpu6050_reset(p_dev) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* 4. ID 校验 */
    uint8_t who_am_i;
    if (static_read_regs(p_dev, MPU6050_REG_WHO_AM_I, &who_am_i, 1) != DRV_OK) {
        return DRV_ERR_IO;
    }
    // MPU6050 WHO_AM_I 默认值通常是 0x68 (bit 6-1), 但有些版本可能不同，这里做弱校验或打印警告
    // 严格校验: if ((who_am_i & 0x7E) >> 1 != 0x34) ... (取决于具体芯片版本，通常0x68是I2C地址，寄存器值也是0x68)
    if (who_am_i != MPU6050_WHO_AM_I_VAL) {
        // return DRV_ERR_COMMON; // 视情况是否严格报错
    }

    /* 5. 解除休眠并设置时钟源 (PLL with X axis gyro reference) */
    if (static_write_reg(p_dev, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR1_CLKSEL_PLL_X) != DRV_OK) {
        return DRV_ERR_IO;
    }
    p_dev->time_ops.delay_ms(10);

    /* 6. 配置采样率分频 */
    if (static_write_reg(p_dev, MPU6050_REG_SMPLRT_DIV, p_cfg->sample_rate_div) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* 7. 配置 DLPF */
    if (static_write_reg(p_dev, MPU6050_REG_CONFIG, p_cfg->dlpf_cfg) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* 8. 配置陀螺仪量程 */
    uint8_t gyro_config = (p_cfg->gyro_fs << 3);
    if (static_write_reg(p_dev, MPU6050_REG_GYRO_CONFIG, gyro_config) != DRV_OK) {
        return DRV_ERR_IO;
    }
    
    /* 更新灵敏度 */
    switch (p_cfg->gyro_fs) {
        case MPU6050_GYRO_FS_250:  p_dev->gyro_sensitivity = 131.0f; break;
        case MPU6050_GYRO_FS_500:  p_dev->gyro_sensitivity = 65.5f; break;
        case MPU6050_GYRO_FS_1000: p_dev->gyro_sensitivity = 32.8f; break;
        case MPU6050_GYRO_FS_2000: p_dev->gyro_sensitivity = 16.4f; break;
        default: p_dev->gyro_sensitivity = 131.0f; break;
    }

    /* 9. 配置加速度计量程 */
    uint8_t accel_config = (p_cfg->accel_fs << 3);
    if (static_write_reg(p_dev, MPU6050_REG_ACCEL_CONFIG, accel_config) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* 更新灵敏度 */
    switch (p_cfg->accel_fs) {
        case MPU6050_ACCEL_FS_2:  p_dev->accel_sensitivity = 16384; break;
        case MPU6050_ACCEL_FS_4:  p_dev->accel_sensitivity = 8192; break;
        case MPU6050_ACCEL_FS_8:  p_dev->accel_sensitivity = 4096; break;
        case MPU6050_ACCEL_FS_16: p_dev->accel_sensitivity = 2048; break;
        default: p_dev->accel_sensitivity = 16384; break;
    }

    p_dev->is_initialized = true;
    return DRV_OK;
}

driver_status_t mpu6050_reset(mpu6050_dev_t *p_dev)
{
    if (p_dev == NULL) return DRV_ERR_INVALID_VAL;
    
    /* 写入复位位 */
    if (static_write_reg(p_dev, MPU6050_REG_PWR_MGMT_1, MPU6050_PWR1_DEVICE_RESET) != DRV_OK) {
        return DRV_ERR_IO;
    }
    
    /* 等待复位完成 */
    p_dev->time_ops.delay_ms(100);
    return DRV_OK;
}

driver_status_t mpu6050_read_accel(mpu6050_dev_t *p_dev, mpu6050_raw_data_t *p_raw, mpu6050_float_data_t *p_val)
{
    if (p_dev == NULL || !p_dev->is_initialized) return DRV_ERR_INVALID_VAL;
    
    uint8_t buf[6];
    if (static_read_regs(p_dev, MPU6050_REG_ACCEL_XOUT_H, buf, 6) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* 转换原始数据 (Big Endian) */
    int16_t raw_x = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t raw_y = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t raw_z = (int16_t)((buf[4] << 8) | buf[5]);

    if (p_raw) {
        p_raw->x = raw_x;
        p_raw->y = raw_y;
        p_raw->z = raw_z;
    }

    if (p_val) {
        p_val->x = (float)raw_x / p_dev->accel_sensitivity;
        p_val->y = (float)raw_y / p_dev->accel_sensitivity;
        p_val->z = (float)raw_z / p_dev->accel_sensitivity;
    }

    return DRV_OK;
}

driver_status_t mpu6050_read_gyro(mpu6050_dev_t *p_dev, mpu6050_raw_data_t *p_raw, mpu6050_float_data_t *p_val)
{
    if (p_dev == NULL || !p_dev->is_initialized) return DRV_ERR_INVALID_VAL;
    
    uint8_t buf[6];
    if (static_read_regs(p_dev, MPU6050_REG_GYRO_XOUT_H, buf, 6) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* 转换原始数据 (Big Endian) */
    int16_t raw_x = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t raw_y = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t raw_z = (int16_t)((buf[4] << 8) | buf[5]);

    if (p_raw) {
        p_raw->x = raw_x;
        p_raw->y = raw_y;
        p_raw->z = raw_z;
    }

    if (p_val) {
        p_val->x = (float)raw_x / p_dev->gyro_sensitivity;
        p_val->y = (float)raw_y / p_dev->gyro_sensitivity;
        p_val->z = (float)raw_z / p_dev->gyro_sensitivity;
    }

    return DRV_OK;
}

driver_status_t mpu6050_read_temp(mpu6050_dev_t *p_dev, float *p_temp)
{
    if (p_dev == NULL || !p_dev->is_initialized) return DRV_ERR_INVALID_VAL;

    uint8_t buf[2];
    if (static_read_regs(p_dev, MPU6050_REG_TEMP_OUT_H, buf, 2) != DRV_OK) {
        return DRV_ERR_IO;
    }

    int16_t raw_temp = (int16_t)((buf[0] << 8) | buf[1]);
    
    /* 温度计算公式: Temp = (Raw / 340) + 36.53 */
    if (p_temp) {
        *p_temp = ((float)raw_temp / 340.0f) + 36.53f;
    }

    return DRV_OK;
}

driver_status_t mpu6050_read_all(mpu6050_dev_t *p_dev, 
                                 mpu6050_float_data_t *p_accel, 
                                 mpu6050_float_data_t *p_gyro, 
                                 float *p_temp)
{
    if (p_dev == NULL || !p_dev->is_initialized) return DRV_ERR_INVALID_VAL;

    /* 连续读取14个字节: Accel(6) + Temp(2) + Gyro(6) */
    uint8_t buf[14];
    if (static_read_regs(p_dev, MPU6050_REG_ACCEL_XOUT_H, buf, 14) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* Accel */
    if (p_accel) {
        int16_t ax = (int16_t)((buf[0] << 8) | buf[1]);
        int16_t ay = (int16_t)((buf[2] << 8) | buf[3]);
        int16_t az = (int16_t)((buf[4] << 8) | buf[5]);
        p_accel->x = (float)ax / p_dev->accel_sensitivity;
        p_accel->y = (float)ay / p_dev->accel_sensitivity;
        p_accel->z = (float)az / p_dev->accel_sensitivity;
    }

    /* Temp */
    if (p_temp) {
        int16_t t = (int16_t)((buf[6] << 8) | buf[7]);
        *p_temp = ((float)t / 340.0f) + 36.53f;
    }

    /* Gyro */
    if (p_gyro) {
        int16_t gx = (int16_t)((buf[8] << 8) | buf[9]);
        int16_t gy = (int16_t)((buf[10] << 8) | buf[11]);
        int16_t gz = (int16_t)((buf[12] << 8) | buf[13]);
        p_gyro->x = (float)gx / p_dev->gyro_sensitivity;
        p_gyro->y = (float)gy / p_dev->gyro_sensitivity;
        p_gyro->z = (float)gz / p_dev->gyro_sensitivity;
    }

    return DRV_OK;
}

/* --- 4. 私有辅助函数实现 (Private Functions) --- */

static driver_status_t static_write_reg(mpu6050_dev_t *p_dev, uint8_t reg, uint8_t data)
{
    return p_dev->i2c_ops.write_reg(p_dev->bus_ctx, p_dev->dev_addr, reg, &data, 1);
}

static driver_status_t static_read_regs(mpu6050_dev_t *p_dev, uint8_t reg, uint8_t *p_data, uint32_t len)
{
    return p_dev->i2c_ops.read_reg(p_dev->bus_ctx, p_dev->dev_addr, reg, p_data, len);
}
