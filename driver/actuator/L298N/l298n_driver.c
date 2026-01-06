/**
 * @file l298n_driver.c
 * @brief L298N 双路直流电机驱动实现
 * @note  遵循"无隐式状态"和"严格分层"原则
 */

#include "l298n_driver.h"
#include <string.h>

/* --- 1. 私有函数 (Private Functions) --- */

/**
 * @brief 设置电机1方向引脚
 */
static void static_set_motor1_direction(l298n_dev_t *p_dev, l298n_direction_t direction) {
    uint8_t in1_state = 0;
    uint8_t in2_state = 0;

    switch (direction) {
        case L298N_DIR_FORWARD:
            in1_state = 1;
            in2_state = 0;
            break;
        case L298N_DIR_BACKWARD:
            in1_state = 0;
            in2_state = 1;
            break;
        case L298N_DIR_STOP:
        default:
            in1_state = 0;
            in2_state = 0;
            break;
    }

    p_dev->gpio_ops.write_pin(p_dev->pin_cfg.in1_pin_ctx, in1_state);
    p_dev->gpio_ops.write_pin(p_dev->pin_cfg.in2_pin_ctx, in2_state);
}

/**
 * @brief 设置电机2方向引脚
 */
static void static_set_motor2_direction(l298n_dev_t *p_dev, l298n_direction_t direction) {
    uint8_t in3_state = 0;
    uint8_t in4_state = 0;

    switch (direction) {
        case L298N_DIR_FORWARD:
            in3_state = 1;
            in4_state = 0;
            break;
        case L298N_DIR_BACKWARD:
            in3_state = 0;
            in4_state = 1;
            break;
        case L298N_DIR_STOP:
        default:
            in3_state = 0;
            in4_state = 0;
            break;
    }

    p_dev->gpio_ops.write_pin(p_dev->pin_cfg.in3_pin_ctx, in3_state);
    p_dev->gpio_ops.write_pin(p_dev->pin_cfg.in4_pin_ctx, in4_state);
}

/**
 * @brief 设置电机1速度
 */
static driver_status_t static_set_motor1_speed(l298n_dev_t *p_dev, float duty) {
    if (duty < L298N_MIN_DUTY) {
        duty = L298N_MIN_DUTY;
    } else if (duty > L298N_MAX_DUTY) {
        duty = L298N_MAX_DUTY;
    }

    return p_dev->pwm_ops.set_duty(p_dev->pin_cfg.pwm_ctx, p_dev->pin_cfg.ena_pwm_channel, duty);
}

/**
 * @brief 设置电机2速度
 */
static driver_status_t static_set_motor2_speed(l298n_dev_t *p_dev, float duty) {
    if (duty < L298N_MIN_DUTY) {
        duty = L298N_MIN_DUTY;
    } else if (duty > L298N_MAX_DUTY) {
        duty = L298N_MAX_DUTY;
    }

    return p_dev->pwm_ops.set_duty(p_dev->pin_cfg.pwm_ctx, p_dev->pin_cfg.enb_pwm_channel, duty);
}

/* --- 2. 公共API实现 (Public API Implementation) --- */

driver_status_t l298n_init(l298n_dev_t *p_dev,
                           const driver_gpio_ops_t *p_gpio_ops,
                           const driver_pwm_ops_t *p_pwm_ops,
                           const l298n_pin_config_t *p_pin_cfg) {
    if (p_dev == NULL || p_gpio_ops == NULL || p_pwm_ops == NULL || p_pin_cfg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_gpio_ops->write_pin == NULL || p_pwm_ops->set_duty == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    memset(p_dev, 0, sizeof(l298n_dev_t));

    p_dev->gpio_ops = *p_gpio_ops;
    p_dev->pwm_ops = *p_pwm_ops;
    p_dev->pin_cfg = *p_pin_cfg;

    p_dev->motor1.direction = L298N_DIR_STOP;
    p_dev->motor1.mode = L298N_MODE_STOPPED;
    p_dev->motor1.duty_cycle = 0.0f;

    p_dev->motor2.direction = L298N_DIR_STOP;
    p_dev->motor2.mode = L298N_MODE_STOPPED;
    p_dev->motor2.duty_cycle = 0.0f;

    l298n_stop_all(p_dev);

    p_dev->is_initialized = true;

    return DRV_OK;
}

driver_status_t l298n_stop_all(l298n_dev_t *p_dev) {
    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    static_set_motor1_direction(p_dev, L298N_DIR_STOP);
    static_set_motor2_direction(p_dev, L298N_DIR_STOP);

    static_set_motor1_speed(p_dev, 0.0f);
    static_set_motor2_speed(p_dev, 0.0f);

    p_dev->motor1.direction = L298N_DIR_STOP;
    p_dev->motor1.mode = L298N_MODE_STOPPED;
    p_dev->motor1.duty_cycle = 0.0f;

    p_dev->motor2.direction = L298N_DIR_STOP;
    p_dev->motor2.mode = L298N_MODE_STOPPED;
    p_dev->motor2.duty_cycle = 0.0f;

    return DRV_OK;
}

driver_status_t l298n_set_direction(l298n_dev_t *p_dev, l298n_motor_t motor, l298n_direction_t direction) {
    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (motor == L298N_MOTOR_1) {
        static_set_motor1_direction(p_dev, direction);
        p_dev->motor1.direction = direction;

        if (direction == L298N_DIR_STOP) {
            p_dev->motor1.mode = L298N_MODE_STOPPED;
        } else {
            p_dev->motor1.mode = L298N_MODE_RUNNING;
        }
    } else if (motor == L298N_MOTOR_2) {
        static_set_motor2_direction(p_dev, direction);
        p_dev->motor2.direction = direction;

        if (direction == L298N_DIR_STOP) {
            p_dev->motor2.mode = L298N_MODE_STOPPED;
        } else {
            p_dev->motor2.mode = L298N_MODE_RUNNING;
        }
    } else {
        return DRV_ERR_INVALID_VAL;
    }

    return DRV_OK;
}

driver_status_t l298n_set_speed(l298n_dev_t *p_dev, l298n_motor_t motor, float duty) {
    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (motor == L298N_MOTOR_1) {
        driver_status_t status = static_set_motor1_speed(p_dev, duty);
        if (status == DRV_OK) {
            p_dev->motor1.duty_cycle = duty;
        }
        return status;
    } else if (motor == L298N_MOTOR_2) {
        driver_status_t status = static_set_motor2_speed(p_dev, duty);
        if (status == DRV_OK) {
            p_dev->motor2.duty_cycle = duty;
        }
        return status;
    } else {
        return DRV_ERR_INVALID_VAL;
    }
}

driver_status_t l298n_control_motor(l298n_dev_t *p_dev, l298n_motor_t motor, l298n_direction_t direction, float duty) {
    driver_status_t status;

    status = l298n_set_direction(p_dev, motor, direction);
    if (status != DRV_OK) {
        return status;
    }

    status = l298n_set_speed(p_dev, motor, duty);
    return status;
}

driver_status_t l298n_get_motor_state(l298n_dev_t *p_dev, l298n_motor_t motor, l298n_motor_state_t *p_state) {
    if (p_dev == NULL || !p_dev->is_initialized || p_state == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    if (motor == L298N_MOTOR_1) {
        *p_state = p_dev->motor1;
    } else if (motor == L298N_MOTOR_2) {
        *p_state = p_dev->motor2;
    } else {
        return DRV_ERR_INVALID_VAL;
    }

    return DRV_OK;
}

driver_status_t l298n_move_forward(l298n_dev_t *p_dev, float duty) {
    driver_status_t status;

    status = l298n_control_motor(p_dev, L298N_MOTOR_1, L298N_DIR_FORWARD, duty);
    if (status != DRV_OK) {
        return status;
    }

    status = l298n_control_motor(p_dev, L298N_MOTOR_2, L298N_DIR_FORWARD, duty);
    return status;
}

driver_status_t l298n_move_backward(l298n_dev_t *p_dev, float duty) {
    driver_status_t status;

    status = l298n_control_motor(p_dev, L298N_MOTOR_1, L298N_DIR_BACKWARD, duty);
    if (status != DRV_OK) {
        return status;
    }

    status = l298n_control_motor(p_dev, L298N_MOTOR_2, L298N_DIR_BACKWARD, duty);
    return status;
}

driver_status_t l298n_turn_left(l298n_dev_t *p_dev, float duty) {
    driver_status_t status;

    status = l298n_control_motor(p_dev, L298N_MOTOR_1, L298N_DIR_STOP, 0.0f);
    if (status != DRV_OK) {
        return status;
    }

    status = l298n_control_motor(p_dev, L298N_MOTOR_2, L298N_DIR_FORWARD, duty);
    return status;
}

driver_status_t l298n_turn_right(l298n_dev_t *p_dev, float duty) {
    driver_status_t status;

    status = l298n_control_motor(p_dev, L298N_MOTOR_1, L298N_DIR_FORWARD, duty);
    if (status != DRV_OK) {
        return status;
    }

    status = l298n_control_motor(p_dev, L298N_MOTOR_2, L298N_DIR_STOP, 0.0f);
    return status;
}
