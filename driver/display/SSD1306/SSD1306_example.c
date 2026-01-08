#include "ssd1306_driver.h"
#include <stdio.h>

static driver_status_t hal_i2c_write_reg(void *ctx, uint8_t dev_addr, uint8_t reg_addr, const uint8_t *p_data, uint32_t len)
{
    printf("I2C Write: addr=0x%02X, reg=0x%02X, len=%d\n", dev_addr, reg_addr, len);
    return DRV_OK;
}

static driver_status_t hal_i2c_read_reg(void *ctx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *p_data, uint32_t len)
{
    printf("I2C Read: addr=0x%02X, reg=0x%02X, len=%d\n", dev_addr, reg_addr, len);
    return DRV_OK;
}

static void hal_delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    for (i = 0; i < ms * 1000; i++) {
        __NOP();
    }
}

static const driver_i2c_ops_t g_hal_i2c_ops = {
    .write_reg = hal_i2c_write_reg,
    .read_reg = hal_i2c_read_reg,
    .reserved = NULL
};

static const driver_time_ops_t g_hal_time_ops = {
    .delay_ms = hal_delay_ms,
    .delay_us = NULL,
    .get_tick = NULL,
    .reserved = NULL
};

static ssd1306_dev_t g_ssd1306_dev;

void ssd1306_example_basic(void)
{
    driver_status_t status;
    ssd1306_line_t line;
    ssd1306_rect_t rect;
    
    printf("SSD1306 Basic Example\n");
    
    status = ssd1306_init(&g_ssd1306_dev, &g_hal_i2c_ops, &g_hal_time_ops, NULL, SSD1306_ADDR_LOW);
    if (status != DRV_OK) {
        printf("SSD1306 init failed: %d\n", status);
        return;
    }
    printf("SSD1306 init success\n");
    
    status = ssd1306_clear(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 clear failed: %d\n", status);
        return;
    }
    printf("SSD1306 clear success\n");
    
    status = ssd1306_draw_string(&g_ssd1306_dev, 0, 0, "Hello World!", SSD1306_FONT_SIZE_8x16, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 draw string failed: %d\n", status);
        return;
    }
    printf("SSD1306 draw string success\n");
    
    status = ssd1306_update_display(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 update display failed: %d\n", status);
        return;
    }
    printf("SSD1306 update display success\n");
    
    hal_delay_ms(2000);
    
    status = ssd1306_clear(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 clear failed: %d\n", status);
        return;
    }
    
    status = ssd1306_draw_line(&g_ssd1306_dev, &(ssd1306_line_t){{0, 0}, {127, 63}}, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 draw line failed: %d\n", status);
        return;
    }
    printf("SSD1306 draw line success\n");
    
    status = ssd1306_draw_rect(&g_ssd1306_dev, &(ssd1306_rect_t){{10, 10}, 50, 30}, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 draw rect failed: %d\n", status);
        return;
    }
    printf("SSD1306 draw rect success\n");
    
    status = ssd1306_fill_rect(&g_ssd1306_dev, &(ssd1306_rect_t){{70, 20}, 40, 30}, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 fill rect failed: %d\n", status);
        return;
    }
    printf("SSD1306 fill rect success\n");
    
    status = ssd1306_update_display(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 update display failed: %d\n", status);
        return;
    }
    
    hal_delay_ms(2000);
    
    status = ssd1306_clear(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 clear failed: %d\n", status);
        return;
    }
    
    status = ssd1306_draw_uint(&g_ssd1306_dev, 0, 0, 12345, SSD1306_FONT_SIZE_8x16, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 draw uint failed: %d\n", status);
        return;
    }
    printf("SSD1306 draw uint success\n");
    
    status = ssd1306_draw_int(&g_ssd1306_dev, 0, 20, -6789, SSD1306_FONT_SIZE_8x16, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 draw int failed: %d\n", status);
        return;
    }
    printf("SSD1306 draw int success\n");
    
    status = ssd1306_update_display(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 update display failed: %d\n", status);
        return;
    }
    
    hal_delay_ms(2000);
    
    status = ssd1306_display_off(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 display off failed: %d\n", status);
        return;
    }
    printf("SSD1306 display off success\n");
    
    hal_delay_ms(1000);
    
    status = ssd1306_display_on(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 display on failed: %d\n", status);
        return;
    }
    printf("SSD1306 display on success\n");
    
    status = ssd1306_deinit(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 deinit failed: %d\n", status);
        return;
    }
    printf("SSD1306 deinit success\n");
    
    printf("SSD1306 Basic Example Complete\n");
}

void ssd1306_example_advanced(void)
{
    driver_status_t status;
    uint8_t x, y;
    
    printf("SSD1306 Advanced Example\n");
    
    status = ssd1306_init(&g_ssd1306_dev, &g_hal_i2c_ops, &g_hal_time_ops, NULL, SSD1306_ADDR_LOW);
    if (status != DRV_OK) {
        printf("SSD1306 init failed: %d\n", status);
        return;
    }
    
    status = ssd1306_clear(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 clear failed: %d\n", status);
        return;
    }
    
    for (y = 0; y < SSD1306_HEIGHT; y++) {
        for (x = 0; x < SSD1306_WIDTH; x++) {
            status = ssd1306_set_pixel(&g_ssd1306_dev, x, y, SSD1306_COLOR_WHITE);
            if (status != DRV_OK) {
                printf("SSD1306 set pixel failed: %d\n", status);
                return;
            }
        }
    }
    printf("SSD1306 fill screen success\n");
    
    status = ssd1306_update_display(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 update display failed: %d\n", status);
        return;
    }
    
    hal_delay_ms(1000);
    
    status = ssd1306_clear(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 clear failed: %d\n", status);
        return;
    }
    
    status = ssd1306_draw_string(&g_ssd1306_dev, 0, 0, "SSD1306\nDriver\nLibrary", SSD1306_FONT_SIZE_8x16, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 draw string failed: %d\n", status);
        return;
    }
    
    status = ssd1306_draw_string(&g_ssd1306_dev, 0, 48, "Test Complete!", SSD1306_FONT_SIZE_6x8, SSD1306_COLOR_WHITE);
    if (status != DRV_OK) {
        printf("SSD1306 draw string failed: %d\n", status);
        return;
    }
    
    status = ssd1306_update_display(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 update display failed: %d\n", status);
        return;
    }
    
    hal_delay_ms(2000);
    
    status = ssd1306_deinit(&g_ssd1306_dev);
    if (status != DRV_OK) {
        printf("SSD1306 deinit failed: %d\n", status);
        return;
    }
    
    printf("SSD1306 Advanced Example Complete\n");
}

int main(void)
{
    printf("SSD1306 Driver Example Start\n");
    
    ssd1306_example_basic();
    
    hal_delay_ms(1000);
    
    ssd1306_example_advanced();
    
    printf("SSD1306 Driver Example End\n");
    
    return 0;
}
