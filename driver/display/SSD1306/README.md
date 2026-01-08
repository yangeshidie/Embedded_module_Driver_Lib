# SSD1306 驱动模块

## 1. 简介
本模块提供了基于 SSD1306 控制器的 OLED 显示屏驱动程序（支持 128x64 分辨率）。
驱动设计遵循"无隐式状态"和"严格分层"原则，不依赖具体的硬件平台（如 STM32 HAL 或 Standard Lib），所有硬件操作（I2C读写、延时）均通过接口注入。

## 2. 资源占用
- **ROM**: 约 3KB (取决于编译器优化等级)
- **RAM**: 约 1KB (主要为 `ssd1306_dev_t` 结构体中的显示缓冲区，分配在栈或静态区)
- **堆内存**: 0 Bytes (无 `malloc`)

## 3. 依赖项
本驱动依赖以下接口，需在应用层实现并注入：
1. **I2C 写接口**: `driver_i2c_ops_t` (仅需要 write_reg)
2. **毫秒延时接口**: `driver_time_ops_t` (仅需要 delay_ms)

## 4. 如何集成

### 4.1 文件添加
将 `driver/display/SSD1306` 文件夹添加到您的工程中，并包含路径。
- 源文件: `ssd1306_driver.c`, `ssd1306_font.c`
- 头文件: `ssd1306_driver.h`, `ssd1306_font.h`

### 4.2 接口适配
您需要为您的硬件平台实现 I2C 写和延时函数。例如基于 STM32 HAL 库：

```c
/* I2C 写适配 */
static driver_status_t my_i2c_write(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len) {
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Write((I2C_HandleTypeDef*)ctx, dev_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)p_data, len, 100);
    return (status == HAL_OK) ? DRV_OK : DRV_ERR_IO;
}

/* 延时适配 */
static void my_delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}
```

### 4.3 初始化与使用

```c
ssd1306_dev_t oled_dev;

void app_init() {
    // 1. 定义接口
    driver_i2c_ops_t i2c_ops = { .write_reg = my_i2c_write, .read_reg = NULL };
    driver_time_ops_t time_ops = { .delay_ms = my_delay_ms };

    // 2. 初始化
    ssd1306_init(&oled_dev, &i2c_ops, &time_ops, &hi2c1, SSD1306_ADDR_LOW);
}

void app_loop() {
    // 清屏
    ssd1306_clear(&oled_dev);
    
    // 绘制字符串
    ssd1306_draw_string(&oled_dev, 0, 0, "Hello World!", SSD1306_FONT_SIZE_8x16, SSD1306_COLOR_WHITE);
    
    // 绘制数字
    ssd1306_draw_uint(&oled_dev, 0, 20, 12345, SSD1306_FONT_SIZE_8x16, SSD1306_COLOR_WHITE);
    
    // 更新显示
    ssd1306_update_display(&oled_dev);
}
```

## 5. API 说明

### 5.1 初始化与控制
| 函数 | 说明 |
| :--- | :--- |
| `ssd1306_init()` | 初始化 OLED 显示屏 |
| `ssd1306_deinit()` | 反初始化 OLED 显示屏 |
| `ssd1306_display_on()` | 打开显示 |
| `ssd1306_display_off()` | 关闭显示 |

### 5.2 绘图函数
| 函数 | 说明 |
| :--- | :--- |
| `ssd1306_clear()` | 清空显示缓冲区 |
| `ssd1306_update_display()` | 将缓冲区内容刷新到屏幕 |
| `ssd1306_set_pixel()` | 设置单个像素点 |
| `ssd1306_draw_line()` | 绘制直线 |
| `ssd1306_draw_rect()` | 绘制矩形边框 |
| `ssd1306_fill_rect()` | 填充矩形 |

### 5.3 文本函数
| 函数 | 说明 |
| :--- | :--- |
| `ssd1306_draw_char()` | 绘制单个字符 |
| `ssd1306_draw_string()` | 绘制字符串（支持换行符） |
| `ssd1306_draw_uint()` | 绘制无符号整数 |
| `ssd1306_draw_int()` | 绘制有符号整数 |

## 6. 配置说明
| 参数 | 说明 | 可选值 |
| :--- | :--- | :--- |
| `dev_addr` | I2C 设备地址 | 0x78 (SA0=0) 或 0x7A (SA0=1) |
| `font_size` | 字体大小 | SSD1306_FONT_SIZE_6x8 或 SSD1306_FONT_SIZE_8x16 |
| `color` | 绘制颜色 | SSD1306_COLOR_BLACK 或 SSD1306_COLOR_WHITE |

## 7. 注意事项
- 确保 I2C 总线已初始化。
- 确保 OLED 显示屏供电正常。
- SA0 引脚电平决定设备地址 (Low: 0x78, High: 0x7A)。
- 所有绘图操作仅更新显示缓冲区，需要调用 `ssd1306_update_display()` 才能刷新到屏幕。
- 显示缓冲区大小为 128x64 像素，占用 1KB RAM。
- 支持自动换行功能，当字符串超出屏幕宽度时会自动换行。
- 支持负数显示，`ssd1306_draw_int()` 会自动添加负号。

## 8. 示例代码
完整的示例代码请参考 `SSD1306_example.c` 文件，包含：
- 基础初始化和显示测试
- 字符串和数字显示
- 几何图形绘制（直线、矩形、填充矩形）
- 显示开关控制
