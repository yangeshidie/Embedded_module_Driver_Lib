/**
 * @file freertos_adapter.h
 * @brief FreeRTOS适配层示例 (FreeRTOS Adapter Example)
 * @note  展示如何将驱动库适配到FreeRTOS环境
 */

#ifndef _FREERTOS_ADAPTER_H_
#define _FREERTOS_ADAPTER_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "driver_interfaces.h"

/* --- 1. 时间与延时接口适配 --- */

static void delay_ms_freertos(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static uint32_t get_tick_freertos(void) {
    return xTaskGetTickCount();
}

/* --- 2. I2C接口适配（带互斥锁） --- */

typedef struct {
    I2C_HandleTypeDef *hi2c;
    SemaphoreHandle_t mutex;
} i2c_context_t;

static driver_status_t i2c_read_freertos(void *ctx, uint8_t dev_addr, 
                                           uint8_t reg_addr, uint8_t *p_data, 
                                           uint16_t len) {
    i2c_context_t *p_i2c_ctx = (i2c_context_t *)ctx;
    
    if (xSemaphoreTake(p_i2c_ctx->mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return DRV_ERR_TIMEOUT;
    }
    
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(p_i2c_ctx->hi2c, dev_addr, 
                                                reg_addr, I2C_MEMADD_SIZE_8BIT, 
                                                p_data, len, 1000);
    
    xSemaphoreGive(p_i2c_ctx->mutex);
    
    return (status == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

static driver_status_t i2c_write_freertos(void *ctx, uint8_t dev_addr, 
                                            uint8_t reg_addr, const uint8_t *p_data, 
                                            uint16_t len) {
    i2c_context_t *p_i2c_ctx = (i2c_context_t *)ctx;
    
    if (xSemaphoreTake(p_i2c_ctx->mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        return DRV_ERR_TIMEOUT;
    }
    
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(p_i2c_ctx->hi2c, dev_addr, 
                                                 reg_addr, I2C_MEMADD_SIZE_8BIT, 
                                                 (uint8_t *)p_data, len, 1000);
    
    xSemaphoreGive(p_i2c_ctx->mutex);
    
    return (status == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

/* --- 3. GPIO接口适配 --- */

static void gpio_write_freertos(void *ctx, uint8_t state) {
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)((uint32_t)ctx & 0xFFFF);
    uint16_t gpio_pin = (uint16_t)((uint32_t)ctx >> 16);
    
    HAL_GPIO_WritePin(gpio_port, gpio_pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

/* --- 4. Timer/PWM接口适配 --- */

static driver_status_t timer_pwm_set_duty_freertos(void *ctx, uint8_t channel, float duty) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    uint32_t pulse = (uint32_t)(duty * (htim->Init.Period + 1));
    
    if (pulse > htim->Init.Period) {
        pulse = htim->Init.Period;
    }
    
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
    return DRV_OK;
}

static driver_status_t timer_pwm_start_freertos(void *ctx, uint8_t channel) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    HAL_TIM_PWM_Start(htim, channel);
    return DRV_OK;
}

static driver_status_t timer_pwm_stop_freertos(void *ctx, uint8_t channel) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    HAL_TIM_PWM_Stop(htim, channel);
    return DRV_OK;
}

/* --- 5. 适配层初始化函数 --- */

typedef struct {
    driver_time_ops_t  time_ops;
    driver_gpio_ops_t  gpio_ops;
    driver_i2c_ops_t   i2c_ops;
    driver_timer_ops_t timer_ops;
    
    i2c_context_t      i2c_ctx;
} freertos_adapter_t;

void freertos_adapter_init(freertos_adapter_t *p_adapter, I2C_HandleTypeDef *hi2c) {
    p_adapter->i2c_ctx.hi2c = hi2c;
    p_adapter->i2c_ctx.mutex = xSemaphoreCreateMutex();
    
    p_adapter->time_ops.delay_ms = delay_ms_freertos;
    p_adapter->time_ops.get_tick = get_tick_freertos;
    
    p_adapter->gpio_ops.write_pin = gpio_write_freertos;
    p_adapter->gpio_ops.read_pin = NULL;
    
    p_adapter->i2c_ops.read = i2c_read_freertos;
    p_adapter->i2c_ops.write = i2c_write_freertos;
    
    p_adapter->timer_ops.start_periodic = NULL;
    p_adapter->timer_ops.start_one_shot = NULL;
    p_adapter->timer_ops.stop = NULL;
    p_adapter->timer_ops.get_counter = NULL;
    p_adapter->timer_ops.set_capture_callback = NULL;
    p_adapter->timer_ops.get_capture_value = NULL;
    p_adapter->timer_ops.pwm_set_duty = timer_pwm_set_duty_freertos;
    p_adapter->timer_ops.pwm_set_freq = NULL;
    p_adapter->timer_ops.pwm_start = timer_pwm_start_freertos;
    p_adapter->timer_ops.pwm_stop = timer_pwm_stop_freertos;
}

#endif /* _FREERTOS_ADAPTER_H_ */
