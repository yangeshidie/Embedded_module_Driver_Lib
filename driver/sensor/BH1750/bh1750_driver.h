/**
 * @file bh1750_driver.h
 * @brief BH1750FVI光照传感器驱动 (BH1750FVI Ambient Light Sensor Driver)
 * @note  遵循PDA设计原则：驱动逻辑与硬件接口分离
 */

#ifndef _BH1750_DRIVER_H_
#define _BH1750_DRIVER_H_

#include "../../../core/driver_types.h"
#include "../../../core/driver_interfaces.h"
#include <stdint.h>

/* --- 1. 设备地址定义 --- */
#define BH1750_ADDR_DEFAULT    0x46  /* 默认I2C地址 (7位地址) */

/* --- 2. 操作码定义 --- */
#define BH1750_CMD_POWER_DOWN  0x00  /* 断电模式 */
#define BH1750_CMD_POWER_ON    0x01  /* 上电 */
#define BH1750_CMD_RESET       0x07  /* 重置数据寄存器 (仅Power Down模式有效) */

/* 连续测量模式 */
#define BH1750_CMD_CON_H_RES   0x10  /* 连续高分辨率模式 (1 lux, 典型转换时间120ms) */
#define BH1750_CMD_CON_H_RES2  0x11  /* 连续高分辨率模式2 (0.5 lux, 典型转换时间120ms) */
#define BH1750_CMD_CON_L_RES   0x13  /* 连续低分辨率模式 (4 lux, 典型转换时间16ms) */

/* 单次测量模式 */
#define BH1750_CMD_ONE_H_RES   0x20  /* 单次高分辨率模式 (测量后自动Power Down) */
#define BH1750_CMD_ONE_H_RES2  0x21  /* 单次高分辨率模式2 (测量后自动Power Down) */
#define BH1750_CMD_ONE_L_RES   0x23  /* 单次低分辨率模式 (测量后自动Power Down) */

/* 测量时间高字节 (用于灵敏度调节) */
#define BH1750_CMD_MTREG_HIGH  0x40
/* 测量时间低字节 (用于灵敏度调节) */
#define BH1750_CMD_MTREG_LOW   0x60

/* --- 3. 测量模式枚举 --- */
typedef enum {
    BH1750_MODE_CON_H_RES  = 1,  /**< 连续高分辨率模式 (1 lux) */
    BH1750_MODE_CON_H_RES2 = 2,  /**< 连续高分辨率模式2 (0.5 lux) */
    BH1750_MODE_CON_L_RES  = 3,  /**< 连续低分辨率模式 (4 lux) */
    BH1750_MODE_ONE_H_RES  = 4,  /**< 单次高分辨率模式 */
    BH1750_MODE_ONE_H_RES2 = 5,  /**< 单次高分辨率模式2 */
    BH1750_MODE_ONE_L_RES  = 6   /**< 单次低分辨率模式 */
} bh1750_mode_t;

/* --- 4. 配置结构体 --- */
typedef struct {
    uint8_t mt_reg;  /**< 测量时间寄存器 (31-254, 默认69) */
} bh1750_config_t;

/* --- 5. 设备句柄结构体 --- */
typedef struct {
    /* 接口操作集 */
    const driver_i2c_ops_t *p_i2c_ops;
    const driver_time_ops_t *p_time_ops;

    /* 硬件上下文 */
    void *i2c_ctx;     /**< I2C总线句柄 (如I2C_HandleTypeDef*) */
    uint8_t dev_addr;  /**< I2C设备地址 */

    /* 设备状态 */
    bh1750_mode_t mode;    /**< 当前测量模式 */
    uint8_t mt_reg;       /**< 当前测量时间寄存器值 */
    uint8_t is_initialized; /**< 初始化标志 */

    void *reserved;        /**< 预留字段，用于扩展 */
} bh1750_dev_t;

/* --- 6. API函数声明 --- */

/**
 * @brief 初始化BH1750设备
 * @param p_dev 设备句柄指针
 * @param p_i2c_ops I2C操作接口
 * @param p_time_ops 时间操作接口
 * @param i2c_ctx I2C总线上下文
 * @param dev_addr I2C设备地址
 * @param p_config 配置参数 (NULL使用默认配置)
 * @return driver_status_t
 */
driver_status_t bh1750_init(bh1750_dev_t *p_dev, 
                            const driver_i2c_ops_t *p_i2c_ops,
                            const driver_time_ops_t *p_time_ops,
                            void *i2c_ctx,
                            uint8_t dev_addr,
                            const bh1750_config_t *p_config);

/**
 * @brief 断电
 * @param p_dev 设备句柄指针
 * @return driver_status_t
 */
driver_status_t bh1750_power_down(bh1750_dev_t *p_dev);

/**
 * @brief 上电
 * @param p_dev 设备句柄指针
 * @return driver_status_t
 */
driver_status_t bh1750_power_on(bh1750_dev_t *p_dev);

/**
 * @brief 重置数据寄存器 (仅Power Down模式有效)
 * @param p_dev 设备句柄指针
 * @return driver_status_t
 */
driver_status_t bh1750_reset(bh1750_dev_t *p_dev);

/**
 * @brief 设置测量模式
 * @param p_dev 设备句柄指针
 * @param mode 测量模式
 * @return driver_status_t
 */
driver_status_t bh1750_set_mode(bh1750_dev_t *p_dev, bh1750_mode_t mode);

/**
 * @brief 调节测量灵敏度 (通过修改测量时间寄存器)
 * @param p_dev 设备句柄指针
 * @param mt_reg 测量时间寄存器值 (31-254)
 * @return driver_status_t
 */
driver_status_t bh1750_set_sensitivity(bh1750_dev_t *p_dev, uint8_t mt_reg);

/**
 * @brief 读取原始光照数据 (未转换为Lux)
 * @param p_dev 设备句柄指针
 * @param p_raw_data 原始数据输出
 * @return driver_status_t
 * @note 连续模式下，首次测量后需等待120ms才能读取有效数据
 */
driver_status_t bh1750_read_raw(bh1750_dev_t *p_dev, uint16_t *p_raw_data);

/**
 * @brief 读取光照强度 (Lux)
 * @param p_dev 设备句柄指针
 * @param p_lux 光照强度输出
 * @return driver_status_t
 */
driver_status_t bh1750_read_lux(bh1750_dev_t *p_dev, float *p_lux);

/**
 * @brief 单次测量光照强度 (Lux)
 * @param p_dev 设备句柄指针
 * @param p_lux 光照强度输出
 * @return driver_status_t
 * @note 此函数会自动切换到单次测量模式，读取完成后自动断电
 */
driver_status_t bh1750_measure_lux(bh1750_dev_t *p_dev, float *p_lux);

#endif /* _BH1750_DRIVER_H_ */
