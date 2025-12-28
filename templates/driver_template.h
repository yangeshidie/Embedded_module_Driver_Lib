/**
 * @file driver_template.h
 * @brief [模块名] 驱动头文件模板
 * @author [Your Name]
 * @date [Year-Month-Day]
 */

#ifndef _DRIVER_TEMPLATE_H_
#define _DRIVER_TEMPLATE_H_

#include "../core/driver_types.h"
#include "../core/driver_interfaces.h"

/* --- 1. 配置结构体 (Configuration Struct) --- */
/**
 * @brief 设备静态配置参数
 * @note  通常在初始化时确定，运行时不变更
 */
typedef struct {
    uint8_t sample_rate;    /**< 采样率配置值 */
    uint8_t range;          /**< 量程配置值 */
    // ... 其他配置项
} template_cfg_t;

/* --- 2. 依赖注入结构体 (Operations Struct) --- */
/**
 * @brief 外部依赖接口
 * @note  必须在初始化时由上层应用注入
 */
typedef struct {
    /* 必选依赖 */
    driver_i2c_ops_t i2c_ops;   /**< I2C读写接口 */
    driver_time_ops_t time_ops; /**< 时间延时接口 */
    
    /* 可选依赖 (使用前需判空) */
    void (*debug_print)(const char *fmt, ...);
} template_ops_t;

/* --- 3. 设备句柄 (Device Handle) --- */
/**
 * @brief 设备主对象
 * @note  内存由调用者管理(栈/静态区)，严禁驱动内部malloc
 */
typedef struct {
    /* 私有成员 (应用层勿动) */
    template_ops_t ops;     /**< 保存注入的接口 */
    void *bus_ctx;          /**< 保存总线上下文 (透传给I2C/SPI函数) */
    bool is_initialized;    /**< 初始化标志 */
    
    /* 公有成员 (只读状态) */
    uint8_t dev_addr;       /**< 设备I2C地址 */
    template_cfg_t config;  /**< 当前配置副本 */
} template_dev_t;

/* --- 4. API 函数声明 (Function Prototypes) --- */

/**
 * @brief 初始化设备
 * @param p_dev 设备句柄指针
 * @param p_ops 接口操作集指针
 * @param bus_ctx 总线上下文 (如 &hi2c1)
 * @param addr 设备地址
 * @return DRV_OK 成功, 其他失败
 */
driver_status_t template_init(template_dev_t *p_dev, const template_ops_t *p_ops, void *bus_ctx, uint8_t addr);

/**
 * @brief 读取传感器数据示例
 * @param p_dev 设备句柄指针
 * @param p_val 输出数据指针
 * @return driver_status_t
 */
driver_status_t template_read_value(template_dev_t *p_dev, float *p_val);

/**
 * @brief 配置修改示例
 * @param p_dev 设备句柄指针
 * @param new_cfg 新配置结构体
 * @return driver_status_t
 */
driver_status_t template_set_config(template_dev_t *p_dev, template_cfg_t new_cfg);

#endif /* _DRIVER_TEMPLATE_H_ */