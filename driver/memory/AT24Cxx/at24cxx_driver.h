#ifndef AT24CXX_DRIVER_H
#define AT24CXX_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

#include "../../../core/driver_types.h"
#include "../../../core/driver_interfaces.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- 1. 设备型号定义 (Device Models) --- */
typedef enum {
    AT24CXX_MODEL_01  = 1,    /**< AT24C01:  128 bytes,   8-byte page */
    AT24CXX_MODEL_02  = 2,    /**< AT24C02:  256 bytes,   8-byte page */
    AT24CXX_MODEL_04  = 4,    /**< AT24C04:  512 bytes,  16-byte page */
    AT24CXX_MODEL_08  = 8,    /**< AT24C08:  1KB,        16-byte page */
    AT24CXX_MODEL_16  = 16,   /**< AT24C16:  2KB,        16-byte page */
    AT24CXX_MODEL_32  = 32,   /**< AT24C32:  4KB,        32-byte page */
    AT24CXX_MODEL_64  = 64,   /**< AT24C64:  8KB,        32-byte page */
    AT24CXX_MODEL_128 = 128,  /**< AT24C128: 16KB,       64-byte page */
    AT24CXX_MODEL_256 = 256   /**< AT24C256: 32KB,       64-byte page */
} at24cxx_model_t;

/* --- 2. 设备配置结构体 (Device Configuration) --- */
typedef struct {
    uint8_t  device_addr;      /**< I2C设备地址 (7位地址, 如 0x50) */
    uint16_t page_size;        /**< 页写入大小 (bytes) */
    uint16_t capacity;         /**< 总容量 (bytes) */
    uint8_t  addr_bytes;       /**< 地址字节数 (1 or 2) */
    uint32_t write_timeout_ms; /**< 写入超时时间 (ms), 默认10ms */
} at24cxx_config_t;

/* --- 3. 设备句柄结构体 (Device Handle) --- */
typedef struct {
    driver_i2c_ops_t i2c_ops;      /**< I2C操作接口 */
    at24cxx_config_t config;       /**< 设备配置 */
    void *i2c_user_data;           /**< I2C用户数据 (如 I2C_HandleTypeDef*) */
    bool is_initialized;           /**< 初始化标志 */
} at24cxx_dev_t;

/* --- 4. API 函数声明 (API Functions) --- */

/**
 * @brief 初始化 AT24Cxx EEPROM 设备
 * @param p_dev 设备句柄指针
 * @param p_i2c_ops I2C操作接口
 * @param p_config 设备配置 (可为NULL, 使用默认配置)
 * @param i2c_user_data I2C用户数据
 * @return driver_status_t
 * @note 必须先调用此函数初始化设备
 */
driver_status_t at24cxx_init(at24cxx_dev_t *p_dev,
                             const driver_i2c_ops_t *p_i2c_ops,
                             const at24cxx_config_t *p_config,
                             void *i2c_user_data);

/**
 * @brief 检测 EEPROM 是否存在
 * @param p_dev 设备句柄指针
 * @return driver_status_t
 * @retval DRV_OK 设备存在
 * @retval DRV_ERR_IO 设备不存在或通信失败
 */
driver_status_t at24cxx_probe(at24cxx_dev_t *p_dev);

/**
 * @brief 从 EEPROM 读取数据
 * @param p_dev 设备句柄指针
 * @param address 起始地址
 * @param p_data 读取数据缓冲区
 * @param length 读取长度 (bytes)
 * @return driver_status_t
 */
driver_status_t at24cxx_read(at24cxx_dev_t *p_dev,
                             uint16_t address,
                             uint8_t *p_data,
                             uint16_t length);

/**
 * @brief 向 EEPROM 写入数据
 * @param p_dev 设备句柄指针
 * @param address 起始地址
 * @param p_data 写入数据缓冲区
 * @param length 写入长度 (bytes)
 * @return driver_status_t
 * @note 内部自动处理页写入优化
 */
driver_status_t at24cxx_write(at24cxx_dev_t *p_dev,
                              uint16_t address,
                              const uint8_t *p_data,
                              uint16_t length);

/**
 * @brief 擦除 EEPROM (填充为 0xFF)
 * @param p_dev 设备句柄指针
 * @param address 起始地址
 * @param length 擦除长度 (bytes)
 * @return driver_status_t
 */
driver_status_t at24cxx_erase(at24cxx_dev_t *p_dev,
                              uint16_t address,
                              uint16_t length);

/**
 * @brief 读取单个字节
 * @param p_dev 设备句柄指针
 * @param address 地址
 * @param p_data 读取的数据指针
 * @return driver_status_t
 */
driver_status_t at24cxx_read_byte(at24cxx_dev_t *p_dev,
                                  uint16_t address,
                                  uint8_t *p_data);

/**
 * @brief 写入单个字节
 * @param p_dev 设备句柄指针
 * @param address 地址
 * @param data 写入的数据
 * @return driver_status_t
 */
driver_status_t at24cxx_write_byte(at24cxx_dev_t *p_dev,
                                   uint16_t address,
                                   uint8_t data);

/**
 * @brief 比较 EEPROM 中的数据与缓冲区数据
 * @param p_dev 设备句柄指针
 * @param address 起始地址
 * @param p_data 比较数据缓冲区
 * @param length 比较长度 (bytes)
 * @return driver_status_t
 * @retval DRV_OK 数据一致
 * @retval DRV_ERR_COMMON 数据不一致
 */
driver_status_t at24cxx_verify(at24cxx_dev_t *p_dev,
                               uint16_t address,
                               const uint8_t *p_data,
                               uint16_t length);

/**
 * @brief 获取设备配置信息
 * @param p_dev 设备句柄指针
 * @param p_config 输出配置信息
 * @return driver_status_t
 */
driver_status_t at24cxx_get_config(at24cxx_dev_t *p_dev,
                                   at24cxx_config_t *p_config);

/**
 * @brief 获取设备容量
 * @param p_dev 设备句柄指针
 * @return 容量 (bytes)
 */
uint16_t at24cxx_get_capacity(at24cxx_dev_t *p_dev);

/**
 * @brief 获取页大小
 * @param p_dev 设备句柄指针
 * @return 页大小 (bytes)
 */
uint16_t at24cxx_get_page_size(at24cxx_dev_t *p_dev);

/* --- 5. 辅助宏 (Helper Macros) --- */

/**
 * @brief 获取默认配置
 * @param model 设备型号
 * @return at24cxx_config_t
 */
#define AT24CXX_GET_DEFAULT_CONFIG(model) ((at24cxx_config_t){ \
    .device_addr = 0xA0, \
    .page_size = (model) >= AT24CXX_MODEL_128 ? 64 : \
                 (model) >= AT24CXX_MODEL_32 ? 32 : \
                 (model) >= AT24CXX_MODEL_04 ? 16 : 8, \
    .capacity = (model) * 128, \
    .addr_bytes = (model) >= AT24CXX_MODEL_32 ? 2 : 1, \
    .write_timeout_ms = 10 \
})

#ifdef __cplusplus
}
#endif

#endif /* AT24CXX_DRIVER_H */
