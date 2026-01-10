# BH1750 驱动模块

## 1. 简介
本模块提供了 ROHM BH1750FVI 数字光照传感器（Ambient Light Sensor）的驱动程序。
驱动设计遵循"无隐式状态"和"严格分层"原则，不依赖具体的硬件平台（如 STM32 HAL 或 Standard Lib），所有硬件操作（I2C读写、延时）均通过接口注入。

## 2. 资源占用
- **ROM**: 约 1KB (取决于编译器优化等级)
- **RAM**: 约 40 Bytes (主要为 `bh1750_dev_t` 结构体，分配在栈或静态区)
- **堆内存**: 0 Bytes (无 `malloc`)

## 3. 依赖项
本驱动依赖以下接口，需在应用层实现并注入：
1. **I2C 读写接口**: `driver_i2c_ops_t`
2. **毫秒延时接口**: `driver_time_ops_t`

## 4. 如何集成

### 4.1 文件添加
将 `driver/sensor/BH1750` 文件夹添加到您的工程中，并包含路径。
- 源文件: `bh1750_driver.c`
- 头文件: `bh1750_driver.h`

### 4.2 接口适配
您需要为您的硬件平台实现 I2C 读写和延时函数。例如基于 STM32 HAL 库：

```c
/* I2C 写适配 */
static driver_status_t my_i2c_write(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len) {
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Write(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t *)p_data, len, 100);
    return (ret == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

/* I2C 读适配 */
static driver_status_t my_i2c_read(void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len) {
    I2C_HandleTypeDef *hi2c = (I2C_HandleTypeDef *)ctx;
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(hi2c, dev_addr << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, p_data, len, 100);
    return (ret == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

/* 延时适配 */
static void my_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}
```

### 4.3 初始化与使用

```c
bh1750_dev_t bh1750_dev;

void app_init() {
    // 1. 定义接口
    driver_i2c_ops_t i2c_ops = { .write_reg = my_i2c_write, .read_reg = my_i2c_read };
    driver_time_ops_t time_ops = { .delay_ms = my_delay_ms };

    // 2. 定义配置
    bh1750_config_t config = {
        .mt_reg = 69  // 默认测量时间寄存器值
    };

    // 3. 初始化
    bh1750_init(&bh1750_dev, &i2c_ops, &time_ops, &hi2c1, BH1750_ADDR_DEFAULT, &config);
}

void app_loop() {
    float lux;

    // 连续测量模式 - 读取光照强度
    if (bh1750_read_lux(&bh1750_dev, &lux) == DRV_OK) {
        printf("Light Intensity: %.2f lux\n", lux);
    }

    HAL_Delay(100);
}

void app_measure_once() {
    float lux;

    // 单次测量模式 - 测量后自动断电
    if (bh1750_measure_lux(&bh1750_dev, &lux) == DRV_OK) {
        printf("Light Intensity: %.2f lux\n", lux);
    }
}
```

## 5. 配置说明
| 参数 | 说明 | 可选值 |
| :--- | :--- | :--- |
| `mt_reg` | 测量时间寄存器 (灵敏度) | 31-254 (默认69) |
| `mode` | 测量模式 | 见下方说明 |

### 测量模式说明
| 模式 | 分辨率 | 典型转换时间 | 说明 |
| :--- | :--- | :--- | :--- |
| `BH1750_MODE_CON_H_RES` | 1 lux | 120ms | 连续高分辨率模式 |
| `BH1750_MODE_CON_H_RES2` | 0.5 lux | 120ms | 连续高分辨率模式2 (默认) |
| `BH1750_MODE_CON_L_RES` | 4 lux | 16ms | 连续低分辨率模式 |
| `BH1750_MODE_ONE_H_RES` | 1 lux | 120ms | 单次高分辨率模式 |
| `BH1750_MODE_ONE_H_RES2` | 0.5 lux | 120ms | 单次高分辨率模式2 |
| `BH1750_MODE_ONE_L_RES` | 4 lux | 16ms | 单次低分辨率模式 |

### 灵敏度调节
- **MTReg值越大，灵敏度越高**
- 范围: 31-254
- 默认值: 69
- 计算公式: `Lux = (Raw * 5 * 69) / (6 * MTReg)`

## 6. 光照强度参考值
| 环境 | 光照强度 |
| :--- | :--- |
| 晴朗夜空 | 0.001 ~ 0.02 lux |
| 满月夜空 | 0.02 ~ 0.3 lux |
| 室内照明 | 5 ~ 50 lux |
| 办公室照明 | 50 ~ 500 lux |
| 室内日光 | 100 ~ 1000 lux |
| 直射阳光 | 约 10,000 ~ 100,000 lux |

## 7. 注意事项
- 确保 I2C 总线已初始化。
- 确保 BH1750 供电正常 (3.3V ~ 5V)。
- 连续模式下，首次测量后需等待约120ms才能读取有效数据。
- 单次测量模式会在测量完成后自动进入 Power Down 模式，适合低功耗应用。
- BH1750 默认 I2C 地址为 0x46 (7位地址)。

## 8. API 函数列表
| 函数 | 说明 |
| :--- | :--- |
| `bh1750_init()` | 初始化设备 |
| `bh1750_power_down()` | 断电 |
| `bh1750_power_on()` | 上电 |
| `bh1750_reset()` | 重置数据寄存器 |
| `bh1750_set_mode()` | 设置测量模式 |
| `bh1750_set_sensitivity()` | 调节测量灵敏度 |
| `bh1750_read_raw()` | 读取原始数据 |
| `bh1750_read_lux()` | 读取光照强度 |
| `bh1750_measure_lux()` | 单次测量光照强度 |
