/**
 * @file L298N_example.c
 * @brief L298N 驱动使用示例 (基于 STM32 HAL 库)
 * @note  本示例展示如何适配 L298N 驱动到具体硬件平台
 */

#include "l298n_driver.h"
#include "stm32f4xx_hal.h"

/* --- 1. 硬件定义 (Hardware Definitions) --- */

#define PWM_FREQUENCY_HZ    20000

/* GPIO 引脚定义 (根据实际硬件连接修改) */
#define IN1_GPIO_PORT        GPIOA
#define IN1_GPIO_PIN        GPIO_PIN_0
#define IN2_GPIO_PORT        GPIOA
#define IN2_GPIO_PIN        GPIO_PIN_1
#define IN3_GPIO_PORT        GPIOA
#define IN3_GPIO_PIN        GPIO_PIN_2
#define IN4_GPIO_PORT        GPIOA
#define IN4_GPIO_PIN        GPIO_PIN_3

/* PWM 定时器定义 (根据实际硬件连接修改) */
#define PWM_TIMER            TIM3
#define PWM_CHANNEL_ENA      TIM_CHANNEL_1
#define PWM_CHANNEL_ENB      TIM_CHANNEL_2

/* --- 2. 硬件句柄 (Hardware Handles) --- */

extern TIM_HandleTypeDef htim3;

/* --- 3. 接口适配函数 (Interface Adaptation Functions) --- */

/**
 * @brief GPIO 写入适配
 */
static void example_gpio_write(void *ctx, uint8_t state) {
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)ctx;
    uint16_t gpio_pin = (uint16_t)((uint32_t)ctx >> 16);

    if (state) {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief PWM 占空比设置适配
 */
static driver_status_t example_pwm_set_duty(void *ctx, uint8_t channel, float duty) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    uint32_t pulse = (uint32_t)(duty * (htim->Init.Period + 1));

    if (pulse > htim->Init.Period) {
        pulse = htim->Init.Period;
    }

    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
    return DRV_OK;
}

/**
 * @brief PWM 启动适配
 */
static driver_status_t example_pwm_start(void *ctx, uint8_t channel) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    HAL_TIM_PWM_Start(htim, channel);
    return DRV_OK;
}

/**
 * @brief PWM 停止适配
 */
static driver_status_t example_pwm_stop(void *ctx, uint8_t channel) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    HAL_TIM_PWM_Stop(htim, channel);
    return DRV_OK;
}

/**
 * @brief PWM 频率设置适配 (可选)
 */
static driver_status_t example_pwm_set_freq(void *ctx, uint32_t frequency_hz) {
    (void)ctx;
    (void)frequency_hz;
    return DRV_OK;
}

/**
 * @brief 定时器周期启动适配 (可选，L298N不需要)
 */
static driver_status_t example_timer_start_periodic(void *ctx, uint32_t period_us, timer_callback_t cb, void *user_data) {
    (void)ctx;
    (void)period_us;
    (void)cb;
    (void)user_data;
    return DRV_OK;
}

/**
 * @brief 定时器单次启动适配 (可选，L298N不需要)
 */
static driver_status_t example_timer_start_one_shot(void *ctx, uint32_t timeout_us, timer_callback_t cb, void *user_data) {
    (void)ctx;
    (void)timeout_us;
    (void)cb;
    (void)user_data;
    return DRV_OK;
}

/**
 * @brief 定时器停止适配 (可选，L298N不需要)
 */
static driver_status_t example_timer_stop(void *ctx) {
    (void)ctx;
    return DRV_OK;
}

/**
 * @brief 获取定时器计数值适配 (可选，L298N不需要)
 */
static uint32_t example_timer_get_counter(void *ctx) {
    (void)ctx;
    return 0;
}

/**
 * @brief 设置输入捕获回调适配 (可选，L298N不需要)
 */
static driver_status_t example_timer_set_capture_callback(void *ctx, uint8_t channel, timer_callback_t cb, void *user_data) {
    (void)ctx;
    (void)channel;
    (void)cb;
    (void)user_data;
    return DRV_OK;
}

/**
 * @brief 获取捕获值适配 (可选，L298N不需要)
 */
static driver_status_t example_timer_get_capture_value(void *ctx, uint8_t channel, uint32_t *p_value) {
    (void)ctx;
    (void)channel;
    (void)p_value;
    return DRV_OK;
}

/* --- 4. 应用程序 (Application) --- */

l298n_dev_t g_l298n_dev;

void l298n_example_init(void) {
    driver_status_t status;

    driver_gpio_ops_t gpio_ops = {
        .write_pin = example_gpio_write,
        .read_pin = NULL
    };

    driver_timer_ops_t timer_ops = {
        .start_periodic = example_timer_start_periodic,
        .start_one_shot = example_timer_start_one_shot,
        .stop = example_timer_stop,
        .get_counter = example_timer_get_counter,
        .set_capture_callback = example_timer_set_capture_callback,
        .get_capture_value = example_timer_get_capture_value,
        .pwm_set_duty = example_pwm_set_duty,
        .pwm_set_freq = example_pwm_set_freq,
        .pwm_start = example_pwm_start,
        .pwm_stop = example_pwm_stop
    };

    l298n_pin_config_t pin_cfg = {
        .in1_pin_ctx = (void *)((uint32_t)IN1_GPIO_PORT | ((uint32_t)IN1_GPIO_PIN << 16)),
        .in2_pin_ctx = (void *)((uint32_t)IN2_GPIO_PORT | ((uint32_t)IN2_GPIO_PIN << 16)),
        .in3_pin_ctx = (void *)((uint32_t)IN3_GPIO_PORT | ((uint32_t)IN3_GPIO_PIN << 16)),
        .in4_pin_ctx = (void *)((uint32_t)IN4_GPIO_PORT | ((uint32_t)IN4_GPIO_PIN << 16)),
        .pwm_ctx = &htim3,
        .ena_pwm_channel = PWM_CHANNEL_ENA,
        .enb_pwm_channel = PWM_CHANNEL_ENB
    };

    status = l298n_init(&g_l298n_dev, &gpio_ops, &timer_ops, &pin_cfg);
    if (status != DRV_OK) {
        while (1);
    }
}

void l298n_example_basic_control(void) {
    l298n_example_init();

    while (1) {
        l298n_move_forward(&g_l298n_dev, 0.5f);
        HAL_Delay(2000);

        l298n_stop_all(&g_l298n_dev);
        HAL_Delay(500);

        l298n_move_backward(&g_l298n_dev, 0.5f);
        HAL_Delay(2000);

        l298n_stop_all(&g_l298n_dev);
        HAL_Delay(500);

        l298n_turn_left(&g_l298n_dev, 0.5f);
        HAL_Delay(1000);

        l298n_stop_all(&g_l298n_dev);
        HAL_Delay(500);

        l298n_turn_right(&g_l298n_dev, 0.5f);
        HAL_Delay(1000);

        l298n_stop_all(&g_l298n_dev);
        HAL_Delay(500);
    }
}

void l298n_example_speed_control(void) {
    l298n_example_init();
    float speed = 0.0f;

    while (1) {
        for (speed = 0.0f; speed <= 1.0f; speed += 0.1f) {
            l298n_move_forward(&g_l298n_dev, speed);
            HAL_Delay(200);
        }

        l298n_stop_all(&g_l298n_dev);
        HAL_Delay(1000);

        for (speed = 1.0f; speed >= 0.0f; speed -= 0.1f) {
            l298n_move_backward(&g_l298n_dev, speed);
            HAL_Delay(200);
        }

        l298n_stop_all(&g_l298n_dev);
        HAL_Delay(1000);
    }
}

void l298n_example_individual_motor_control(void) {
    l298n_example_init();

    while (1) {
        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_1, L298N_DIR_FORWARD, 0.5f);
        HAL_Delay(2000);

        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_1, L298N_DIR_STOP, 0.0f);
        HAL_Delay(500);

        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_2, L298N_DIR_FORWARD, 0.5f);
        HAL_Delay(2000);

        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_2, L298N_DIR_STOP, 0.0f);
        HAL_Delay(500);

        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_1, L298N_DIR_BACKWARD, 0.5f);
        HAL_Delay(2000);

        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_1, L298N_DIR_STOP, 0.0f);
        HAL_Delay(500);

        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_2, L298N_DIR_BACKWARD, 0.5f);
        HAL_Delay(2000);

        l298n_control_motor(&g_l298n_dev, L298N_MOTOR_2, L298N_DIR_STOP, 0.0f);
        HAL_Delay(500);
    }
}

void l298n_example_get_state(void) {
    l298n_example_init();
    l298n_motor_state_t motor_state;

    l298n_move_forward(&g_l298n_dev, 0.7f);

    while (1) {
        l298n_get_motor_state(&g_l298n_dev, L298N_MOTOR_1, &motor_state);

        HAL_Delay(1000);
    }
}
