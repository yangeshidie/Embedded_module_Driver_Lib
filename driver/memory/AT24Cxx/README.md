# AT24Cxx 驱动模块

## 1. 简介
本模块提供了 AT24C 系列 I2C EEPROM 芯片的驱动程序。
AT24C 系列是常用的串行 EEPROM，支持多种容量（128B ~ 32KB），通过 I2C 总线进行读写操作。
驱动设计遵循"无隐式状态"和"严格分层"原则，不依赖具体的硬件平台（如 STM32 HAL 或 Standard Lib），所有硬件操作（I2C）均通过接口注入。

## 2. 支持的设备型号

| 型号 | 容量 | 页大小 | 地址字节数 |
|:---|:---|:---|:---|
| AT24C01 | 128 Bytes | 8 Bytes | 1 |
| AT24C02 | 256 Bytes | 8 Bytes | 1 |
| AT24C04 | 512 Bytes | 16 Bytes | 1 |
| AT24C08 | 1 KB | 16 Bytes | 1 |
| AT24C16 | 2 KB | 16 Bytes | 1 |
| AT24C32 | 4 KB | 32 Bytes | 2 |
| AT24C64 | 8 KB | 32 Bytes | 2 |
| AT24C128 | 16 KB | 64 Bytes | 2 |
| AT24C256 | 32 KB | 64 Bytes | 2 |

## 3. 资源占用
- **ROM**: 约 1.5KB (取决于编译器优化等级)
- **RAM**: 约 80 Bytes (主要为 `at24cxx_dev_t` 结构体，分配在栈或静态区)
- **堆内存**: 动态分配 (仅在 `erase()`、`verify()` 等函数中临时使用)

## 4. 依赖项
本驱动依赖以下接口，需在应用层实现并注入：
1. **I2C 读写接口**: `driver_i2c_ops_t`

## 5. 硬件连接
AT24Cxx EEPROM 与 MCU 的典型 I2C 连接：

| AT24Cxx 引脚 | MCU 引脚类型 | 说明 |
|:---|:---|:---|
| SCL | I2C_SCL | I2C 时钟线 (需上拉电阻) |
| SDA | I2C_SDA | I2C 数据线 (需上拉电阻) |
| A0, A1, A2 | GND 或 VCC | 地址选择引脚 (决定 I2C 设备地址) |
| WP | GND | 写保护引脚 (接地允许写入) |
| VCC | 3.3V 或 5V | 电源 (根据芯片型号) |
| GND | GND | 地 |

**I2C 设备地址计算**:
- 基础地址: `0xA0` (A2=0, A1=0, A0=0)
- 地址范围: `0xA0` ~ `0xAE` (通过 A0/A1/A2 引脚配置)
- 7位地址: `0x50` ~ `0x57`

## 6. 如何集成

### 6.1 文件添加
将 `driver/memory/AT24Cxx` 文件夹添加到您的工程中，并包含路径。
- 源文件: `at24cxx_driver.c`
- 头文件: `at24cxx_driver.h`

### 6.2 接口适配
您需要为您的硬件平台实现 I2C 读写函数。例如基于 STM32 HAL 库：

```c
/* I2C 读取适配 */
static driver_status_t i2c_adapter_read(void *user_data,
                                         uint8_t device_addr,
                                         uint8_t *p_data,
                                         uint16_t length)
{
    I2C_HandleTypeDef *p_hi2c = (I2C_HandleTypeDef *)user_data;
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Master_Transmit(p_hi2c, device_addr, NULL, 0, 100);
    if (hal_status != HAL_OK) {
        return DRV_ERR_IO;
    }

    hal_status = HAL_I2C_Master_Receive(p_hi2c, device_addr, p_data, length, 100);
    if (hal_status != HAL_OK) {
        return DRV_ERR_IO;
    }

    return DRV_OK;
}

/* I2C 写入适配 */
static driver_status_t i2c_adapter_write(void *user_data,
                                          uint8_t device_addr,
                                          const uint8_t *p_data,
                                          uint16_t length)
{
    I2C_HandleTypeDef *p_hi2c = (I2C_HandleTypeDef *)user_data;
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Master_Transmit(p_hi2c, device_addr,
                                          (uint8_t *)p_data, length, 100);
    if (hal_status != HAL_OK) {
        return DRV_ERR_IO;
    }

    return DRV_OK;
}

/* I2C 内存读取适配 (可选) */
static driver_status_t i2c_adapter_mem_read(void *user_data,
                                             uint8_t device_addr,
                                             uint16_t mem_addr,
                                             uint8_t *p_data,
                                             uint16_t length)
{
    I2C_HandleTypeDef *p_hi2c = (I2C_HandleTypeDef *)user_data;
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Mem_Read(p_hi2c, device_addr, mem_addr,
                                  I2C_MEMADD_SIZE_16BIT, p_data, length, 100);
    if (hal_status != HAL_OK) {
        return DRV_ERR_IO;
    }

    return DRV_OK;
}

/* I2C 内存写入适配 (可选) */
static driver_status_t i2c_adapter_mem_write(void *user_data,
                                              uint8_t device_addr,
                                              uint16_t mem_addr,
                                              const uint8_t *p_data,
                                              uint16_t length)
{
    I2C_HandleTypeDef *p_hi2c = (I2C_HandleTypeDef *)user_data;
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Mem_Write(p_hi2c, device_addr, mem_addr,
                                    I2C_MEMADD_SIZE_16BIT, (uint8_t *)p_data, length, 100);
    if (hal_status != HAL_OK) {
        return DRV_ERR_IO;
    }

    return DRV_OK;
}
```

### 6.3 初始化与使用

```c
at24cxx_dev_t eeprom_dev;

void app_init() {
    driver_i2c_ops_t i2c_ops = {
        .read = i2c_adapter_read,
        .write = i2c_adapter_write,
        .mem_read = i2c_adapter_mem_read,
        .mem_write = i2c_adapter_mem_write
    };

    at24cxx_config_t config = AT24CXX_GET_DEFAULT_CONFIG(AT24CXX_MODEL_128);

    at24cxx_init(&eeprom_dev, &i2c_ops, &config, &hi2c1);
}

void app_loop() {
    uint8_t write_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05,
                               0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t read_data[10];

    at24cxx_write(&eeprom_dev, 0x0000, write_data, 10);
    HAL_Delay(10);

    at24cxx_read(&eeprom_dev, 0x0000, read_data, 10);
}
```

## 7. API 说明

### 7.1 基础函数

| 函数 | 说明 |
|:---|:---|
| `at24cxx_init()` | 初始化 EEPROM 设备 |
| `at24cxx_probe()` | 检测设备是否存在 |
| `at24cxx_read()` | 读取数据 |
| `at24cxx_write()` | 写入数据 (自动页写入优化) |
| `at24cxx_erase()` | 擦除数据 (填充 0xFF) |
| `at24cxx_read_byte()` | 读取单个字节 |
| `at24cxx_write_byte()` | 写入单个字节 |
| `at24cxx_verify()` | 验证数据一致性 |

### 7.2 信息查询函数

| 函数 | 说明 |
|:---|:---|
| `at24cxx_get_config()` | 获取设备配置信息 |
| `at24cxx_get_capacity()` | 获取设备容量 |
| `at24cxx_get_page_size()` | 获取页大小 |

## 8. 页写入优化

EEPROM 的写入操作有页大小限制：
- **AT24C02**: 8 bytes/page
- **AT24C04/08/16**: 16 bytes/page
- **AT24C32/64**: 32 bytes/page
- **AT24C128/256**: 64 bytes/page

本驱动自动处理跨页写入，无需手动管理页边界。例如：
```c
uint8_t data[100];
at24cxx_write(&dev, 0x00C0, data, 100);  /* 自动处理跨页写入 */
```

## 9. 写入时序

EEPROM 写入操作需要等待内部写周期完成：
- **写周期时间**: 通常 5ms ~ 10ms
- **默认超时**: 10ms (可配置)
- **自动等待**: 驱动自动轮询等待写入完成

**注意**: 连续写入时，驱动会自动等待每次写入完成，无需手动延时。

## 10. 典型应用场景

### 10.1 参数存储
```c
typedef struct {
    uint32_t magic;
    float    kp;
    float    ki;
    float    kd;
} pid_params_t;

pid_params_t params;

at24cxx_write(&dev, 0x0100, (uint8_t *)&params, sizeof(pid_params_t));
```

### 10.2 数据记录
```c
uint8_t log_data[64];
uint16_t log_addr = 0x0200;

at24cxx_write(&dev, log_addr, log_data, sizeof(log_data));
log_addr += sizeof(log_data);
```

### 10.3 校准数据存储
```c
float calib_data[10];
at24cxx_write(&dev, 0x0300, (uint8_t *)calib_data, sizeof(calib_data));
```

## 11. 注意事项
- I2C 总线需要上拉电阻（通常 4.7kΩ ~ 10kΩ）
- 写入操作前确保 WP 引脚接地（允许写入）
- 避免频繁写入同一地址（EEPROM 有擦写寿命限制，通常 10万次）
- 写入大块数据时注意页边界，驱动已自动处理
- 读取操作无限制，可随时进行
- 多个 AT24Cxx 可挂载在同一 I2C 总线，通过 A0/A1/A2 引脚区分地址
- 设备地址必须与硬件连接的 A0/A1/A2 引脚状态一致

## 12. 性能参数

| 参数 | 典型值 |
|:---|:---|
| I2C 时钟频率 | 100kHz (标准) / 400kHz (快速) |
| 写入周期时间 | 5ms ~ 10ms |
| 读取时间 | 取决于 I2C 速度 |
| 擦写寿命 | 100,000 次 |
| 数据保持时间 | 100 年 |
