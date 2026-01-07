#include "at24cxx_driver.h"
#include <string.h>
#include <stdlib.h>

/* --- 1. 私有函数声明 (Private Functions) --- */

static driver_status_t static_wait_write_complete(at24cxx_dev_t *p_dev);
static driver_status_t static_write_page(at24cxx_dev_t *p_dev,
                                         uint16_t address,
                                         const uint8_t *p_data,
                                         uint16_t length);

/* --- 2. 公共函数实现 (Public Functions) --- */

driver_status_t at24cxx_init(at24cxx_dev_t *p_dev,
                             const driver_i2c_ops_t *p_i2c_ops,
                             const at24cxx_config_t *p_config,
                             void *i2c_user_data)
{
    if (p_dev == NULL || p_i2c_ops == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_i2c_ops->read_reg == NULL || p_i2c_ops->write_reg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    memset(p_dev, 0, sizeof(at24cxx_dev_t));

    p_dev->i2c_ops = *p_i2c_ops;
    p_dev->i2c_user_data = i2c_user_data;

    if (p_config != NULL) {
        p_dev->config = *p_config;
    } else {
        p_dev->config = AT24CXX_GET_DEFAULT_CONFIG(AT24CXX_MODEL_128);
    }

    p_dev->is_initialized = true;

    return DRV_OK;
}

driver_status_t at24cxx_probe(at24cxx_dev_t *p_dev)
{
    uint8_t dummy;

    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_dev->i2c_ops.read_reg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    driver_status_t status = p_dev->i2c_ops.read_reg(p_dev->i2c_user_data,
                                                       (p_dev->config.device_addr >> 1),
                                                       0,
                                                       &dummy,
                                                       1);

    if (status == DRV_OK) {
        return DRV_OK;
    } else {
        return DRV_ERR_IO;
    }
}

driver_status_t at24cxx_read(at24cxx_dev_t *p_dev,
                             uint16_t address,
                             uint8_t *p_data,
                             uint16_t length)
{
    uint8_t addr_buf[2];
    uint8_t addr_len;
    uint8_t dev_addr_7bit;
    driver_status_t status;

    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_data == NULL || length == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_dev->i2c_ops.write_reg == NULL || p_dev->i2c_ops.read_reg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    if (address + length > p_dev->config.capacity) {
        return DRV_ERR_INVALID_VAL;
    }

    dev_addr_7bit = p_dev->config.device_addr >> 1;
    addr_len = p_dev->config.addr_bytes;

    if (addr_len == 1) {
        status = p_dev->i2c_ops.read_reg(p_dev->i2c_user_data,
                                          dev_addr_7bit,
                                          (uint8_t)address,
                                          p_data,
                                          length);
    } else {
        addr_buf[0] = (uint8_t)(address >> 8);
        addr_buf[1] = (uint8_t)address;

        status = p_dev->i2c_ops.write_reg(p_dev->i2c_user_data,
                                           dev_addr_7bit,
                                           addr_buf[0],
                                           &addr_buf[1],
                                           1);

        if (status != DRV_OK) {
            return status;
        }

        status = p_dev->i2c_ops.read_reg(p_dev->i2c_user_data,
                                          dev_addr_7bit,
                                          addr_buf[1],
                                          p_data,
                                          length);
    }

    return status;
}

driver_status_t at24cxx_write(at24cxx_dev_t *p_dev,
                              uint16_t address,
                              const uint8_t *p_data,
                              uint16_t length)
{
    uint16_t bytes_written;
    uint16_t bytes_to_write;
    uint16_t page_space;
    uint16_t current_addr;
    driver_status_t status;

    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_data == NULL || length == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_dev->i2c_ops.write_reg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    if (address + length > p_dev->config.capacity) {
        return DRV_ERR_INVALID_VAL;
    }

    bytes_written = 0;
    current_addr = address;

    while (bytes_written < length) {
        page_space = p_dev->config.page_size - (current_addr % p_dev->config.page_size);
        bytes_to_write = (length - bytes_written) < page_space ?
                         (length - bytes_written) : page_space;

        status = static_write_page(p_dev, current_addr,
                                   p_data + bytes_written, bytes_to_write);

        if (status != DRV_OK) {
            return status;
        }

        bytes_written += bytes_to_write;
        current_addr += bytes_to_write;
    }

    return DRV_OK;
}

driver_status_t at24cxx_erase(at24cxx_dev_t *p_dev,
                              uint16_t address,
                              uint16_t length)
{
    uint8_t *p_erase_buf;
    uint16_t i;
    driver_status_t status;

    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (length == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    if (address + length > p_dev->config.capacity) {
        return DRV_ERR_INVALID_VAL;
    }

    p_erase_buf = (uint8_t *)malloc(length);
    if (p_erase_buf == NULL) {
        return DRV_ERR_NO_MEM;
    }

    for (i = 0; i < length; i++) {
        p_erase_buf[i] = 0xFF;
    }

    status = at24cxx_write(p_dev, address, p_erase_buf, length);

    free(p_erase_buf);

    return status;
}

driver_status_t at24cxx_read_byte(at24cxx_dev_t *p_dev,
                                  uint16_t address,
                                  uint8_t *p_data)
{
    return at24cxx_read(p_dev, address, p_data, 1);
}

driver_status_t at24cxx_write_byte(at24cxx_dev_t *p_dev,
                                   uint16_t address,
                                   uint8_t data)
{
    return at24cxx_write(p_dev, address, &data, 1);
}

driver_status_t at24cxx_verify(at24cxx_dev_t *p_dev,
                               uint16_t address,
                               const uint8_t *p_data,
                               uint16_t length)
{
    uint8_t *p_read_buf;
    uint16_t i;
    driver_status_t status;

    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_data == NULL || length == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    p_read_buf = (uint8_t *)malloc(length);
    if (p_read_buf == NULL) {
        return DRV_ERR_NO_MEM;
    }

    status = at24cxx_read(p_dev, address, p_read_buf, length);

    if (status == DRV_OK) {
        for (i = 0; i < length; i++) {
            if (p_read_buf[i] != p_data[i]) {
                status = DRV_ERR_COMMON;
                break;
            }
        }
    }

    free(p_read_buf);

    return status;
}

driver_status_t at24cxx_get_config(at24cxx_dev_t *p_dev,
                                   at24cxx_config_t *p_config)
{
    if (p_dev == NULL || !p_dev->is_initialized) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_config == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    *p_config = p_dev->config;

    return DRV_OK;
}

uint16_t at24cxx_get_capacity(at24cxx_dev_t *p_dev)
{
    if (p_dev == NULL || !p_dev->is_initialized) {
        return 0;
    }

    return p_dev->config.capacity;
}

uint16_t at24cxx_get_page_size(at24cxx_dev_t *p_dev)
{
    if (p_dev == NULL || !p_dev->is_initialized) {
        return 0;
    }

    return p_dev->config.page_size;
}

/* --- 3. 私有函数实现 (Private Functions) --- */

static driver_status_t static_wait_write_complete(at24cxx_dev_t *p_dev)
{
    uint32_t retry_count;
    uint32_t max_retries;
    uint8_t dummy;
    driver_status_t status;
    uint8_t dev_addr_7bit;

    max_retries = p_dev->config.write_timeout_ms;
    dev_addr_7bit = p_dev->config.device_addr >> 1;

    for (retry_count = 0; retry_count < max_retries; retry_count++) {
        status = p_dev->i2c_ops.read_reg(p_dev->i2c_user_data,
                                            dev_addr_7bit,
                                            0,
                                            &dummy,
                                            1);

        if (status == DRV_OK) {
            return DRV_OK;
        }
    }

    return DRV_ERR_TIMEOUT;
}

static driver_status_t static_write_page(at24cxx_dev_t *p_dev,
                                         uint16_t address,
                                         const uint8_t *p_data,
                                         uint16_t length)
{
    uint8_t *p_write_buf;
    uint8_t addr_len;
    uint8_t dev_addr_7bit;
    driver_status_t status;

    dev_addr_7bit = p_dev->config.device_addr >> 1;
    addr_len = p_dev->config.addr_bytes;

    p_write_buf = (uint8_t *)malloc(addr_len + length);
    if (p_write_buf == NULL) {
        return DRV_ERR_NO_MEM;
    }

    if (addr_len == 1) {
        p_write_buf[0] = (uint8_t)address;
        memcpy(p_write_buf + 1, p_data, length);

        status = p_dev->i2c_ops.write_reg(p_dev->i2c_user_data,
                                            dev_addr_7bit,
                                            p_write_buf[0],
                                            p_write_buf + 1,
                                            length);
    } else {
        p_write_buf[0] = (uint8_t)(address >> 8);
        p_write_buf[1] = (uint8_t)address;
        memcpy(p_write_buf + 2, p_data, length);

        status = p_dev->i2c_ops.write_reg(p_dev->i2c_user_data,
                                            dev_addr_7bit,
                                            p_write_buf[0],
                                            p_write_buf + 1,
                                            length + 1);
    }

    free(p_write_buf);

    if (status != DRV_OK) {
        return status;
    }

    status = static_wait_write_complete(p_dev);

    return status;
}
