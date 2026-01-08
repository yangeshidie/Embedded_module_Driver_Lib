#include "ssd1306_driver.h"
#include "ssd1306_font.h"
#include <string.h>

#define SSD1306_CMD_SET_CONTRAST          0x81
#define SSD1306_CMD_DISPLAY_ON_RESUME     0xA4
#define SSD1306_CMD_DISPLAY_ON            0xAF
#define SSD1306_CMD_DISPLAY_OFF           0xAE
#define SSD1306_CMD_NORMAL_DISPLAY        0xA6
#define SSD1306_CMD_INVERSE_DISPLAY       0xA7
#define SSD1306_CMD_SET_MULTIPLEX_RATIO   0xA8
#define SSD1306_CMD_SET_DISPLAY_OFFSET    0xD3
#define SSD1306_CMD_SET_DISPLAY_CLK_DIV   0xD5
#define SSD1306_CMD_SET_PRECHARGE_PERIOD  0xD9
#define SSD1306_CMD_SET_COM_PINS          0xDA
#define SSD1306_CMD_SET_VCOMH_DESELECT    0xDB
#define SSD1306_CMD_NOP                   0xE3
#define SSD1306_CMD_SET_CHARGE_PUMP       0x8D
#define SSD1306_CMD_SET_MEMORY_ADDR_MODE  0x20
#define SSD1306_CMD_SET_COLUMN_ADDR       0x21
#define SSD1306_CMD_SET_PAGE_ADDR         0x22
#define SSD1306_CMD_DEACTIVATE_SCROLL     0x2E
#define SSD1306_CMD_SET_SEGMENT_REMAP     0xA1
#define SSD1306_CMD_SET_COM_SCAN_DIR      0xC8

static driver_status_t ssd1306_write_cmd(ssd1306_dev_t *p_dev, uint8_t cmd)
{
    if (p_dev == NULL || p_dev->p_i2c_ops == NULL || p_dev->p_i2c_ops->write_reg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    
    return p_dev->p_i2c_ops->write_reg(p_dev->p_bus_handle, p_dev->dev_addr, 0x00, &cmd, 1);
}

static driver_status_t ssd1306_write_data(ssd1306_dev_t *p_dev, const uint8_t *p_data, uint32_t len)
{
    if (p_dev == NULL || p_dev->p_i2c_ops == NULL || p_dev->p_i2c_ops->write_reg == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    
    return p_dev->p_i2c_ops->write_reg(p_dev->p_bus_handle, p_dev->dev_addr, 0x40, p_data, len);
}

static driver_status_t ssd1306_set_cursor(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y)
{
    driver_status_t status;
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_COLUMN_ADDR);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, x);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_WIDTH - 1);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_PAGE_ADDR);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, y);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_PAGE_COUNT - 1);
    if (status != DRV_OK) {
        return status;
    }
    
    return DRV_OK;
}

static bool ssd1306_check_params(ssd1306_dev_t *p_dev)
{
    if (p_dev == NULL) {
        return false;
    }
    
    if (p_dev->p_i2c_ops == NULL || p_dev->p_i2c_ops->write_reg == NULL) {
        return false;
    }
    
    if (p_dev->p_time_ops == NULL) {
        return false;
    }
    
    return true;
}

driver_status_t ssd1306_init(ssd1306_dev_t *p_dev, 
                             const driver_i2c_ops_t *p_i2c_ops,
                             const driver_time_ops_t *p_time_ops,
                             void *p_bus_handle,
                             uint8_t dev_addr)
{
    driver_status_t status;
    
    if (p_dev == NULL || p_i2c_ops == NULL || p_time_ops == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    
    memset(p_dev, 0, sizeof(ssd1306_dev_t));
    
    p_dev->p_i2c_ops = p_i2c_ops;
    p_dev->p_time_ops = p_time_ops;
    p_dev->p_bus_handle = p_bus_handle;
    p_dev->dev_addr = dev_addr;
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_DISPLAY_OFF);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_DISPLAY_CLK_DIV);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0x80);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_MULTIPLEX_RATIO);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_HEIGHT - 1);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_DISPLAY_OFFSET);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0x00);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_SEGMENT_REMAP | 0x01);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_COM_SCAN_DIR);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_COM_PINS);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0x12);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_CONTRAST);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0xCF);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_PRECHARGE_PERIOD);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0xF1);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_VCOMH_DESELECT);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0x40);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_DISPLAY_ON_RESUME);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_NORMAL_DISPLAY);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_DEACTIVATE_SCROLL);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_MEMORY_ADDR_MODE);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0x00);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, SSD1306_CMD_SET_CHARGE_PUMP);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_write_cmd(p_dev, 0x14);
    if (status != DRV_OK) {
        return status;
    }
    
    if (p_dev->p_time_ops->delay_ms != NULL) {
        p_dev->p_time_ops->delay_ms(100);
    }
    
    status = ssd1306_clear(p_dev);
    if (status != DRV_OK) {
        return status;
    }
    
    status = ssd1306_display_on(p_dev);
    if (status != DRV_OK) {
        return status;
    }
    
    p_dev->is_initialized = true;
    
    return DRV_OK;
}

driver_status_t ssd1306_deinit(ssd1306_dev_t *p_dev)
{
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    p_dev->is_initialized = false;
    
    return ssd1306_display_off(p_dev);
}

driver_status_t ssd1306_clear(ssd1306_dev_t *p_dev)
{
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    memset(p_dev->display_buf, 0, sizeof(p_dev->display_buf));
    
    return DRV_OK;
}

driver_status_t ssd1306_update_display(ssd1306_dev_t *p_dev)
{
    driver_status_t status;
    uint8_t page;
    
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    for (page = 0; page < SSD1306_PAGE_COUNT; page++) {
        status = ssd1306_set_cursor(p_dev, 0, page);
        if (status != DRV_OK) {
            return status;
        }
        
        status = ssd1306_write_data(p_dev, p_dev->display_buf[page], SSD1306_WIDTH);
        if (status != DRV_OK) {
            return status;
        }
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_set_pixel(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, ssd1306_color_t color)
{
    uint8_t page;
    uint8_t bit;
    
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        return DRV_ERR_INVALID_VAL;
    }
    
    page = y / 8;
    bit = y % 8;
    
    if (color == SSD1306_COLOR_WHITE) {
        p_dev->display_buf[page][x] |= (1 << bit);
    } else {
        p_dev->display_buf[page][x] &= ~(1 << bit);
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_draw_line(ssd1306_dev_t *p_dev, ssd1306_line_t *p_line, ssd1306_color_t color)
{
    int16_t dx, dy, sx, sy, err, e2;
    uint8_t x0, y0, x1, y1;
    driver_status_t status;
    
    if (!ssd1306_check_params(p_dev) || p_line == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    
    x0 = p_line->start.x;
    y0 = p_line->start.y;
    x1 = p_line->end.x;
    y1 = p_line->end.y;
    
    if (x0 >= SSD1306_WIDTH || y0 >= SSD1306_HEIGHT || x1 >= SSD1306_WIDTH || y1 >= SSD1306_HEIGHT) {
        return DRV_ERR_INVALID_VAL;
    }
    
    dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);
    dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);
    
    sx = (x0 < x1) ? 1 : -1;
    sy = (y0 < y1) ? 1 : -1;
    
    err = dx - dy;
    
    while (true) {
        status = ssd1306_set_pixel(p_dev, x0, y0, color);
        if (status != DRV_OK) {
            return status;
        }
        
        if (x0 == x1 && y0 == y1) {
            break;
        }
        
        e2 = 2 * err;
        
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_draw_rect(ssd1306_dev_t *p_dev, ssd1306_rect_t *p_rect, ssd1306_color_t color)
{
    ssd1306_line_t line;
    driver_status_t status;
    uint8_t x, y;
    
    if (!ssd1306_check_params(p_dev) || p_rect == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (p_rect->pos.x >= SSD1306_WIDTH || p_rect->pos.y >= SSD1306_HEIGHT) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (p_rect->pos.x + p_rect->width > SSD1306_WIDTH || p_rect->pos.y + p_rect->height > SSD1306_HEIGHT) {
        return DRV_ERR_INVALID_VAL;
    }
    
    line.start.x = p_rect->pos.x;
    line.start.y = p_rect->pos.y;
    line.end.x = p_rect->pos.x + p_rect->width - 1;
    line.end.y = p_rect->pos.y;
    status = ssd1306_draw_line(p_dev, &line, color);
    if (status != DRV_OK) {
        return status;
    }
    
    line.start.x = p_rect->pos.x + p_rect->width - 1;
    line.start.y = p_rect->pos.y;
    line.end.x = p_rect->pos.x + p_rect->width - 1;
    line.end.y = p_rect->pos.y + p_rect->height - 1;
    status = ssd1306_draw_line(p_dev, &line, color);
    if (status != DRV_OK) {
        return status;
    }
    
    line.start.x = p_rect->pos.x;
    line.start.y = p_rect->pos.y + p_rect->height - 1;
    line.end.x = p_rect->pos.x + p_rect->width - 1;
    line.end.y = p_rect->pos.y + p_rect->height - 1;
    status = ssd1306_draw_line(p_dev, &line, color);
    if (status != DRV_OK) {
        return status;
    }
    
    line.start.x = p_rect->pos.x;
    line.start.y = p_rect->pos.y;
    line.end.x = p_rect->pos.x;
    line.end.y = p_rect->pos.y + p_rect->height - 1;
    status = ssd1306_draw_line(p_dev, &line, color);
    if (status != DRV_OK) {
        return status;
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_fill_rect(ssd1306_dev_t *p_dev, ssd1306_rect_t *p_rect, ssd1306_color_t color)
{
    uint8_t x, y;
    driver_status_t status;
    
    if (!ssd1306_check_params(p_dev) || p_rect == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (p_rect->pos.x >= SSD1306_WIDTH || p_rect->pos.y >= SSD1306_HEIGHT) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (p_rect->pos.x + p_rect->width > SSD1306_WIDTH || p_rect->pos.y + p_rect->height > SSD1306_HEIGHT) {
        return DRV_ERR_INVALID_VAL;
    }
    
    for (y = p_rect->pos.y; y < p_rect->pos.y + p_rect->height; y++) {
        for (x = p_rect->pos.x; x < p_rect->pos.x + p_rect->width; x++) {
            status = ssd1306_set_pixel(p_dev, x, y, color);
            if (status != DRV_OK) {
                return status;
            }
        }
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_draw_char(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, char ch, ssd1306_font_size_t font_size, ssd1306_color_t color)
{
    uint8_t i, j;
    uint8_t char_index;
    driver_status_t status;
    
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (font_size == SSD1306_FONT_SIZE_6x8) {
        if (x + 6 > SSD1306_WIDTH || y + 8 > SSD1306_HEIGHT) {
            return DRV_ERR_INVALID_VAL;
        }
        
        if (ch < ' ' || ch > '~') {
            ch = ' ';
        }
        
        char_index = ch - ' ';
        
        for (i = 0; i < 6; i++) {
            for (j = 0; j < 8; j++) {
                if (g_ssd1306_font_8x16[char_index][i] & (1 << j)) {
                    status = ssd1306_set_pixel(p_dev, x + i, y + j, color);
                    if (status != DRV_OK) {
                        return status;
                    }
                }
            }
        }
    } else if (font_size == SSD1306_FONT_SIZE_8x16) {
        if (x + 8 > SSD1306_WIDTH || y + 16 > SSD1306_HEIGHT) {
            return DRV_ERR_INVALID_VAL;
        }
        
        if (ch < ' ' || ch > '~') {
            ch = ' ';
        }
        
        char_index = ch - ' ';
        
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                if (g_ssd1306_font_8x16[char_index][i] & (1 << j)) {
                    status = ssd1306_set_pixel(p_dev, x + i, y + j, color);
                    if (status != DRV_OK) {
                        return status;
                    }
                }
            }
        }
        
        for (i = 0; i < 8; i++) {
            for (j = 0; j < 8; j++) {
                if (g_ssd1306_font_8x16[char_index][i + 8] & (1 << j)) {
                    status = ssd1306_set_pixel(p_dev, x + i, y + j + 8, color);
                    if (status != DRV_OK) {
                        return status;
                    }
                }
            }
        }
    } else {
        return DRV_ERR_INVALID_VAL;
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_draw_string(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, const char *p_str, ssd1306_font_size_t font_size, ssd1306_color_t color)
{
    uint8_t char_width;
    uint8_t char_height;
    uint8_t pos_x;
    uint8_t pos_y;
    driver_status_t status;
    
    if (!ssd1306_check_params(p_dev) || p_str == NULL) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (font_size == SSD1306_FONT_SIZE_6x8) {
        char_width = 6;
        char_height = 8;
    } else if (font_size == SSD1306_FONT_SIZE_8x16) {
        char_width = 8;
        char_height = 16;
    } else {
        return DRV_ERR_INVALID_VAL;
    }
    
    pos_x = x;
    pos_y = y;
    
    while (*p_str != '\0') {
        if (*p_str == '\n') {
            pos_x = x;
            pos_y += char_height;
            p_str++;
            continue;
        }
        
        if (pos_x + char_width > SSD1306_WIDTH) {
            pos_x = x;
            pos_y += char_height;
        }
        
        if (pos_y + char_height > SSD1306_HEIGHT) {
            break;
        }
        
        status = ssd1306_draw_char(p_dev, pos_x, pos_y, *p_str, font_size, color);
        if (status != DRV_OK) {
            return status;
        }
        
        pos_x += char_width;
        p_str++;
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_draw_uint(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, uint32_t num, ssd1306_font_size_t font_size, ssd1306_color_t color)
{
    char str[16];
    uint8_t i;
    uint8_t len;
    uint32_t temp;
    driver_status_t status;
    
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
    } else {
        temp = num;
        len = 0;
        
        while (temp > 0) {
            temp /= 10;
            len++;
        }
        
        for (i = 0; i < len; i++) {
            str[len - 1 - i] = '0' + (num % 10);
            num /= 10;
        }
        
        str[len] = '\0';
    }
    
    status = ssd1306_draw_string(p_dev, x, y, str, font_size, color);
    if (status != DRV_OK) {
        return status;
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_draw_int(ssd1306_dev_t *p_dev, uint8_t x, uint8_t y, int32_t num, ssd1306_font_size_t font_size, ssd1306_color_t color)
{
    char str[16];
    uint8_t i;
    uint8_t len;
    int32_t temp;
    bool is_negative;
    driver_status_t status;
    
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    is_negative = false;
    
    if (num < 0) {
        is_negative = true;
        num = -num;
    }
    
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
    } else {
        temp = num;
        len = 0;
        
        while (temp > 0) {
            temp /= 10;
            len++;
        }
        
        if (is_negative) {
            str[0] = '-';
            for (i = 0; i < len; i++) {
                str[len - i] = '0' + (num % 10);
                num /= 10;
            }
            str[len + 1] = '\0';
        } else {
            for (i = 0; i < len; i++) {
                str[len - 1 - i] = '0' + (num % 10);
                num /= 10;
            }
            str[len] = '\0';
        }
    }
    
    status = ssd1306_draw_string(p_dev, x, y, str, font_size, color);
    if (status != DRV_OK) {
        return status;
    }
    
    return DRV_OK;
}

driver_status_t ssd1306_display_on(ssd1306_dev_t *p_dev)
{
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    return ssd1306_write_cmd(p_dev, SSD1306_CMD_DISPLAY_ON);
}

driver_status_t ssd1306_display_off(ssd1306_dev_t *p_dev)
{
    if (!ssd1306_check_params(p_dev)) {
        return DRV_ERR_INVALID_VAL;
    }
    
    return ssd1306_write_cmd(p_dev, SSD1306_CMD_DISPLAY_OFF);
}
