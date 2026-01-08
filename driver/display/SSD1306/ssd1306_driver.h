#ifndef _SSD1306_DRIVER_H_
#define _SSD1306_DRIVER_H_

#include "../../../core/driver_types.h"
#include "../../../core/driver_interfaces.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_WIDTH            128
#define SSD1306_HEIGHT           64
#define SSD1306_PAGE_COUNT      8

typedef enum {
    SSD1306_ADDR_LOW  = 0x78,
    SSD1306_ADDR_HIGH = 0x7A
} ssd1306_addr_t;

typedef enum {
    SSD1306_COLOR_BLACK = 0,
    SSD1306_COLOR_WHITE = 1
} ssd1306_color_t;

typedef enum {
    SSD1306_FONT_SIZE_6x8 = 0,
    SSD1306_FONT_SIZE_8x16 = 1
} ssd1306_font_size_t;

typedef struct {
    uint8_t x;
    uint8_t y;
} ssd1306_point_t;

typedef struct {
    ssd1306_point_t start;
    ssd1306_point_t end;
} ssd1306_line_t;

typedef struct {
    ssd1306_point_t pos;
    uint8_t width;
    uint8_t height;
} ssd1306_rect_t;

typedef struct {
    const driver_i2c_ops_t *p_i2c_ops;
    const driver_time_ops_t *p_time_ops;
    void *p_bus_handle;
    uint8_t dev_addr;
    uint8_t display_buf[SSD1306_PAGE_COUNT][SSD1306_WIDTH];
    bool is_initialized;
    void *reserved;
} ssd1306_dev_t;

driver_status_t ssd1306_init(ssd1306_dev_t *p_dev, 
                             const driver_i2c_ops_t *p_i2c_ops,
                             const driver_time_ops_t *p_time_ops,
                             void *p_bus_handle,
                             uint8_t dev_addr);

driver_status_t ssd1306_deinit(ssd1306_dev_t *p_dev);

driver_status_t ssd1306_clear(ssd1306_dev_t *p_dev);

driver_status_t ssd1306_update_display(ssd1306_dev_t *p_dev);

driver_status_t ssd1306_set_pixel(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, ssd1306_color_t color);

driver_status_t ssd1306_draw_line(ssd1306_dev_t *p_dev, ssd1306_line_t *p_line, ssd1306_color_t color);

driver_status_t ssd1306_draw_rect(ssd1306_dev_t *p_dev, ssd1306_rect_t *p_rect, ssd1306_color_t color);

driver_status_t ssd1306_fill_rect(ssd1306_dev_t *p_dev, ssd1306_rect_t *p_rect, ssd1306_color_t color);

driver_status_t ssd1306_draw_char(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, char ch, ssd1306_font_size_t font_size, ssd1306_color_t color);

driver_status_t ssd1306_draw_string(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, const char *p_str, ssd1306_font_size_t font_size, ssd1306_color_t color);

driver_status_t ssd1306_draw_uint(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, uint32_t num, ssd1306_font_size_t font_size, ssd1306_color_t color);

driver_status_t ssd1306_draw_int(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, int32_t num, ssd1306_font_size_t font_size, ssd1306_color_t color);

driver_status_t ssd1306_display_on(ssd1306_dev_t *p_dev);

driver_status_t ssd1306_display_off(ssd1306_dev_t *p_dev);

#ifdef __cplusplus
}
#endif

#endif
