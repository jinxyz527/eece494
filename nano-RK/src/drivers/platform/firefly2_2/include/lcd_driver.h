/******************************************************************************
*
*  Filename: lcd_driver.h
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
*  Escape Characters:
*  '\n' - New line
*  '\a' - Autocycling
*  '\b' - Buzz
*  '\d' - Degree symbol
*  '\l' - Left arrow
*  '\r' - Right arrow
*  '\X' - Set LED 1
*  '\Y' - Set LED 2
*  '\Z' - Set LED 3
*  '\x' - Clear LED 1
*  '\y' - Clear LED 2
*  '\z' - Clear LED 3
*
*******************************************************************************/

#ifndef _LCD_DRIVER_H_
#define _LCD_DRIVER_H_

// Escape Characters
#define LCD_AUTOCYCLING_TIME 2500000 // Time in microseconds to spend on each screen when autocycling
#define LCD_BUZZ_TIME 100 // Number of 250 microsecond buzzes to cycle upon receiving escape character
#ifndef LCD_BIG
  #define LCD_CHAR_DEGREE 223
  #define LCD_CHAR_ARROW_LEFT 127
  #define LCD_CHAR_ARROW_RIGHT 126
#else
  #define LCD_CHAR_DEGREE 178
  #define LCD_CHAR_ARROW_LEFT 200
  #define LCD_CHAR_ARROW_RIGHT 199
#endif

// Functions
void lcd_control_pins_set(uint8_t rs, uint8_t rw);
void lcd_enable_cycle();
void lcd_data_pins_set(uint8_t d7, uint8_t d6, uint8_t d5, uint8_t d4, uint8_t d3, uint8_t d2, uint8_t d1, uint8_t d0);

void lcd_initialize();
void lcd_function_set_4bit();
void lcd_function_set();

void lcd_display_on();
void lcd_display_off();

void lcd_display_clear();
void lcd_entry_mode_set();

void lcd_line_set(uint8_t line);
void lcd_char_send(char c);

void lcd_string_display_line(char* s, uint8_t line);
void lcd_line_clear(uint8_t line);

void lcd_string_display_wrap(char* s);
void lcd_string_display_escape(char* s);

void lcd_string_array_load(char* s);
void lcd_string_array_load_helper(char* s);
void lcd_string_array_add(char* s);

void lcd_string_array_cycle();
void lcd_string_array_cycle_reverse();
uint8_t lcd_string_array_autocycling_get();

#endif
