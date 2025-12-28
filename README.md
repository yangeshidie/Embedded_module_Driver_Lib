# Embedded_module_Driver_Lib
🏗️ Portable Driver Architecture (PDA) 设计原则
本项目遵循 PDA (Portable Driver Architecture) 设计规范，旨在构建一套 “一次编写，到处运行” 的高可移植性驱动库。无论是在 8-bit 单片机、高性能 MCU (STM32/ESP32)、RTOS 环境还是 Linux 用户态，驱动核心代码均无需修改。
核心设计哲学
"Driver Logic / Hardware Interface Separation"
驱动只负责业务逻辑（寄存器操作、协议解析），一切与硬件相关的操作（GPIO、总线、时延）均通过抽象接口注入。
📐 架构分层
系统严格划分为三层，层级间单向依赖：
APP / Business Layer (顶层)
业务逻辑调用驱动提供的 API (如 mpu_get_accel())。
Device Driver Layer (中间层 - 本库核心)
纯 C 实现：不包含任何厂商 HAL 库头文件 (如 stm32_hal.h)。
零全局状态：所有上下文通过结构体句柄传递，支持多实例。
依赖注入：通过函数指针调用底层的读写与延时能力。
Board Support Package (BSP) / Adapter Layer (底层)
硬件适配：实现驱动层定义的接口 (read/write/delay)。
资源管理：负责 GPIO 初始化、时钟配置、中断管理。
🛡️ 五大实现原则
1. 严格的能力注入 (Dependency Injection)
驱动内部不直接调用 HAL_I2C_Write 或 osDelay。所有的外部依赖（I2C/SPI 读写、毫秒延时、互斥锁）必须在初始化时通过结构体指针传入。
2. 上下文与对象化 (Context & Object-Oriented)
句柄优先：所有 API 第一个参数必须是设备句柄指针 (device_t *dev)。
无隐式状态：禁止使用 static 全局变量存储设备状态，确保同一份驱动代码可同时驱动多个相同型号的硬件。
透传机制：读写接口必须携带 void *user_data 参数，以便适配层区分不同的硬件总线。
3. 内存管理所有权 (Memory Ownership)
调用者分配：驱动层不进行动态内存分配 (malloc)。设备结构体由上层应用静态定义或在栈/堆中分配后传入。
确保确定性：避免内存碎片，适应对安全要求苛刻的裸机环境。
4. 时序与并发抽象 (Timing & Concurrency)
时间抽象：必须注入 delay_ms 或 get_tick 函数，以兼容裸机循环延时和 RTOS 阻塞延时。
原子操作：不假设总线独占，驱动仅负责逻辑。如果需要原子传输，应由适配层处理互斥锁 (Mutex)。
5. 数据与平台无关性 (Platform Independence)
类型明确：只使用 <stdint.h> 定义的标准类型 (uint8_t, int32_t)。
字节序处理：内部通过移位操作处理多字节数据，严禁直接指针强转，兼容大小端 CPU。
