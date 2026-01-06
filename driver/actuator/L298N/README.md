# L298N 驱动模块

## 1. 简介
本模块提供了 L298N 双路直流电机驱动芯片的驱动程序。
L298N 是一款常用的双路全桥电机驱动芯片，可驱动两个直流电机或一个步进电机，支持 PWM 速度控制。
驱动设计遵循"无隐式状态"和"严格分层"原则，不依赖具体的硬件平台（如 STM32 HAL 或 Standard Lib），所有硬件操作（GPIO、PWM）均通过接口注入。

## 2. 资源占用
- **ROM**: 约 1.2KB (取决于编译器优化等级)
- **RAM**: 约 80 Bytes (主要为 `l298n_dev_t` 结构体，分配在栈或静态区)
- **堆内存**: 0 Bytes (无 `malloc`)

## 3. 依赖项
本驱动依赖以下接口，需在应用层实现并注入：
1. **GPIO 写入接口**: `driver_gpio_ops_t.write_pin`
2. **PWM 操作接口**: `driver_pwm_ops_t`

## 4. 硬件连接
L298N 模块与 MCU 的典型连接：

| L298N 引脚 | MCU 引脚类型 | 说明 |
|:---|:---|:---|
| IN1 | GPIO 输出 | 电机1方向控制1 |
| IN2 | GPIO 输出 | 电机1方向控制2 |
| IN3 | GPIO 输出 | 电机2方向控制1 |
| IN4 | GPIO 输出 | 电机2方向控制2 |
| ENA | PWM 输出 | 电机1使能/速度控制 |
| ENB | PWM 输出 | 电机2使能/速度控制 |
| 12V | 外部电源 | 电机供电 (9V-35V) |
| 5V | 5V 输出 | 可为逻辑电路供电 |
| GND | 地 | 必须与 MCU 共地 |

## 5. 如何集成

### 5.1 文件添加
将 `driver/actuator/L298N` 文件夹添加到您的工程中，并包含路径。
- 源文件: `l298n_driver.c`
- 头文件: `l298n_driver.h`

### 5.2 接口适配
您需要为您的硬件平台实现 GPIO 写入和 PWM 操作函数。例如基于 STM32 HAL 库：

```c
/* GPIO 写入适配 */
static void my_gpio_write(void *ctx, uint8_t state) {
    GPIO_TypeDef *gpio_port = (GPIO_TypeDef *)ctx;
    uint16_t gpio_pin = (uint16_t)((uint32_t)ctx >> 16);

    if (state) {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(gpio_port, gpio_pin, GPIO_PIN_RESET);
    }
}

/* PWM 占空比设置适配 */
static driver_status_t my_pwm_set_duty(void *ctx, uint8_t channel, float duty) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    uint32_t pulse = (uint32_t)(duty * (htim->Init.Period + 1));

    if (pulse > htim->Init.Period) {
        pulse = htim->Init.Period;
    }

    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
    return DRV_OK;
}

/* PWM 启动/停止适配 */
static driver_status_t my_pwm_start(void *ctx, uint8_t channel) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    HAL_TIM_PWM_Start(htim, channel);
    return DRV_OK;
}

static driver_status_t my_pwm_stop(void *ctx, uint8_t channel) {
    TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)ctx;
    HAL_TIM_PWM_Stop(htim, channel);
    return DRV_OK;
}
```

### 5.3 初始化与使用

```c
l298n_dev_t l298n_dev;

void app_init() {
    driver_gpio_ops_t gpio_ops = {
        .write_pin = my_gpio_write,
        .read_pin = NULL
    };

    driver_pwm_ops_t pwm_ops = {
        .set_duty = my_pwm_set_duty,
        .set_freq = NULL,
        .start = my_pwm_start,
        .stop = my_pwm_stop
    };

    l298n_pin_config_t pin_cfg = {
        .in1_pin_ctx = (void *)((uint32_t)GPIOA | ((uint32_t)GPIO_PIN_0 << 16)),
        .in2_pin_ctx = (void *)((uint32_t)GPIOA | ((uint32_t)GPIO_PIN_1 << 16)),
        .in3_pin_ctx = (void *)((uint32_t)GPIOA | ((uint32_t)GPIO_PIN_2 << 16)),
        .in4_pin_ctx = (void *)((uint32_t)GPIOA | ((uint32_t)GPIO_PIN_3 << 16)),
        .pwm_ctx = &htim3,
        .ena_pwm_channel = TIM_CHANNEL_1,
        .enb_pwm_channel = TIM_CHANNEL_2
    };

    l298n_init(&l298n_dev, &gpio_ops, &pwm_ops, &pin_cfg);
}

void app_loop() {
    l298n_move_forward(&l298n_dev, 0.5f);
    HAL_Delay(2000);

    l298n_turn_left(&l298n_dev, 0.5f);
    HAL_Delay(1000);

    l298n_move_backward(&l298n_dev, 0.5f);
    HAL_Delay(2000);

    l298n_stop_all(&l298n_dev);
}
```

## 6. API 说明

### 6.1 基础控制函数

| 函数 | 说明 |
|:---|:---|
| `l298n_init()` | 初始化 L298N 驱动 |
| `l298n_stop_all()` | 停止所有电机 (紧急制动) |
| `l298n_set_direction()` | 设置单个电机方向 |
| `l298n_set_speed()` | 设置单个电机速度 (PWM占空比) |
| `l298n_control_motor()` | 同时控制单个电机的方向和速度 |
| `l298n_get_motor_state()` | 获取电机当前状态 |

### 6.2 小车运动函数

| 函数 | 说明 |
|:---|:---|
| `l298n_move_forward()` | 小车前进 (两个电机正转) |
| `l298n_move_backward()` | 小车后退 (两个电机反转) |
| `l298n_turn_left()` | 小车左转 (左电机停止，右电机正转) |
| `l298n_turn_right()` | 小车右转 (右电机停止，左电机正转) |

## 7. 电机方向控制

L298N 通过 IN1/IN2 (电机1) 和 IN3/IN4 (电机2) 控制电机方向：

| IN1 | IN2 | 电机1状态 | 说明 |
|:---:|:---:|:---|:---|
| 0 | 0 | 停止 | 制动状态 |
| 1 | 0 | 正转 | 顺时针旋转 |
| 0 | 1 | 反转 | 逆时针旋转 |
| 1 | 1 | 制动 | 快速制动 |

电机2 的控制逻辑相同，对应 IN3/IN4。

## 8. PWM 频率建议

| 应用场景 | 推荐频率 | 说明 |
|:---|:---|:---|
| 直流电机 | 1kHz ~ 20kHz | 常用 10kHz ~ 20kHz，兼顾噪音和效率 |
| 步进电机 | 10kHz ~ 100kHz | 根据步进电机特性调整 |

**注意**: 频率过高可能导致 MOSFET 开关损耗增加，频率过低可能导致电机噪音和抖动。

## 9. 注意事项
- 确保 L298N 供电电压在 9V-35V 范围内。
- 必须将 L298N 的 GND 与 MCU 的 GND 共地。
- PWM 占空比范围为 0.0 ~ 1.0，超出范围会被自动限制。
- 长时间大电流工作时，注意 L298N 的散热问题。
- 驱动感性负载（电机）时，建议在电机两端并联续流二极管（L298N 内部已集成）。
- 如果电机转动方向与预期相反，只需交换电机的两根线即可。

## 10. 典型应用场景
- 两轮差速小车
- 四轮驱动小车 (使用两个 L298N)
- 传送带控制
- 窗帘/门禁系统
- 机器人关节控制
