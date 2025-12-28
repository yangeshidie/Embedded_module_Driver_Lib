/**
 * @file driver_types.h
 * @brief 核心类型定义与通用宏 (Core Type Definitions & Common Macros)
 * @note  遵循严格的编码规范：stdint类型，统一错误码
 */

#ifndef _DRIVER_TYPES_H_
#define _DRIVER_TYPES_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* --- 1. 统一状态码 (Standardized Status Codes) --- */
typedef enum {
    DRV_OK              = 0,    /**< 操作成功 */
    DRV_ERR_COMMON      = -1,   /**< 通用错误 */
    DRV_ERR_INVALID_VAL = -2,   /**< 无效参数/值 */
    DRV_ERR_TIMEOUT     = -3,   /**< 操作超时 */
    DRV_ERR_BUSY        = -4,   /**< 设备忙 */
    DRV_ERR_IO          = -5,   /**< 硬件IO/总线错误 */
    DRV_ERR_NO_MEM      = -6,   /**< 内存不足 (若涉及) */
    DRV_ERR_NOT_SUPPORT = -7    /**< 功能不支持 */
} driver_status_t;

/* --- 2. 通用宏定义 (Common Macros) --- */

/* 位操作宏 */
#define DRV_BIT(n)              (1UL << (n))
#define DRV_SET_BIT(reg, bit)   ((reg) |= (bit))
#define DRV_CLR_BIT(reg, bit)   ((reg) &= ~(bit))
#define DRV_READ_BIT(reg, bit)  ((reg) & (bit))

/* 数组元素计数 */
#define DRV_ARRAY_SIZE(a)       (sizeof(a) / sizeof((a)[0]))

/* 字节序处理 (大小端兼容) */
#define DRV_U16_HIGH(x)         ((uint8_t)(((x) >> 8) & 0xFF))
#define DRV_U16_LOW(x)          ((uint8_t)((x) & 0xFF))
#define DRV_MAKE_U16(h, l)      ((uint16_t)(((uint16_t)(h) << 8) | (l)))

#define DRV_U32_BYTE0(x)        ((uint8_t)(((x) >> 24) & 0xFF))
#define DRV_U32_BYTE1(x)        ((uint8_t)(((x) >> 16) & 0xFF))
#define DRV_U32_BYTE2(x)        ((uint8_t)(((x) >> 8) & 0xFF))
#define DRV_U32_BYTE3(x)        ((uint8_t)((x) & 0xFF))
#define DRV_MAKE_U32(b0, b1, b2, b3) \
    ((uint32_t)(((uint32_t)(b0) << 24) | ((uint32_t)(b1) << 16) | ((uint32_t)(b2) << 8) | (b3)))

#endif /* _DRIVER_TYPES_H_ */