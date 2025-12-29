/**
 * @file mpu6050_driver.h
 * @brief MPU6050 6轴运动处理组件驱动头文件
 * @note  遵循“无隐式状态”和“严格分层”原则
 *        本驱动不包含任何硬件相关代码(HAL/GPIO/I2C)，所有硬件操作通过接口注入
 */

#ifndef _MPU6050_DRIVER_H_
#define _MPU6050_DRIVER_H_

#include "../../../core/driver_types.h"
#include "../../../core/driver_interfaces.h"

/* --- 1. 宏定义 (Macros) --- */
#define MPU6050_ADDR_AD0_LOW     0x68  /**< AD0引脚接GND时的I2C地址 */
#define MPU6050_ADDR_AD0_HIGH    0x69  /**< AD0引脚接VCC时的I2C地址 */

/* --- 2. 枚举定义 (Enumerations) --- */

/**
 * @brief 陀螺仪量程选择
 */
typedef enum {
    MPU6050_GYRO_FS_250  = 0,  /**< ±250 dps */
    MPU6050_GYRO_FS_500  = 1,  /**< ±500 dps */
    MPU6050_GYRO_FS_1000 = 2,  /**< ±1000 dps */
    MPU6050_GYRO_FS_2000 = 3   /**< ±2000 dps */
} mpu6050_gyro_fs_t;

/**
 * @brief 加速度计量程选择
 */
typedef enum {
    MPU6050_ACCEL_FS_2   = 0,  /**< ±2g */
    MPU6050_ACCEL_FS_4   = 1,  /**< ±4g */
    MPU6050_ACCEL_FS_8   = 2,  /**< ±8g */
    MPU6050_ACCEL_FS_16  = 3   /**< ±16g */
} mpu6050_accel_fs_t;

/**
 * @brief 数字低通滤波器(DLPF)带宽配置
 */
typedef enum {
    MPU6050_DLPF_BW_260  = 0,  /**< Accel: 260Hz, Gyro: 256Hz */
    MPU6050_DLPF_BW_184  = 1,  /**< Accel: 184Hz, Gyro: 188Hz */
    MPU6050_DLPF_BW_94   = 2,  /**< Accel: 94Hz,  Gyro: 98Hz */
    MPU6050_DLPF_BW_44   = 3,  /**< Accel: 44Hz,  Gyro: 42Hz */
    MPU6050_DLPF_BW_21   = 4,  /**< Accel: 21Hz,  Gyro: 20Hz */
    MPU6050_DLPF_BW_10   = 5,  /**< Accel: 10Hz,  Gyro: 10Hz */
    MPU6050_DLPF_BW_5    = 6   /**< Accel: 5Hz,   Gyro: 5Hz */
} mpu6050_dlpf_cfg_t;

/* --- 3. 配置结构体 (Configuration Struct) --- */

/**
 * @brief MPU6050 初始化配置参数
 */
typedef struct {
    mpu6050_gyro_fs_t  gyro_fs;    /**< 陀螺仪量程 */
    mpu6050_accel_fs_t accel_fs;   /**< 加速度计量程 */
    mpu6050_dlpf_cfg_t dlpf_cfg;   /**< 低通滤波器配置 */
    uint8_t            sample_rate_div; /**< 采样率分频 (Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)) */
} mpu6050_config_t;

/* --- 4. 设备句柄 (Device Handle) --- */

/**
 * @brief MPU6050 设备对象
 * @note  内存由调用者管理
 */
typedef struct {
    /* 依赖接口 (必须注入) */
    driver_i2c_ops_t  i2c_ops;     /**< I2C读写接口 */
    driver_time_ops_t time_ops;    /**< 延时接口 */
    
    /* 运行状态 */
    void     *bus_ctx;             /**< 总线上下文 (如 I2C_HandleTypeDef*) */
    uint8_t   dev_addr;            /**< 设备I2C地址 */
    bool      is_initialized;      /**< 初始化标志 */
    
    /* 当前配置副本 (用于数据转换) */
    float     gyro_sensitivity;    /**< 陀螺仪灵敏度 (LSB/dps) */
    uint16_t  accel_sensitivity;   /**< 加速度计灵敏度 (LSB/g) */
} mpu6050_dev_t;

/* --- 5. 数据结构 (Data Structs) --- */

/**
 * @brief 3轴原始数据 (int16_t)
 */
typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} mpu6050_raw_data_t;

/**
 * @brief 3轴浮点数据 (物理量)
 */
typedef struct {
    float x;
    float y;
    float z;
} mpu6050_float_data_t;

/* --- 6. API 函数声明 (Function Prototypes) --- */

/**
 * @brief 初始化 MPU6050
 * @param p_dev 设备句柄指针
 * @param p_i2c_ops I2C接口指针
 * @param p_time_ops 时间接口指针
 * @param bus_ctx 总线上下文
 * @param addr 设备地址 (MPU6050_ADDR_AD0_LOW 或 MPU6050_ADDR_AD0_HIGH)
 * @param p_cfg 初始化配置
 * @return DRV_OK 成功, 其他失败
 */
driver_status_t mpu6050_init(mpu6050_dev_t *p_dev, 
                             const driver_i2c_ops_t *p_i2c_ops, 
                             const driver_time_ops_t *p_time_ops,
                             void *bus_ctx, 
                             uint8_t addr,
                             const mpu6050_config_t *p_cfg);

/**
 * @brief 复位设备 (软复位)
 * @param p_dev 设备句柄指针
 * @return driver_status_t
 */
driver_status_t mpu6050_reset(mpu6050_dev_t *p_dev);

/**
 * @brief 读取加速度计数据
 * @param p_dev 设备句柄指针
 * @param p_raw 原始数据输出 (可选, NULL则不输出)
 * @param p_val 物理量数据输出 (单位: g)
 * @return driver_status_t
 */
driver_status_t mpu6050_read_accel(mpu6050_dev_t *p_dev, mpu6050_raw_data_t *p_raw, mpu6050_float_data_t *p_val);

/**
 * @brief 读取陀螺仪数据
 * @param p_dev 设备句柄指针
 * @param p_raw 原始数据输出 (可选, NULL则不输出)
 * @param p_val 物理量数据输出 (单位: dps)
 * @return driver_status_t
 */
driver_status_t mpu6050_read_gyro(mpu6050_dev_t *p_dev, mpu6050_raw_data_t *p_raw, mpu6050_float_data_t *p_val);

/**
 * @brief 读取温度
 * @param p_dev 设备句柄指针
 * @param p_temp 温度输出 (单位: 摄氏度)
 * @return driver_status_t
 */
driver_status_t mpu6050_read_temp(mpu6050_dev_t *p_dev, float *p_temp);

/**
 * @brief 读取所有传感器数据 (Accel + Gyro + Temp)
 * @note  推荐使用此函数，效率最高 (一次I2C读取)
 * @param p_dev 设备句柄指针
 * @param p_accel 加速度输出 (g)
 * @param p_gyro 陀螺仪输出 (dps)
 * @param p_temp 温度输出 (C)
 * @return driver_status_t
 */
driver_status_t mpu6050_read_all(mpu6050_dev_t *p_dev, 
                                 mpu6050_float_data_t *p_accel, 
                                 mpu6050_float_data_t *p_gyro, 
                                 float *p_temp);

#endif /* _MPU6050_DRIVER_H_ */
