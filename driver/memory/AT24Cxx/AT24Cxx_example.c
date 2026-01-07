#include "at24cxx_driver.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- 1. 硬件相关头文件 (Hardware Headers) --- */
#include "stm32f4xx_hal.h"

/* --- 2. 全局变量 (Global Variables) --- */
extern I2C_HandleTypeDef hi2c1;

static at24cxx_dev_t g_at24c128_dev;

/* --- 3. I2C 接口适配 (I2C Interface Adaptation) --- */

static driver_status_t i2c_adapter_write_reg(void *user_data,
                                               uint8_t dev_addr,
                                               uint8_t reg_addr,
                                               const uint8_t *p_data,
                                               uint32_t length)
{
    I2C_HandleTypeDef *p_hi2c = (I2C_HandleTypeDef *)user_data;
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Mem_Write(p_hi2c, dev_addr, reg_addr,
                                    I2C_MEMADD_SIZE_8BIT, (uint8_t *)p_data, length, 100);
    if (hal_status != HAL_OK) {
        return DRV_ERR_IO;
    }

    return DRV_OK;
}

static driver_status_t i2c_adapter_read_reg(void *user_data,
                                              uint8_t dev_addr,
                                              uint8_t reg_addr,
                                              uint8_t *p_data,
                                              uint32_t length)
{
    I2C_HandleTypeDef *p_hi2c = (I2C_HandleTypeDef *)user_data;
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_I2C_Mem_Read(p_hi2c, dev_addr, reg_addr,
                                   I2C_MEMADD_SIZE_8BIT, p_data, length, 100);
    if (hal_status != HAL_OK) {
        return DRV_ERR_IO;
    }

    return DRV_OK;
}

/* --- 4. 初始化函数 (Initialization) --- */

void at24cxx_example_init(void)
{
    driver_i2c_ops_t i2c_ops = {
        .write_reg = i2c_adapter_write_reg,
        .read_reg = i2c_adapter_read_reg
    };

    at24cxx_config_t config = AT24CXX_GET_DEFAULT_CONFIG(AT24CXX_MODEL_128);

    config.device_addr = 0x50;

    at24cxx_init(&g_at24c128_dev, &i2c_ops, &config, &hi2c1);
}

/* --- 5. 示例1: 基础读写 (Example 1: Basic Read/Write) --- */

void at24cxx_example_basic_rw(void)
{
    uint8_t write_data[10] = {0x01, 0x02, 0x03, 0x04, 0x05,
                               0x06, 0x07, 0x08, 0x09, 0x0A};
    uint8_t read_data[10];
    driver_status_t status;
    uint16_t i;

    printf("=== Example 1: Basic Read/Write ===\n");

    status = at24cxx_write(&g_at24c128_dev, 0x0000, write_data, 10);
    if (status == DRV_OK) {
        printf("Write OK\n");
    } else {
        printf("Write Failed: %d\n", status);
        return;
    }

    HAL_Delay(10);

    status = at24cxx_read(&g_at24c128_dev, 0x0000, read_data, 10);
    if (status == DRV_OK) {
        printf("Read OK\n");
        printf("Data: ");
        for (i = 0; i < 10; i++) {
            printf("%02X ", read_data[i]);
        }
        printf("\n");
    } else {
        printf("Read Failed: %d\n", status);
    }
}

/* --- 6. 示例2: 字符串存储 (Example 2: String Storage) --- */

void at24cxx_example_string_storage(void)
{
    const char *p_string = "Hello, AT24Cxx!";
    uint8_t read_buf[32];
    driver_status_t status;
    uint16_t str_len;

    printf("\n=== Example 2: String Storage ===\n");

    str_len = strlen(p_string) + 1;

    status = at24cxx_write(&g_at24c128_dev, 0x0100,
                           (const uint8_t *)p_string, str_len);
    if (status == DRV_OK) {
        printf("Write String OK: %s\n", p_string);
    } else {
        printf("Write String Failed: %d\n", status);
        return;
    }

    HAL_Delay(10);

    status = at24cxx_read(&g_at24c128_dev, 0x0100, read_buf, str_len);
    if (status == DRV_OK) {
        printf("Read String OK: %s\n", read_buf);
    } else {
        printf("Read String Failed: %d\n", status);
    }
}

/* --- 7. 示例3: 跨页写入 (Example 3: Cross-Page Write) --- */

void at24cxx_example_cross_page_write(void)
{
    uint8_t write_data[100];
    uint8_t read_data[100];
    driver_status_t status;
    uint16_t i;

    printf("\n=== Example 3: Cross-Page Write ===\n");

    for (i = 0; i < 100; i++) {
        write_data[i] = (uint8_t)i;
    }

    status = at24cxx_write(&g_at24c128_dev, 0x00C0, write_data, 100);
    if (status == DRV_OK) {
        printf("Cross-Page Write OK (100 bytes)\n");
    } else {
        printf("Cross-Page Write Failed: %d\n", status);
        return;
    }

    HAL_Delay(10);

    status = at24cxx_read(&g_at24c128_dev, 0x00C0, read_data, 100);
    if (status == DRV_OK) {
        printf("Cross-Page Read OK\n");

        for (i = 0; i < 100; i++) {
            if (read_data[i] != write_data[i]) {
                printf("Data Mismatch at index %d: write=%02X, read=%02X\n",
                       i, write_data[i], read_data[i]);
                return;
            }
        }
        printf("Data Verification OK\n");
    } else {
        printf("Cross-Page Read Failed: %d\n", status);
    }
}

/* --- 8. 示例4: 数据验证 (Example 4: Data Verification) --- */

void at24cxx_example_data_verify(void)
{
    uint8_t write_data[20] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE,
                               0x11, 0x22, 0x33, 0x44, 0x55,
                               0x66, 0x77, 0x88, 0x99, 0xAA,
                               0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    driver_status_t status;

    printf("\n=== Example 4: Data Verification ===\n");

    status = at24cxx_write(&g_at24c128_dev, 0x0200, write_data, 20);
    if (status != DRV_OK) {
        printf("Write Failed: %d\n", status);
        return;
    }

    HAL_Delay(10);

    status = at24cxx_verify(&g_at24c128_dev, 0x0200, write_data, 20);
    if (status == DRV_OK) {
        printf("Data Verification PASSED\n");
    } else {
        printf("Data Verification FAILED\n");
    }
}

/* --- 9. 示例5: 单字节操作 (Example 5: Single Byte Operations) --- */

void at24cxx_example_single_byte(void)
{
    uint8_t write_value = 0x55;
    uint8_t read_value;
    driver_status_t status;

    printf("\n=== Example 5: Single Byte Operations ===\n");

    status = at24cxx_write_byte(&g_at24c128_dev, 0x0300, write_value);
    if (status == DRV_OK) {
        printf("Write Byte OK: 0x%02X\n", write_value);
    } else {
        printf("Write Byte Failed: %d\n", status);
        return;
    }

    HAL_Delay(10);

    status = at24cxx_read_byte(&g_at24c128_dev, 0x0300, &read_value);
    if (status == DRV_OK) {
        printf("Read Byte OK: 0x%02X\n", read_value);
    } else {
        printf("Read Byte Failed: %d\n", status);
    }
}

/* --- 10. 示例6: 擦除操作 (Example 6: Erase Operation) --- */

void at24cxx_example_erase(void)
{
    uint8_t read_data[10];
    driver_status_t status;
    uint16_t i;
    bool all_ff;

    printf("\n=== Example 6: Erase Operation ===\n");

    status = at24cxx_erase(&g_at24c128_dev, 0x0400, 10);
    if (status == DRV_OK) {
        printf("Erase OK (10 bytes)\n");
    } else {
        printf("Erase Failed: %d\n", status);
        return;
    }

    HAL_Delay(10);

    status = at24cxx_read(&g_at24c128_dev, 0x0400, read_data, 10);
    if (status != DRV_OK) {
        printf("Read Failed: %d\n", status);
        return;
    }

    all_ff = true;
    for (i = 0; i < 10; i++) {
        if (read_data[i] != 0xFF) {
            all_ff = false;
            break;
        }
    }

    if (all_ff) {
        printf("Erase Verification OK (All 0xFF)\n");
    } else {
        printf("Erase Verification FAILED\n");
    }
}

/* --- 11. 示例7: 设备探测 (Example 7: Device Probe) --- */

void at24cxx_example_device_probe(void)
{
    driver_status_t status;
    at24cxx_config_t config;

    printf("\n=== Example 7: Device Probe ===\n");

    status = at24cxx_probe(&g_at24c128_dev);
    if (status == DRV_OK) {
        printf("Device Probe: FOUND\n");
    } else {
        printf("Device Probe: NOT FOUND\n");
        return;
    }

    status = at24cxx_get_config(&g_at24c128_dev, &config);
    if (status == DRV_OK) {
        printf("Device Address: 0x%02X\n", config.device_addr);
        printf("Page Size: %d bytes\n", config.page_size);
        printf("Capacity: %d bytes\n", config.capacity);
        printf("Address Bytes: %d\n", config.addr_bytes);
    }
}

/* --- 12. 主测试函数 (Main Test Function) --- */

void at24cxx_example_main(void)
{
    at24cxx_example_init();

    at24cxx_example_device_probe();
    at24cxx_example_basic_rw();
    at24cxx_example_string_storage();
    at24cxx_example_cross_page_write();
    at24cxx_example_data_verify();
    at24cxx_example_single_byte();
    at24cxx_example_erase();

    printf("\n=== All Examples Completed ===\n");
}
