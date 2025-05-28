#include "display.h"

ssd1306_t display;

void init_barr_i2c()
{
    i2c_init(PORT_I2C, 400 * 1000);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
}

void init_display()
{
    ssd1306_init(&display, 128, 64, 0x3C, PORT_I2C);
}

void draw_display(uint32_t x_text, uint32_t y_text, uint32_t scale_text, const char* text)
{
    ssd1306_draw_string(&display, x_text, y_text, scale_text, text);
}

void show_display()
{
    ssd1306_show(&display);
}

void clear_display()
{
    ssd1306_clear(&display);
}