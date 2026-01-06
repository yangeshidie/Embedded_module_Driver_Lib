/**
 * @file l298n_driver.h
 * @brief L298N 双路直流电机驱动头文件
 * @note  遵循"无隐式状态"和"严格分层"原则
 *        本驱动不包含任何硬件相关代码(HAL/GPIO/PWM)，所有硬件操作通过接口注入
 */

#ifndef _L298N_DRIVER_H_
#define _L298N_DRIVER_H_

#include "../../../core/driver_types.h"
#include "../../../core/driver_interfaces.h"

/* --- 1. 宏定义 (Macros) --- */

#define L298N_MOTOR_CHANNEL_1  0
#define L298N_MOTOR_CHANNEL_2  1
#define L298N_MAX_DUTY         1.0f
#define L298N_MIN_DUTY         0.0f

/* --- 2. 枚举定义 (Enumerations) --- */

/**
 * @brief 电机通道选择
 */
typedef enum {
    L298N_MOTOR_1 = 0,  /**< 电机1 */
    L298N_MOTOR_2 = 1   /**< 电机2 */
} l298n_motor_t;

/**
 * @brief 电机方向控制
 */
typedef enum {
    L298N_DIR_STOP    = 0,  /**< 停止 (IN1=0, IN2=0) */
    L298N_DIR_FORWARD = 1,  /**< 正转 (IN1=1, IN2=0) */
    L298N_DIR_BACKWARD = 2  /**< 反转 (IN1=0, IN2=1) */
} l298n_direction_t;

/**
 * @brief 电机控制模式
 */
typedef enum {
    L298N_MODE_STOPPED = 0,  /**< 停止状态 */
    L298N_MODE_RUNNING = 1   /**< 运行状态 */
} l298n_mode_t;

/* --- 3. 配置结构体 (Configuration Struct) --- */

/**
 * @brief L298N 电机引脚配置
 */
typedef struct {
    void *in1_pin_ctx;       /**< IN1引脚上下文 */
    void *in2_pin_ctx;       /**< IN2引脚上下文 */
    void *in3_pin_ctx;       /**< IN3引脚上下文 */
    void *in4_pin_ctx;       /**< IN4引脚上下文 */
    void *pwm_ctx;           /**< PWM句柄 */
    uint8_t ena_pwm_channel; /**< ENA PWM通道 */
    uint8_t enb_pwm_channel; /**< ENB PWM通道 */
} l298n_pin_config_t;

/* --- 4. 设备句柄 (Device Handle) --- */

/**
 * @brief 单个电机状态
 */
typedef struct {
    l298n_direction_t direction;  /**< 当前方向 */
    l298n_mode_t     mode;        /**< 当前模式 */
    float            duty_cycle;  /**< 当前占空比 (0.0 ~ 1.0) */
} l298n_motor_state_t;

/**
 * @brief L298N 设备对象
 * @note  内存由调用者管理
 */
typedef struct {
    /* 依赖接口 (必须注入) */
    driver_gpio_ops_t gpio_ops;   /**< GPIO操作接口 */
    driver_pwm_ops_t  pwm_ops;    /**< PWM操作接口 */

    /* 运行状态 */
    l298n_pin_config_t pin_cfg;   /**< 引脚配置 */
    l298n_motor_state_t motor1;   /**< 电机1状态 */
    l298n_motor_state_t motor2;   /**< 电机2状态 */
    bool               is_initialized; /**< 初始化标志 */
} l298n_dev_t;

/* --- 5. API 函数声明 (Function Prototypes) --- */

/**
 * @brief 初始化 L298N
 * @param p_dev 设备句柄指针
 * @param p_gpio_ops GPIO接口指针
 * @param p_pwm_ops PWM接口指针
 * @param p_pin_cfg 引脚配置
 * @return DRV_OK 成功, 其他失败
 */
driver_status_t l298n_init(l298n_dev_t *p_dev,
                           const driver_gpio_ops_t *p_gpio_ops,
                           const driver_pwm_ops_t *p_pwm_ops,
                           const l298n_pin_config_t *p_pin_cfg);

/**
 * @brief 停止所有电机 (紧急制动)
 * @param p_dev 设备句柄指针
 * @return driver_status_t
 */
driver_status_t l298n_stop_all(l298n_dev_t *p_dev);

/**
 * @brief 设置单个电机方向
 * @param p_dev 设备句柄指针
 * @param motor 电机选择 (L298N_MOTOR_1 或 L298N_MOTOR_2)
 * @param direction 方向 (L298N_DIR_STOP, L298N_DIR_FORWARD, L298N_DIR_BACKWARD)
 * @return driver_status_t
 */
driver_status_t l298n_set_direction(l298n_dev_t *p_dev, l298n_motor_t motor, l298n_direction_t direction);

/**
 * @brief 设置单个电机速度 (PWM占空比)
 * @param p_dev 设备句柄指针
 * @param motor 电机选择 (L298N_MOTOR_1 或 L298N_MOTOR_2)
 * @param duty 占空比 (0.0 ~ 1.0)
 * @return driver_status_t
 */
driver_status_t l298n_set_speed(l298n_dev_t *p_dev, l298n_motor_t motor, float duty);

/**
 * @brief 控制单个电机 (方向 + 速度)
 * @param p_dev 设备句柄指针
 * @param motor 电机选择 (L298N_MOTOR_1 或 L298N_MOTOR_2)
 * @param direction 方向
 * @param duty 占空比 (0.0 ~ 1.0)
 * @return driver_status_t
 */
driver_status_t l298n_control_motor(l298n_dev_t *p_dev, l298n_motor_t motor, l298n_direction_t direction, float duty);

/**
 * @brief 获取电机状态
 * @param p_dev 设备句柄指针
 * @param motor 电机选择
 * @param p_state 状态输出
 * @return driver_status_t
 */
driver_status_t l298n_get_motor_state(l298n_dev_t *p_dev, l298n_motor_t motor, l298n_motor_state_t *p_state);

/**
 * @brief 小车前进 (两个电机正转)
 * @param p_dev 设备句柄指针
 * @param duty 占空比 (0.0 ~ 1.0)
 * @return driver_status_t
 */
driver_status_t l298n_move_forward(l298n_dev_t *p_dev, float duty);

/**
 * @brief 小车后退 (两个电机反转)
 * @param p_dev 设备句柄指针
 * @param duty 占空比 (0.0 ~ 1.0)
 * @return driver_status_t
 */
driver_status_t l298n_move_backward(l298n_dev_t *p_dev, float duty);

/**
 * @brief 小车左转 (左电机停止/减速，右电机正转)
 * @param p_dev 设备句柄指针
 * @param duty 占空比 (0.0 ~ 1.0)
 * @return driver_status_t
 */
driver_status_t l298n_turn_left(l298n_dev_t *p_dev, float duty);

/**
 * @brief 小车右转 (右电机停止/减速，左电机正转)
 * @param p_dev 设备句柄指针
 * @param duty 占空比 (0.0 ~ 1.0)
 * @return driver_status_t
 */
driver_status_t l298n_turn_right(l298n_dev_t *p_dev, float duty);

#endif /* _L298N_DRIVER_H_ */
