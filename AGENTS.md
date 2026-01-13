# Agent Guidelines

## Build/Commands
No build system - this is a portable C driver library intended to be compiled into your embedded project. No tests, lints, or individual test commands available.

## Code Style (PDA Architecture)
- **Naming**: Variables/functions in `snake_case`, macros/enums `UPPER_CASE`, types `snake_case_t`
- **Types**: Only use `<stdint.h>` types (uint8_t, int16_t, uint32_t) - never int/short/char except for loop counters
- **Pointers**: Prefix with `p_` (recommended), static globals with `s_`, globals with `g_`, arrays with `_buf`/`_arr`
- **No Global State**: All device state must be in device handle struct passed to all APIs - no static globals for device context
- **Dependency Injection**: All hardware access (I2C/SPI/GPIO/time) injected via function pointer interfaces during init
- **Multi-byte Data**: Use bit shifts for byte order: `(buf[0] << 8) | buf[1]` - never pointer casting `*(int16_t*)buf`
- **Error Codes**: Return unified `driver_status_t` enum (DRV_OK, DRV_ERR_TIMEOUT, DRV_ERR_IO, etc.) - never pass through HAL errors
- **Files**: Use relative includes like `#include "../../../core/driver_types.h"` for cross-module dependencies
