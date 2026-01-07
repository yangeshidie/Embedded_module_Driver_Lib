/**
 * @file driver_interfaces.h
 * @brief 硬件抽象接口定义 (Hardware Abstraction Interfaces)
 * @note  所有函数指针必须携带 void *ctx 上下文参数
 * 
 * @section design_principles 设计原则
 * 1. 驱动逻辑与硬件接口分离 - 驱动只负责业务逻辑，硬件操作通过接口注入
 * 2. 严格的能力注入 - 所有外部依赖通过函数指针注入
 * 3. 上下文与对象化 - 所有API第一个参数必须是设备句柄指针
 * 4. 内存管理所有权 - 调用者分配内存，驱动不进行动态分配
 * 5. 时序与并发抽象 - 通过注入的延时函数兼容裸机和RTOS
 * 6. 数据与平台无关性 - 使用标准定宽类型，通过移位操作处理字节序
 * 7. 统一错误码 - 返回库内定义的统一枚举
 * 8. 向后兼容性 - 所有接口结构体包含 void *reserved 预留字段
 * 
 * @section interface_categories 接口分类
 * - 通信接口: I2C, SPI, UART, CAN
 * - 控制接口: GPIO, Timer (含PWM), EXTI
 * - 模拟接口: ADC, DAC
 * - 时间接口: Time & Delay
 * - 系统可靠性: Watchdog
 * 
 * @section interrupt_management 中断管理职责划分
 * - driver_exti_ops_t: 仅管理GPIO外部中断
 * - 其他外设接口: 各自管理内部中断（通过回调函数）
 * 
 * @section interface_extension 接口扩展说明
 * 所有接口结构体均包含 void *reserved 预留字段，用于：
 * - 未来扩展新功能而不破坏二进制兼容性
 * - 存储适配层私有数据
 * - 版本控制和特性标记
 */

#ifndef _DRIVER_INTERFACES_H_
#define _DRIVER_INTERFACES_H_

#include "driver_types.h"

/* --- 通用回调类型定义 --- */
typedef void (*driver_completion_callback_t)(void *user_data, driver_status_t status);

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
    
    void *reserved;
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
    
    void *reserved;
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
     * @return driver_status_t
     */
    driver_status_t (*write_reg)(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len);

    /**
     * @brief I2C 异步写寄存器 (使用DMA或中断)
     * @param ctx 总线句柄
     * @param dev_addr 设备7位地址
     * @param reg_addr 寄存器地址
     * @param p_data 数据指针
     * @param len 数据长度
     * @param cb 完成回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*write_reg_async)(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len,
                                        driver_completion_callback_t cb, void *user_data);

    /**
     * @brief I2C 读寄存器 (阻塞式)
     * @param ctx 总线句柄 (如 I2C_HandleTypeDef*)
     * @param dev_addr 设备7位地址
     * @param reg_addr 寄存器地址
     * @param p_data 数据指针
     * @param len 数据长度
     * @return driver_status_t
     */
    driver_status_t (*read_reg)(void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len);

    /**
     * @brief I2C 异步读寄存器 (使用DMA或中断)
     * @param ctx 总线句柄
     * @param dev_addr 设备7位地址
     * @param reg_addr 寄存器地址
     * @param p_data 数据指针
     * @param len 数据长度
     * @param cb 完成回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*read_reg_async)(void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len,
                                       driver_completion_callback_t cb, void *user_data);
    
    void *reserved;
} driver_i2c_ops_t;

/* --- 4. SPI 总线接口 (SPI Bus Operations) --- */
typedef struct {
    /**
     * @brief SPI 交换数据 (阻塞式)
     * @param ctx 总线句柄
     * @param p_tx 发送缓冲区 (可为NULL)
     * @param p_rx 接收缓冲区 (可为NULL)
     * @param len 长度
     * @return driver_status_t
     */
    driver_status_t (*transfer)(void *ctx, const uint8_t *p_tx, uint8_t *p_rx, uint32_t len);

    /**
     * @brief SPI 异步交换数据 (使用DMA或中断)
     * @param ctx 总线句柄
     * @param p_tx 发送缓冲区 (可为NULL)
     * @param p_rx 接收缓冲区 (可为NULL)
     * @param len 长度
     * @param cb 完成回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*transfer_async)(void *ctx, const uint8_t *p_tx, uint8_t *p_rx, uint32_t len,
                                      driver_completion_callback_t cb, void *user_data);

    /**
     * @brief 片选控制 (可选，如果由硬件NSS管理则不需要)
     * @param ctx GPIO句柄
     * @param state 0:选中(Low), 1:释放(High)
     */
    void (*cs_control)(void *ctx, uint8_t state);
    
    void *reserved;
} driver_spi_ops_t;

/* --- 5. UART 接口 (UART Operations) --- */
typedef struct {
    /**
     * @brief UART 写数据 (阻塞式)
     * @param ctx UART句柄
     * @param p_data 发送缓冲区
     * @param len 数据长度
     * @return driver_status_t
     */
    driver_status_t (*write)(void *ctx, const uint8_t *p_data, uint32_t len);

    /**
     * @brief UART 异步写数据 (使用DMA或中断)
     * @param ctx UART句柄
     * @param p_data 发送缓冲区
     * @param len 数据长度
     * @param cb 完成回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*write_async)(void *ctx, const uint8_t *p_data, uint32_t len,
                                    driver_completion_callback_t cb, void *user_data);

    /**
     * @brief UART 读数据 (阻塞式)
     * @param ctx UART句柄
     * @param p_data 接收缓冲区
     * @param len 数据长度
     * @return driver_status_t
     */
    driver_status_t (*read)(void *ctx, uint8_t *p_data, uint32_t len);

    /**
     * @brief UART 异步读数据 (使用DMA或中断)
     * @param ctx UART句柄
     * @param p_data 接收缓冲区
     * @param len 数据长度
     * @param cb 完成回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*read_async)(void *ctx, uint8_t *p_data, uint32_t len,
                                   driver_completion_callback_t cb, void *user_data);

    /**
     * @brief 获取可用数据长度 (非阻塞)
     * @param ctx UART句柄
     * @return 可用字节数
     */
    uint32_t (*available)(void *ctx);
    
    void *reserved;
} driver_uart_ops_t;

/* --- 6. 定时器接口 (Timer & PWM Operations) --- */
typedef void (*timer_callback_t)(void *user_data);

typedef struct {
    /* --- 基本定时器功能 --- */
    
    /**
     * @brief 启动定时器（周期模式）
     * @param ctx 定时器句柄 (如 TIM_HandleTypeDef*)
     * @param period_us 周期时间 (微秒)
     * @param cb 周期回调函数 (可选，NULL则不使用回调)
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*start_periodic)(void *ctx, uint32_t period_us, timer_callback_t cb, void *user_data);

    /**
     * @brief 启动定时器（单次模式）
     * @param ctx 定时器句柄
     * @param timeout_us 超时时间 (微秒)
     * @param cb 超时回调函数 (可选，NULL则不使用回调)
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*start_one_shot)(void *ctx, uint32_t timeout_us, timer_callback_t cb, void *user_data);

    /**
     * @brief 停止定时器
     * @param ctx 定时器句柄
     * @return driver_status_t
     */
    driver_status_t (*stop)(void *ctx);

    /**
     * @brief 获取当前计数值
     * @param ctx 定时器句柄
     * @return 当前计数值
     */
    uint32_t (*get_counter)(void *ctx);

    /* --- 输入捕获功能 --- */

    /**
     * @brief 设置输入捕获回调
     * @param ctx 定时器句柄
     * @param channel 捕获通道 (1, 2, 3, 4...)
     * @param cb 捕获回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*set_capture_callback)(void *ctx, uint8_t channel, timer_callback_t cb, void *user_data);

    /**
     * @brief 获取捕获值
     * @param ctx 定时器句柄
     * @param channel 捕获通道
     * @param p_value 捕获值输出
     * @return driver_status_t
     */
    driver_status_t (*get_capture_value)(void *ctx, uint8_t channel, uint32_t *p_value);

    /* --- PWM输出功能 --- */

    /**
     * @brief 设置 PWM 占空比
     * @param ctx 定时器句柄 (如 TIM_HandleTypeDef*)
     * @param channel PWM通道 (1, 2, 3, 4...)
     * @param duty 占空比 (0.0 ~ 1.0)
     * @return driver_status_t
     */
    driver_status_t (*pwm_set_duty)(void *ctx, uint8_t channel, float duty);

    /**
     * @brief 设置 PWM 频率
     * @param ctx 定时器句柄
     * @param frequency_hz 频率 (Hz)
     * @return driver_status_t
     */
    driver_status_t (*pwm_set_freq)(void *ctx, uint32_t frequency_hz);

    /**
     * @brief 启动 PWM 输出
     * @param ctx 定时器句柄
     * @param channel PWM通道
     * @return driver_status_t
     */
    driver_status_t (*pwm_start)(void *ctx, uint8_t channel);

    /**
     * @brief 停止 PWM 输出
     * @param ctx 定时器句柄
     * @param channel PWM通道
     * @return driver_status_t
     */
    driver_status_t (*pwm_stop)(void *ctx, uint8_t channel);
    
    void *reserved;
} driver_timer_ops_t;

/* --- 7. 外部中断接口 (External Interrupt - EXTI Operations) --- */
typedef void (*exti_callback_t)(void *user_data);

typedef struct {
    /**
     * @brief 使能外部中断并注册回调 (GPIO中断)
     * @param ctx 外部中断句柄 (如 EXTI_HandleTypeDef*)
     * @param pin_num 引脚号 (0-15)
     * @param trigger 触发方式 (0: 上升沿, 1: 下降沿, 2: 双边沿)
     * @param cb 中断回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     * @note 仅用于管理GPIO外部中断，不包含外设内部中断
     */
    driver_status_t (*enable)(void *ctx, uint8_t pin_num, uint8_t trigger, 
                               exti_callback_t cb, void *user_data);

    /**
     * @brief 禁用外部中断
     * @param ctx 外部中断句柄
     * @param pin_num 引脚号 (0-15)
     * @return driver_status_t
     */
    driver_status_t (*disable)(void *ctx, uint8_t pin_num);

    /**
     * @brief 清除中断标志
     * @param ctx 外部中断句柄
     * @param pin_num 引脚号 (0-15)
     * @return driver_status_t
     */
    driver_status_t (*clear)(void *ctx, uint8_t pin_num);
    
    void *reserved;
} driver_exti_ops_t;

/* --- 8. ADC 接口 (ADC Operations) --- */
typedef struct {
    /**
     * @brief 读取 ADC 通道原始值
     * @param ctx ADC句柄 (如 ADC_HandleTypeDef*)
     * @param channel ADC通道
     * @param p_value ADC原始值输出
     * @return driver_status_t
     */
    driver_status_t (*read_channel)(void *ctx, uint8_t channel, uint16_t *p_value);

    /**
     * @brief 读取 ADC 通道并转换为电压
     * @param ctx ADC句柄
     * @param channel ADC通道
     * @param p_voltage 电压值输出 (V)
     * @return driver_status_t
     */
    driver_status_t (*read_voltage)(void *ctx, uint8_t channel, float *p_voltage);

    /**
     * @brief 启动连续采集（使用DMA或中断）
     * @param ctx ADC句柄
     * @param channel ADC通道
     * @param p_buffer 数据缓冲区
     * @param length 采集长度
     * @return driver_status_t
     */
    driver_status_t (*start_continuous)(void *ctx, uint8_t channel, uint16_t *p_buffer, uint32_t length);

    /**
     * @brief 停止连续采集
     * @param ctx ADC句柄
     * @return driver_status_t
     */
    driver_status_t (*stop_continuous)(void *ctx);

    /**
     * @brief 获取连续采集完成状态
     * @param ctx ADC句柄
     * @return true 完成, false 未完成
     */
    bool (*is_conversion_complete)(void *ctx);
    
    void *reserved;
} driver_adc_ops_t;

/* --- 9. DAC 接口 (DAC Operations) --- */
typedef struct {
    /**
     * @brief 设置 DAC 输出值（原始值）
     * @param ctx DAC句柄 (如 DAC_HandleTypeDef*)
     * @param channel DAC通道 (1, 2...)
     * @param value DAC原始值
     * @return driver_status_t
     */
    driver_status_t (*set_value)(void *ctx, uint8_t channel, uint16_t value);

    /**
     * @brief 设置 DAC 输出电压
     * @param ctx DAC句柄
     * @param channel DAC通道
     * @param voltage 电压值 (V)
     * @return driver_status_t
     */
    driver_status_t (*set_voltage)(void *ctx, uint8_t channel, float voltage);

    /**
     * @brief 启动 DAC 输出
     * @param ctx DAC句柄
     * @param channel DAC通道
     * @return driver_status_t
     */
    driver_status_t (*start)(void *ctx, uint8_t channel);

    /**
     * @brief 停止 DAC 输出
     * @param ctx DAC句柄
     * @param channel DAC通道
     * @return driver_status_t
     */
    driver_status_t (*stop)(void *ctx, uint8_t channel);

    /**
     * @brief 启动 DAC 连续输出（使用DMA或定时器触发）
     * @param ctx DAC句柄
     * @param channel DAC通道
     * @param p_data 数据缓冲区
     * @param length 数据长度
     * @param sample_rate 采样率 (Hz)
     * @param cb 完成回调函数 (可选，NULL则循环播放)
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*start_continuous)(void *ctx, uint8_t channel, const uint16_t *p_data, uint32_t length,
                                         uint32_t sample_rate, driver_completion_callback_t cb, void *user_data);

    /**
     * @brief 停止 DAC 连续输出
     * @param ctx DAC句柄
     * @param channel DAC通道
     * @return driver_status_t
     */
    driver_status_t (*stop_continuous)(void *ctx, uint8_t channel);

    /**
     * @brief 使能 DAC 波形生成（三角波/噪声波）
     * @param ctx DAC句柄
     * @param channel DAC通道
     * @param wave_type 波形类型 (0: 禁用, 1: 噪声波, 2: 三角波)
     * @return driver_status_t
     */
    driver_status_t (*enable_wave)(void *ctx, uint8_t channel, uint8_t wave_type);
    
    void *reserved;
} driver_dac_ops_t;

/* --- 10. CAN 总线接口 (CAN Bus Operations) --- */
typedef struct {
    /**
     * @brief CAN 发送消息 (阻塞式)
     * @param ctx CAN句柄 (如 CAN_HandleTypeDef*)
     * @param id CAN ID (标准帧11位或扩展帧29位)
     * @param is_extended 是否为扩展帧 (true: 扩展帧, false: 标准帧)
     * @param p_data 数据指针
     * @param len 数据长度 (0-8字节)
     * @return driver_status_t
     */
    driver_status_t (*send)(void *ctx, uint32_t id, bool is_extended, 
                              const uint8_t *p_data, uint8_t len);

    /**
     * @brief CAN 异步发送消息 (使用DMA或中断)
     * @param ctx CAN句柄
     * @param id CAN ID
     * @param is_extended 是否为扩展帧
     * @param p_data 数据指针
     * @param len 数据长度 (0-8字节)
     * @param cb 完成回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*send_async)(void *ctx, uint32_t id, bool is_extended, 
                                    const uint8_t *p_data, uint8_t len,
                                    driver_completion_callback_t cb, void *user_data);

    /**
     * @brief CAN 接收消息 (阻塞式)
     * @param ctx CAN句柄
     * @param p_id CAN ID输出
     * @param p_is_extended 扩展帧标志输出
     * @param p_data 数据缓冲区
     * @param p_len 数据长度输出
     * @param timeout_ms 超时时间 (毫秒)
     * @return driver_status_t
     */
    driver_status_t (*receive)(void *ctx, uint32_t *p_id, bool *p_is_extended, 
                                uint8_t *p_data, uint8_t *p_len, uint32_t timeout_ms);

    /**
     * @brief CAN 异步接收消息 (使用中断)
     * @param ctx CAN句柄
     * @param p_id CAN ID输出
     * @param p_is_extended 扩展帧标志输出
     * @param p_data 数据缓冲区
     * @param p_len 数据长度输出
     * @param cb 接收回调函数
     * @param user_data 用户数据，会透传给回调
     * @return driver_status_t
     */
    driver_status_t (*receive_async)(void *ctx, uint32_t *p_id, bool *p_is_extended, 
                                       uint8_t *p_data, uint8_t *p_len,
                                       driver_completion_callback_t cb, void *user_data);

    /**
     * @brief 配置CAN过滤器 (接收过滤)
     * @param ctx CAN句柄
     * @param filter_id 过滤器ID
     * @param filter_mask 过滤器掩码
     * @param filter_index 过滤器索引
     * @param is_extended 是否为扩展帧过滤器
     * @return driver_status_t
     */
    driver_status_t (*config_filter)(void *ctx, uint32_t filter_id, uint32_t filter_mask, 
                                       uint8_t filter_index, bool is_extended);

    /**
     * @brief 启用CAN总线
     * @param ctx CAN句柄
     * @return driver_status_t
     */
    driver_status_t (*start)(void *ctx);

    /**
     * @brief 禁用CAN总线
     * @param ctx CAN句柄
     * @return driver_status_t
     */
    driver_status_t (*stop)(void *ctx);

    /**
     * @brief 获取CAN错误状态
     * @param ctx CAN句柄
     * @param p_tx_error 发送错误计数输出
     * @param p_rx_error 接收错误计数输出
     * @return driver_status_t
     */
    driver_status_t (*get_error_status)(void *ctx, uint8_t *p_tx_error, uint8_t *p_rx_error);
    
    void *reserved;
} driver_can_ops_t;

/* --- 11. 看门狗接口 (Watchdog Operations) --- */
typedef struct {
    /**
     * @brief 初始化看门狗
     * @param ctx 看门狗句柄 (如 IWDG_HandleTypeDef*)
     * @param timeout_ms 超时时间 (毫秒)
     * @return driver_status_t
     */
    driver_status_t (*init)(void *ctx, uint32_t timeout_ms);

    /**
     * @brief 启动看门狗
     * @param ctx 看门狗句柄
     * @return driver_status_t
     */
    driver_status_t (*start)(void *ctx);

    /**
     * @brief 喂狗（重置计数器）
     * @param ctx 看门狗句柄
     * @return driver_status_t
     */
    driver_status_t (*refresh)(void *ctx);

    /**
     * @brief 停止看门狗 (部分MCU支持)
     * @param ctx 看门狗句柄
     * @return driver_status_t
     * @note 独立看门狗(IWDG)通常不支持停止，窗口看门狗(WWDG)可能支持
     */
    driver_status_t (*stop)(void *ctx);

    /**
     * @brief 获取看门狗状态
     * @param ctx 看门狗句柄
     * @param p_is_enabled 是否已启用输出
     * @param p_remaining_ms 剩余时间输出 (毫秒)
     * @return driver_status_t
     */
    driver_status_t (*get_status)(void *ctx, bool *p_is_enabled, uint32_t *p_remaining_ms);
    
    void *reserved;
} driver_wdg_ops_t;

#endif /* _DRIVER_INTERFACES_H_ */