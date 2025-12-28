/**
 * @file driver_interfaces.h
 * @brief 硬件抽象接口定义 (Hardware Abstraction Interfaces)
 * @note  所有函数指针必须携带 void *ctx 上下文参数
 */

#ifndef _DRIVER_INTERFACES_H_
#define _DRIVER_INTERFACES_H_

#include "driver_types.h"

/* --- 1. 时间与延时接口 (Time & Delay) --- */
typedef struct {
    /** 
     * @brief 毫秒级延时
     * @param ms 延时时间
     */
    void (*delay_ms)(uint32_t ms);
    
    /** 
     * @brief 微秒级延时 (可选)
     */
    void (*delay_us)(uint32_t us);
    
    /** 
     * @brief 获取系统嘀嗒 (用于超时判断)
     * @return 当前系统Tick (ms)
     */
    uint32_t (*get_tick)(void);
} driver_time_ops_t;

/* --- 2. GPIO 接口 (General Purpose IO) --- */
typedef struct {
    /**
     * @brief 写引脚电平
     * @param ctx 用户上下文 (如 GPIO_TypeDef*)
     * @param state 1: High, 0: Low
     */
    void (*write_pin)(void *ctx, uint8_t state);

    /**
     * @brief 读引脚电平
     * @return 1: High, 0: Low
     */
    uint8_t (*read_pin)(void *ctx);
} driver_gpio_ops_t;

/* --- 3. I2C 总线接口 (I2C Bus Operations) --- */
typedef struct {
    /**
     * @brief I2C 写寄存器 (阻塞式)
     * @param ctx 总线句柄 (如 I2C_HandleTypeDef*)
     * @param dev_addr 设备7位地址
     * @param reg_addr 寄存器地址
     * @param p_data 数据指针
     * @param len 数据长度
     */
    int32_t (*write_reg)(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len);

    /**
     * @brief I2C 读寄存器 (阻塞式)
     */
    int32_t (*read_reg) (void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len);
} driver_i2c_ops_t;

/* --- 4. SPI 总线接口 (SPI Bus Operations) --- */
typedef struct {
    /**
     * @brief SPI 交换数据 (读写同时进行)
     * @param ctx 总线句柄
     * @param p_tx 发送缓冲区 (可为NULL)
     * @param p_rx 接收缓冲区 (可为NULL)
     * @param len 长度
     */
    int32_t (*transfer)(void *ctx, const uint8_t *p_tx, uint8_t *p_rx, uint32_t len);
    
    /**
     * @brief 片选控制 (可选，如果由硬件NSS管理则不需要)
     * @param ctx GPIO句柄
     * @param state 0:选中(Low), 1:释放(High)
     */
    void (*cs_control)(void *ctx, uint8_t state);
} driver_spi_ops_t;

#endif /* _DRIVER_INTERFACES_H_ */