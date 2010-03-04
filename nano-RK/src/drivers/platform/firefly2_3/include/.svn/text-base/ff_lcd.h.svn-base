/******************************************************************************
*
*  Filename: ff_lcd.h
*  Author: Vikram Rajkumar
*  Last Updated: 2009-07-27
*
*  This driver interfaces the FireFly 2.2 with the FireFly LCD v1.2 board.
*
*  The FireFly LCD v1.2 board includes a Sitronix ST7066U LCD controller using
*  a 4-bit data interface.
*
*  Function definitions and pin defines can be found in files 'ff_lcd.c' and
*  'lcd_driver.c'.
*
*******************************************************************************/

#ifndef _FF_LCD_H_
#define _FF_LCD_H_

// Screen
#define LCD_BIG // Comment out if using 8x2 screen or uncomment if using 16x2 screen
#ifndef LCD_BIG
  #define LCD_MAX_CHARS_PER_LINE 8  // Line size of small display
#else
  #define LCD_MAX_CHARS_PER_LINE 16 // Line size of big display
#endif

// Functions
void lcd_wait_us(uint32_t micro_secs);

uint8_t lcd_pin_get(uint8_t pin);
void lcd_pin_set(uint8_t pin, uint8_t value);

void lcd_setup();

uint8_t lcd_switch_get(uint8_t sw);
uint8_t lcd_switch_pressed(uint8_t sw);
uint8_t lcd_switch_released(uint8_t sw);

void lcd_buzz(uint32_t n);

uint8_t lcd_led_get(uint8_t led);
void lcd_led_set(uint8_t led, uint8_t value);
void lcd_led_toggle(uint8_t led);

#endif
