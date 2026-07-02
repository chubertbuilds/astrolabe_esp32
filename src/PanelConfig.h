#pragma once

#include "esp_lcd_panel_rgb.h"
#include "esp_lcd_panel_ops.h"
#include "esp_check.h"

esp_lcd_rgb_panel_config_t panel_config_single = {
    .clk_src = LCD_CLK_SRC_DEFAULT,
    .timings = {
        .pclk_hz = 8400000L,
        .h_res = 720,
        .v_res = 720,
        .hsync_pulse_width = 2,
        .hsync_back_porch = 44,
        .hsync_front_porch = 46,
        .vsync_pulse_width = 16,
        .vsync_back_porch = 16,
        .vsync_front_porch = 50,
        .flags = {
            .hsync_idle_low =  0,
            .vsync_idle_low = 0,
            .de_idle_high = 0,
            .pclk_active_neg = 0,
            .pclk_idle_high = 0,
        },
    },
    .data_width = 16, // RGB565 in parallel mode, thus 16 bits in width
    .bits_per_pixel = 0,
    .num_fbs = 1,
    .bounce_buffer_size_px = 0,
    .sram_trans_align = 8,
    .psram_trans_align = 64,
    .hsync_gpio_num = TFT_HSYNC,
    .vsync_gpio_num = TFT_VSYNC,
    .de_gpio_num = TFT_DE,
    .pclk_gpio_num = TFT_PCLK,
    .disp_gpio_num = GPIO_NUM_NC,
    .data_gpio_nums = {TFT_B1,TFT_B2,TFT_B3,TFT_B4,TFT_B5,TFT_G0,TFT_G1,TFT_G2,TFT_G3,TFT_G4,TFT_G5,TFT_R1,TFT_R2,TFT_R3,TFT_R4,TFT_R5
    },
    // The timing parameters should refer to your LCD spec

    .flags = {.fb_in_psram = true}, // allocate frame buffer from PSRAM
};