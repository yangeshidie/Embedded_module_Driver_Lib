/**
 * @file driver_template.c
 * @brief [模块名] 驱动实现文件
 * @note  遵循“无隐式状态”和“严格分层”原则
 */

#include "driver_template.h"

/* --- 私有宏定义 (Private Macros) --- */
#define REG_WHO_AM_I        0x00
#define REG_CTRL_1          0x01
#define REG_DATA_START      0x10
#define EXPECTED_CHIP_ID    0xA5

/* --- 私有辅助函数声明 (Private Function Prototypes) --- */
static driver_status_t static_read_reg(template_dev_t *p_dev, uint8_t reg, uint8_t *p_data, uint32_t len);
static driver_status_t static_write_reg(template_dev_t *p_dev, uint8_t reg, const uint8_t *p_data, uint32_t len);

/* --- API 实现 (API Implementation) --- */

driver_status_t template_init(template_dev_t *p_dev, const template_ops_t *p_ops, void *bus_ctx, uint8_t addr)
{
    /* 1. 参数检查 */
    if (p_dev == NULL || p_ops == NULL || bus_ctx == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    if (p_ops->i2c_ops.read_reg == NULL || p_ops->i2c_ops.write_reg == NULL || p_ops->time_ops.delay_ms == NULL) {
        return DRV_ERR_NOT_SUPPORT; /* 缺少必要接口 */
    }

    /* 2. 对象初始化 */
    p_dev->ops = *p_ops;         /* 拷贝接口结构体 */
    p_dev->bus_ctx = bus_ctx;    /* 保存总线上下文 */
    p_dev->dev_addr = addr;
    p_dev->is_initialized = false;

    /* 3. 硬件复位序列 (示例) */
    p_dev->ops.time_ops.delay_ms(10); // 上电延时

    /* 4. ID 校验 */
    uint8_t chip_id = 0;
    driver_status_t ret = static_read_reg(p_dev, REG_WHO_AM_I, &chip_id, 1);
    
    if (ret != DRV_OK) {
        return ret;
    }
    if (chip_id != EXPECTED_CHIP_ID) {
        return DRV_ERR_COMMON; // ID 不匹配
    }

    /* 5. 标记成功 */
    p_dev->is_initialized = true;
    return DRV_OK;
}

driver_status_t template_read_value(template_dev_t *p_dev, float *p_val)
{
    if (p_dev == NULL || p_val == NULL) return DRV_ERR_INVALID_VAL;
    if (!p_dev->is_initialized) return DRV_ERR_COMMON;

    uint8_t raw_buf[2];
    driver_status_t ret;

    /* 读取2字节数据 */
    ret = static_read_reg(p_dev, REG_DATA_START, raw_buf, 2);
    if (ret != DRV_OK) return ret;

    /* 
     * 数据处理：严禁直接指针强转 *(int16_t*)raw_buf 
     * 必须使用移位操作以兼容大小端 CPU
     */
    int16_t raw_val = (int16_t)((raw_buf[0] << 8) | raw_buf[1]); // 假设大端模式 (High byte first)

    /* 转换为物理量 */
    *p_val = (float)raw_val * 0.01f;

    return DRV_OK;
}

driver_status_t template_set_config(template_dev_t *p_dev, template_cfg_t new_cfg)
{
    if (p_dev == NULL) return DRV_ERR_INVALID_VAL;

    // 写入寄存器逻辑...
    uint8_t reg_val = new_cfg.sample_rate | (new_cfg.range << 4);
    
    if (static_write_reg(p_dev, REG_CTRL_1, &reg_val, 1) != DRV_OK) {
        return DRV_ERR_IO;
    }

    /* 更新本地副本 */
    p_dev->config = new_cfg;
    return DRV_OK;
}

/* --- 私有辅助函数实现 (Private Functions) --- */

static driver_status_t static_read_reg(template_dev_t *p_dev, uint8_t reg, uint8_t *p_data, uint32_t len)
{
    /* 调用注入的接口，透传 bus_ctx */
    int32_t res = p_dev->ops.i2c_ops.read_reg(p_dev->bus_ctx, p_dev->dev_addr, reg, p_data, len);
    
    /* 统一错误码映射：假设底层返回0为成功 */
    return (res == 0) ? DRV_OK : DRV_ERR_IO;
}

static driver_status_t static_write_reg(template_dev_t *p_dev, uint8_t reg, const uint8_t *p_data, uint32_t len)
{
    int32_t res = p_dev->ops.i2c_ops.write_reg(p_dev->bus_ctx, p_dev->dev_addr, reg, p_data, len);
    return (res == 0) ? DRV_OK : DRV_ERR_IO;
}