# MPU6050 驱动模块

## 1. 简介
本模块提供了 InvenSense MPU6050 6轴运动处理组件（3轴陀螺仪 + 3轴加速度计）的驱动程序。
驱动设计遵循“无隐式状态”和“严格分层”原则，不依赖具体的硬件平台（如 STM32 HAL 或 Standard Lib），所有硬件操作（I2C读写、延时）均通过接口注入。

## 2. 资源占用
- **ROM**: 约 1.5KB (取决于编译器优化等级)
- **RAM**: 约 60 Bytes (主要为 `mpu6050_dev_t` 结构体，分配在栈或静态区)
- **堆内存**: 0 Bytes (无 `malloc`)

## 3. 依赖项
本驱动依赖以下接口，需在应用层实现并注入：
1. **I2C 读写接口**: `driver_i2c_ops_t`
2. **毫秒延时接口**: `driver_time_ops_t`

## 4. 如何集成

### 4.1 文件添加
将 `driver/MPU6050` 文件夹添加到您的工程中，并包含路径。
- 源文件: `mpu6050_driver.c`
- 头文件: `mpu6050_driver.h`

### 4.2 接口适配
您需要为您的硬件平台实现 I2C 读写和延时函数。例如基于 STM32 HAL 库：

```c
/* I2C 写适配 */
static driver_status_t my_i2c_write(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len) {
    // 调用 HAL_I2C_Mem_Write ...
}

/* I2C 读适配 */
static driver_status_t my_i2c_read(void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len) {
    // 调用 HAL_I2C_Mem_Read ...
}

/* 延时适配 */
static void my_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}
```

### 4.3 初始化与使用

```c
mpu6050_dev_t mpu_dev;

void app_init() {
    // 1. 定义接口
    driver_i2c_ops_t i2c_ops = { .write_reg = my_i2c_write, .read_reg = my_i2c_read };
    driver_time_ops_t time_ops = { .delay_ms = my_delay_ms };

    // 2. 定义配置
    mpu6050_config_t config = {
        .gyro_fs = MPU6050_GYRO_FS_2000,
        .accel_fs = MPU6050_ACCEL_FS_8,
        .dlpf_cfg = MPU6050_DLPF_BW_44,
        .sample_rate_div = 9
    };

    // 3. 初始化
    mpu6050_init(&mpu_dev, &i2c_ops, &time_ops, &hi2c1, MPU6050_ADDR_AD0_LOW, &config);
}

void app_loop() {
    mpu6050_float_data_t accel, gyro;
    float temp;
    
    // 读取数据
    mpu6050_read_all(&mpu_dev, &accel, &gyro, &temp);
}
```

## 5. 配置说明
| 参数 | 说明 | 可选值 |
| :--- | :--- | :--- |
| `gyro_fs` | 陀螺仪量程 | ±250, ±500, ±1000, ±2000 dps |
| `accel_fs` | 加速度计量程 | ±2, ±4, ±8, ±16 g |
| `dlpf_cfg` | 低通滤波器带宽 | 260Hz ~ 5Hz |
| `sample_rate_div` | 采样率分频 | Sample Rate = 1kHz / (1 + div) |

## 6. 注意事项
- 确保 I2C 总线已初始化。
- 确保 MPU6050 供电正常。
- AD0 引脚电平决定设备地址 (Low: 0x68, High: 0x69)。
