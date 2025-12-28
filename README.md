# Embedded_module_Driver_Lib

# 🏗️ Portable Driver Architecture (PDA) 设计原则

本项目遵循 **PDA (Portable Driver Architecture)** 设计规范，旨在构建一套 **“一次编写，到处运行”** 的高可移植性驱动库。无论是在 8-bit 单片机、高性能 MCU (STM32/ESP32)、RTOS 环境还是 Linux 用户态，驱动核心代码均无需修改。

## 核心设计哲学
> **"Driver Logic / Hardware Interface Separation"**
> 驱动只负责业务逻辑（寄存器操作、协议解析），一切与硬件相关的操作（GPIO、总线、时延）均通过抽象接口注入。

## 📐 架构分层
系统严格划分为三层，层级间单向依赖：

1.  **APP / Business Layer** (顶层)
    *   业务逻辑调用驱动提供的 API (如 `mpu_get_accel()`)。
2.  **Device Driver Layer** (中间层 - **本库核心**)
    *   **纯 C 实现**：不包含任何厂商 HAL 库头文件 (如 `stm32_hal.h`)。
    *   **零全局状态**：所有上下文通过结构体句柄传递，支持多实例。
    *   **依赖注入**：通过函数指针调用底层的读写与延时能力。
3.  **Board Support Package (BSP) / Adapter Layer** (底层)
    *   **硬件适配**：实现驱动层定义的接口 (read/write/delay)。
    *   **资源管理**：负责 GPIO 初始化、时钟配置、中断管理。

#### 🛡️ 驱动架构六大核心原则 (Design Principles)

本驱动库遵循“高内聚、低耦合”的设计哲学，旨在实现**一次编写，到处编译**。

**1. 💉 严格的能力注入 (Dependency Injection)**
*   **原则**：驱动内部**严禁**包含任何具体硬件平台的头文件（如 `<stm32.h>`, `<esp_hal.h>`）。
*   **实现**：所有的外部依赖（I2C/SPI 读写、毫秒延时、GPIO 操作）必须在初始化时通过**函数指针结构体**注入。驱动只调用接口，不关心实现。

**2. 🧩 上下文与对象化 (Context & Object-Oriented)**
*   **句柄优先**：所有 API 的第一个参数必须是设备句柄指针 (`device_t *dev`)。
*   **无隐式状态**：严禁使用 `static` 全局变量存储设备状态，确保同一份驱动代码可同时驱动多个挂载在不同总线上的相同型号硬件。
*   **透传机制**：底层读写接口必须携带 `void *user_data` 参数，以便适配层区分不同的硬件总线实例（如 `I2C1` vs `I2C2`）。

**3. 📦 内存管理所有权 (Memory Ownership)**
*   **调用者分配**：驱动层**不进行**动态内存分配 (`malloc`)。
*   **确定性**：设备句柄结构体由上层应用静态定义（`.data`/`.bss`）或在栈/堆中分配后传入。这确保了内存使用的确定性，完美适配对安全要求苛刻的裸机环境。

**4. ⏱️ 时序与并发抽象 (Timing & Concurrency)**
*   **时间抽象**：必须注入 `delay_ms` 或 `get_tick` 函数，以兼容裸机（循环延时）和 RTOS（阻塞延时）。
*   **无锁设计**：驱动层专注于业务逻辑，不包含互斥锁。如果需要线程安全，应由适配层（Adapter）在注入的读写函数中处理互斥锁 (Mutex)。

**5. 🌐 数据与平台无关性 (Platform Independence)**
*   **类型明确**：只使用 `<stdint.h>` 定义的标准定宽类型 (`uint8_t`, `int32_t`)。
*   **字节序处理**：内部通过**移位操作** (`<<`, `>>`) 处理多字节数据，**严禁**直接进行指针强转 (`*(uint16_t*)buf`)，确保兼容大小端 CPU 及内存对齐要求。

**6. 🚦 统一错误码 (Standardized Error Codes)**
*   **屏蔽差异**：驱动层不透传底层 HAL 的错误码。
*   **统一接口**：所有 API 返回库内定义的统一枚举（如 `DRV_OK`, `DRV_ERR_TIMEOUT`），让上层应用逻辑与底层硬件彻底解耦。

---
