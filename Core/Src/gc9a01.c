/*
 * gc9a01.c
 *
 *  Created on: Mar 31, 2026
 *      Author: ASUS
 */


#include "gc9a01.h"

extern SPI_HandleTypeDef hspi1;

// ==== DEFINE CHÂN ====
#define LCD_CS_LOW()   HAL_GPIO_WritePin(GPIOA, CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_HIGH()  HAL_GPIO_WritePin(GPIOA, CS_Pin, GPIO_PIN_SET)

#define LCD_DC_CMD()   HAL_GPIO_WritePin(GPIOB, DC_Pin, GPIO_PIN_RESET)
#define LCD_DC_DATA()  HAL_GPIO_WritePin(GPIOB, DC_Pin, GPIO_PIN_SET)

// ==== GỬI DATA ====
void LCD_WriteCommand(uint8_t cmd)
{
    LCD_DC_CMD();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, 10);
    LCD_CS_HIGH();
}

void LCD_WriteData(uint8_t *data, uint16_t size)
{
    LCD_DC_DATA();
    LCD_CS_LOW();
    HAL_SPI_Transmit(&hspi1, data, size, 100); // Gửi cả mảng mà không ngắt CS
    LCD_CS_HIGH();
}

// ==== INIT LCD ====
void GC9A01_Init(void)
{
    HAL_Delay(100);

    LCD_WriteCommand(0x11); // sleep out
    HAL_Delay(120);

    LCD_WriteCommand(0x36);
    uint8_t data = 0x48;
    LCD_WriteData(&data, 1);

    LCD_WriteCommand(0x3A);
    data = 0x05;
    LCD_WriteData(&data, 1);

    LCD_WriteCommand(0x29); // display on
}

// ==== SET WINDOW ====
void LCD_SetAddr(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4];

    // Cấu hình Column Address
    LCD_WriteCommand(0x2A);
    data[0] = x1 >> 8; data[1] = x1 & 0xFF;
    data[2] = x2 >> 8; data[3] = x2 & 0xFF;
    LCD_WriteData(data, 4); // CS giữ thấp suốt 4 byte

    // Cấu hình Row Address
    LCD_WriteCommand(0x2B);
    data[0] = y1 >> 8; data[1] = y1 & 0xFF;
    data[2] = y2 >> 8; data[3] = y2 & 0xFF;
    LCD_WriteData(data, 4); // CS giữ thấp suốt 4 byte

    // Chuẩn bị viết vào bộ nhớ RAM
    LCD_WriteCommand(0x2C);
}

// ==== FILL MÀU ====
void LCD_FillColor(uint16_t color)
{
    uint8_t color_buf[2] = {color >> 8, color & 0xFF};

    LCD_SetAddr(0, 0, 239, 239);

    LCD_DC_DATA();
    LCD_CS_LOW();

    // Để nhanh hơn, ta gửi theo dòng thay vì gửi từng pixel đơn lẻ
    // Một dòng có 240 pixel * 2 byte = 480 bytes
    uint8_t line_buffer[480];
    for(int x = 0; x < 240; x++) {
        line_buffer[2*x] = color_buf[0];
        line_buffer[2*x+1] = color_buf[1];
    }

    // Gửi 240 dòng
    for(int y = 0; y < 240; y++) {
        HAL_SPI_Transmit(&hspi1, line_buffer, 480, 100);
    }

    LCD_CS_HIGH();
}
