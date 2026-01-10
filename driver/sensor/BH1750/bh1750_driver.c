/**
 * @file bh1750_driver.c
 * @brief BH1750FVI光照传感器驱动实现
 */

#include "bh1750_driver.h"

/* --- 私有函数声明 --- */

/**
 * @brief 写命令到BH1750
 * @param p_dev 设备句柄指针
 * @param cmd 命令字节
 * @return driver_status_t
 */
static driver_status_t bh1750_write_cmd(bh1750_dev_t *p_dev, uint8_t cmd);

/**
 * @brief 读取2字节数据
 * @param p_dev 设备句柄指针
 * @param p_data 数据缓冲区
 * @return driver_status_t
 */
static driver_status_t bh1750_read_data(bh1750_dev_t *p_dev, uint8_t *p_data);

/* --- 私有函数实现 --- */

static driver_status_t bh1750_write_cmd(bh1750_dev_t *p_dev, uint8_t cmd)
{
    if (p_dev == NULL || p_dev->p_i2c_ops == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    return p_dev->p_i2c_ops->write_reg(p_dev->i2c_ctx, p_dev->dev_addr, cmd, NULL, 0);
}

static driver_status_t bh1750_read_data(bh1750_dev_t *p_dev, uint8_t *p_data)
{
    if (p_dev == NULL || p_dev->p_i2c_ops == NULL || p_data == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    return p_dev->p_i2c_ops->read_reg(p_dev->i2c_ctx, p_dev->dev_addr, 0x00, p_data, 2);
}

/* --- 公共API实现 --- */

driver_status_t bh1750_init(bh1750_dev_t *p_dev,
                            const driver_i2c_ops_t *p_i2c_ops,
                            const driver_time_ops_t *p_time_ops,
                            void *i2c_ctx,
                            uint8_t dev_addr,
                            const bh1750_config_t *p_config)
{
    if (p_dev == NULL || p_i2c_ops == NULL || p_time_ops == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_i2c_ops->write_reg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    p_dev->p_i2c_ops = p_i2c_ops;
    p_dev->p_time_ops = p_time_ops;
    p_dev->i2c_ctx = i2c_ctx;
    p_dev->dev_addr = dev_addr;
    p_dev->is_initialized = 0;

    driver_status_t ret = bh1750_write_cmd(p_dev, BH1750_CMD_POWER_ON);
    if (ret != DRV_OK) {
        return ret;
    }

    if (p_config != NULL) {
        ret = bh1750_set_sensitivity(p_dev, p_config->mt_reg);
        if (ret != DRV_OK) {
            return ret;
        }
    } else {
        p_dev->mt_reg = 69;
    }

    ret = bh1750_set_mode(p_dev, BH1750_MODE_CON_H_RES2);
    if (ret != DRV_OK) {
        return ret;
    }

    p_dev->is_initialized = 1;

    return DRV_OK;
}

driver_status_t bh1750_power_down(bh1750_dev_t *p_dev)
{
    if (p_dev == NULL || p_dev->is_initialized == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    return bh1750_write_cmd(p_dev, BH1750_CMD_POWER_DOWN);
}

driver_status_t bh1750_power_on(bh1750_dev_t *p_dev)
{
    if (p_dev == NULL || p_dev->is_initialized == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    return bh1750_write_cmd(p_dev, BH1750_CMD_POWER_ON);
}

driver_status_t bh1750_reset(bh1750_dev_t *p_dev)
{
    if (p_dev == NULL || p_dev->is_initialized == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    return bh1750_write_cmd(p_dev, BH1750_CMD_RESET);
}

driver_status_t bh1750_set_mode(bh1750_dev_t *p_dev, bh1750_mode_t mode)
{
    if (p_dev == NULL || p_dev->is_initialized == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    uint8_t cmd;

    switch (mode) {
        case BH1750_MODE_CON_H_RES:
            cmd = BH1750_CMD_CON_H_RES;
            break;
        case BH1750_MODE_CON_H_RES2:
            cmd = BH1750_CMD_CON_H_RES2;
            break;
        case BH1750_MODE_CON_L_RES:
            cmd = BH1750_CMD_CON_L_RES;
            break;
        case BH1750_MODE_ONE_H_RES:
            cmd = BH1750_CMD_ONE_H_RES;
            break;
        case BH1750_MODE_ONE_H_RES2:
            cmd = BH1750_CMD_ONE_H_RES2;
            break;
        case BH1750_MODE_ONE_L_RES:
            cmd = BH1750_CMD_ONE_L_RES;
            break;
        default:
            return DRV_ERR_INVALID_VAL;
    }

    driver_status_t ret = bh1750_write_cmd(p_dev, cmd);
    if (ret == DRV_OK) {
        p_dev->mode = mode;
    }

    return ret;
}

driver_status_t bh1750_set_sensitivity(bh1750_dev_t *p_dev, uint8_t mt_reg)
{
    if (p_dev == NULL || p_dev->is_initialized == 0) {
        return DRV_ERR_INVALID_VAL;
    }

    if (mt_reg < 31) {
        mt_reg = 31;
    } else if (mt_reg > 254) {
        mt_reg = 254;
    }

    p_dev->mt_reg = mt_reg;

    driver_status_t ret = bh1750_write_cmd(p_dev, BH1750_CMD_MTREG_HIGH | (mt_reg >> 5));
    if (ret != DRV_OK) {
        return ret;
    }

    ret = bh1750_write_cmd(p_dev, BH1750_CMD_MTREG_LOW | (mt_reg & 0x1F));
    if (ret != DRV_OK) {
        return ret;
    }

    return bh1750_set_mode(p_dev, p_dev->mode);
}

driver_status_t bh1750_read_raw(bh1750_dev_t *p_dev, uint16_t *p_raw_data)
{
    if (p_dev == NULL || p_dev->is_initialized == 0 || p_raw_data == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    uint8_t data_buf[2];
    driver_status_t ret = bh1750_read_data(p_dev, data_buf);
    if (ret != DRV_OK) {
        return ret;
    }

    *p_raw_data = DRV_MAKE_U16(data_buf[0], data_buf[1]);

    return DRV_OK;
}

driver_status_t bh1750_read_lux(bh1750_dev_t *p_dev, float *p_lux)
{
    if (p_dev == NULL || p_dev->is_initialized == 0 || p_lux == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    uint16_t raw_data;
    driver_status_t ret = bh1750_read_raw(p_dev, &raw_data);
    if (ret != DRV_OK) {
        return ret;
    }

    float lux = (float)(raw_data * 5 * 69) / (6.0f * p_dev->mt_reg);

    if (p_dev->mode == BH1750_MODE_CON_H_RES2 || p_dev->mode == BH1750_MODE_ONE_H_RES2) {
        lux = lux / 2.0f;
    }

    *p_lux = lux;

    return DRV_OK;
}

driver_status_t bh1750_measure_lux(bh1750_dev_t *p_dev, float *p_lux)
{
    if (p_dev == NULL || p_dev->is_initialized == 0 || p_lux == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    if (p_dev->p_time_ops == NULL || p_dev->p_time_ops->delay_ms == NULL) {
        return DRV_ERR_INVALID_VAL;
    }

    driver_status_t ret = bh1750_set_mode(p_dev, BH1750_MODE_ONE_H_RES2);
    if (ret != DRV_OK) {
        return ret;
    }

    p_dev->p_time_ops->delay_ms(180);

    ret = bh1750_read_lux(p_dev, p_lux);
    if (ret != DRV_OK) {
        return ret;
    }

    return bh1750_power_down(p_dev);
}
