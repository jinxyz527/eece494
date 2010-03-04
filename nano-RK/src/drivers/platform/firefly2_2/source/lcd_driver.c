/******************************************************************************
*
*  Filename: lcd_driver.c
*  Author: Vikram Rajkumar
*  Last Updated: 2009-07-27
*
*  This driver interfaces the FireFly 2.2 with the FireFly LCD v1.2 board.
*
*  The FireFly LCD v1.2 board includes a Sitronix ST7066U LCD controller using
*  a 4-bit data interface.
*
*  Function prototypes and hardware settings can be found in files 'ff_lcd.h'
*  and 'lcd_driver.h'.
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

#include <nrk.h>
#include <ff_lcd.h>
#include <lcd_driver.h>

// Control Pins
#define LCD_RS NRK_ADC_INPUT_1 // Register Select
#define LCD_RW NRK_ADC_INPUT_2 // Read/Write
#define LCD_EN NRK_ADC_INPUT_3 // Enable

// Data Pins
#define LCD_DB4 NRK_ADC_INPUT_4
#define LCD_DB5 NRK_ADC_INPUT_5
#define LCD_DB6 NRK_ADC_INPUT_6
#define LCD_DB7 NRK_ADC_INPUT_7

char** lcd_string_array;
uint8_t lcd_string_array_size;
uint8_t lcd_string_array_current_position;
uint8_t lcd_string_array_cycled;
uint8_t lcd_string_array_autocycling;

void lcd_control_pins_set(uint8_t rs, uint8_t rw)
{
  lcd_pin_set(LCD_RS, rs);
  lcd_pin_set(LCD_RW, rw);
}

void lcd_enable_cycle()
{
  lcd_pin_set(LCD_EN, 1);
  lcd_pin_set(LCD_EN, 0);
}

void lcd_data_pins_set(uint8_t d7, uint8_t d6, uint8_t d5, uint8_t d4, uint8_t d3, uint8_t d2, uint8_t d1, uint8_t d0)
{
  lcd_pin_set(LCD_DB7, d7);
  lcd_pin_set(LCD_DB6, d6);
  lcd_pin_set(LCD_DB5, d5);
  lcd_pin_set(LCD_DB4, d4);
  lcd_enable_cycle();

  lcd_pin_set(LCD_DB7, d3);
  lcd_pin_set(LCD_DB6, d2);
  lcd_pin_set(LCD_DB5, d1);
  lcd_pin_set(LCD_DB4, d0);
  lcd_enable_cycle();
}

void lcd_initialize()
{
  // Initialize variables
  lcd_string_array = NULL;
  lcd_string_array_size = lcd_string_array_current_position = 0;
  lcd_string_array_cycled = lcd_string_array_autocycling = 0;

  // Set control pins to output
  nrk_gpio_direction(LCD_RS, NRK_PIN_OUTPUT);
  nrk_gpio_direction(LCD_RW, NRK_PIN_OUTPUT);
  nrk_gpio_direction(LCD_EN, NRK_PIN_OUTPUT);

  // Set data pins to output
  nrk_gpio_direction(LCD_DB7, NRK_PIN_OUTPUT);
  nrk_gpio_direction(LCD_DB6, NRK_PIN_OUTPUT);
  nrk_gpio_direction(LCD_DB5, NRK_PIN_OUTPUT);
  nrk_gpio_direction(LCD_DB4, NRK_PIN_OUTPUT);

  lcd_wait_us(40000);

  lcd_function_set();
  lcd_function_set_4bit();
  lcd_function_set();
  lcd_function_set();
  lcd_display_on();
  lcd_display_clear();
  lcd_entry_mode_set();
}

void lcd_function_set_4bit()
{
  lcd_control_pins_set(0, 0);

  lcd_pin_set(LCD_DB7, 0);
  lcd_pin_set(LCD_DB6, 0);
  lcd_pin_set(LCD_DB5, 1);
  lcd_pin_set(LCD_DB4, 1);
  lcd_enable_cycle();

  lcd_wait_us(37);
}

// 4: 8 or 4 Bit Bus Mode
// 3: 2 Line Display Mode
// 2: 5 x 8 Format Display Mode
// 1: Don't Care
// 0: Don't Care
void lcd_function_set()
{
  lcd_control_pins_set(0, 0);
  lcd_data_pins_set(0, 0, 1, 0, 1, 0, 0, 0);
  lcd_wait_us(37);
}

// 2: Display On
// 1: Cursor Off
// 0: Cursor Blink Off
void lcd_display_on()
{
  lcd_control_pins_set(0, 0);
  lcd_data_pins_set(0, 0, 0, 0, 1, 1, 0, 0);
  lcd_wait_us(37);
}

// 2: Display Off
// 1: Cursor Off
// 0: Cursor Blink Off
void lcd_display_off()
{
  lcd_control_pins_set(0, 0);
  lcd_data_pins_set(0, 0, 0, 0, 1, 0, 0, 0);
  lcd_wait_us(37);
}

void lcd_display_clear()
{
  lcd_control_pins_set(0, 0);
  lcd_data_pins_set(0, 0, 0, 0, 0, 0, 0, 1);
  lcd_wait_us(1520);
}

// 1: Increment DDRAM Address
// 0: No Display Shift
void lcd_entry_mode_set()
{
  lcd_control_pins_set(0, 0);
  lcd_data_pins_set(0, 0, 0, 0, 0, 1, 1, 0);
  lcd_wait_us(37);
}

// Set DDRAM address counter to beginning of line 'line'
void lcd_line_set(uint8_t line)
{
  if(line != 1 && line != 2)
    return;

  lcd_control_pins_set(0, 0);
  lcd_data_pins_set(1, line - 1, 0, 0, 0, 0, 0, 0);
  lcd_wait_us(37);
}

void lcd_char_send(char c)
{
  lcd_control_pins_set(1, 0);
  lcd_data_pins_set(c & 128, c & 64, c & 32, c & 16, c & 8, c & 4, c & 2, c & 1);
  lcd_wait_us(37);
}

void lcd_string_display_line(char* s, uint8_t line)
{
  uint8_t i;

  lcd_line_set(line);

  for(i = 0; i < strlen(s); i++)
  {
    // Stop if line is full
    if(i >= LCD_MAX_CHARS_PER_LINE)
      return;

    lcd_char_send(s[i]);
  }

  // Clear remainder of line
  for(i = 0; i < LCD_MAX_CHARS_PER_LINE - strlen(s); i++)
    lcd_char_send(' ');
}

void lcd_line_clear(uint8_t line)
{
  lcd_string_display_line("", line);
}

void lcd_string_display_wrap(char* s)
{
  lcd_string_display_line(s, 1);

  if(strlen(s) > LCD_MAX_CHARS_PER_LINE)
    lcd_string_display_line(s + LCD_MAX_CHARS_PER_LINE, 2);
  else
    lcd_line_clear(2);
}

void lcd_string_display_escape(char* s)
{
  uint8_t i, j;
  uint8_t length = strlen(s); // String length
  uint8_t line = 1; // Current line
  uint8_t n = 0; // Number of characters on current line
  char c; // Character to send

  lcd_line_set(line);

  for(i = 0; i < length; i++)
  {
    c = s[i];

    // If escape character
    if(c == '\\')
    {
      // If end of string
      if(i + 1 >= length)
        break;

      switch(s[++i])
      {
        case 'n': // New line
          if(line < 2) // If not already on line 2
          {
            // Clear remainder of line
            for(j = 0; j < LCD_MAX_CHARS_PER_LINE - n; j++)
              lcd_char_send(' ');

            // Move to next line
            line = 2;
            lcd_line_set(line);

            // Reset line character count
            n = 0;
          }
          break;

        case 'a': // Autocycling
          lcd_string_array_autocycling = 1;
          break;

        case 'b': // Buzz
          if(!lcd_string_array_cycled)
            lcd_buzz(LCD_BUZZ_TIME);
          break;

        case 'd': // Degree symbol
          c = (char)LCD_CHAR_DEGREE;
          break;

        case 'l': // Left arrow
          c = (char)LCD_CHAR_ARROW_LEFT;
          break;

        case 'r': // Right arrow
          c = (char)LCD_CHAR_ARROW_RIGHT;
          break;

        case 'X': // Set LED 1
          lcd_led_set(1, 1);
          break;

        case 'Y': // Set LED 2
          lcd_led_set(2, 1);
          break;

        case 'Z': // Set LED 3
          lcd_led_set(3, 1);
          break;

        case 'x': // Clear LED 1
          lcd_led_set(1, 0);
          break;

        case 'y': // Clear LED 2
          lcd_led_set(2, 0);
          break;

        case 'z': // Clear LED 3
          lcd_led_set(3, 0);
          break;
      }
    }

    // If line is not full and character is valid
    if(n < LCD_MAX_CHARS_PER_LINE && c != '\\')
    {
      lcd_char_send(c);
      n++;
    }
  }

  // Clear remainder of current line
  for(i = 0; i < LCD_MAX_CHARS_PER_LINE - n; i++)
    lcd_char_send(' ');

  // If still on line 1 then clear line 2
  if(line < 2)
    lcd_line_clear(2);
}

void lcd_string_array_load(char* s)
{
  uint8_t i;

  // If array already populated then free
  if(lcd_string_array != NULL)
  {
    for(i = 0; i < lcd_string_array_size; i++)
      free(lcd_string_array[i]);

    free(lcd_string_array);
    lcd_string_array = NULL;
  }

  lcd_string_array_size = 0;
  lcd_string_array_load_helper(s);

  lcd_string_array_current_position = 0;
  lcd_string_array_cycled = 0;
  lcd_string_array_autocycling = 0;
  lcd_string_display_escape(lcd_string_array[lcd_string_array_current_position]);
}

void lcd_string_array_load_helper(char* s)
{
  uint8_t i;
  uint8_t length = strlen(s); // String length
  uint8_t line = 1; // Current line
  char* str; // String to add to array

  for(i = 0; i < length; i++)
  {
    // If end of string
    if(i + 1 >= length)
      break;

    // If escape character
    if(s[i] == '\\')
    {
      if(s[++i] == 'n') // If line break
      {
        line++;

        // If extra line
        if(line > 2)
        {
          // Build string
          str = malloc(i);
          strcpy(str, "");
          strncat(str, s, i - 1);

          // Add string to array
          lcd_string_array_add(str);

          // Recurse
          lcd_string_array_load_helper(s + i + 1);
          return;
        }
      }
    }
  }

  // Build string
  str = malloc(length + 1);
  strcpy(str, s);

  // Add string to array
  lcd_string_array_add(str);
}

void lcd_string_array_add(char* s)
{
  // Resize string array
  lcd_string_array_size++;
  lcd_string_array = realloc(lcd_string_array, lcd_string_array_size * sizeof(char*));

  // Add string to array
  lcd_string_array[lcd_string_array_size - 1] = s;
}

void lcd_string_array_cycle()
{
  if(lcd_string_array_size < 2 || lcd_string_array == NULL)
    return;

  if(lcd_string_array_current_position + 1 < lcd_string_array_size)
    lcd_string_array_current_position++;
  else
    lcd_string_array_current_position = 0;

  lcd_string_array_cycled = 1;
  lcd_string_display_escape(lcd_string_array[lcd_string_array_current_position]);
}

void lcd_string_array_cycle_reverse()
{
  if(lcd_string_array_size < 2 || lcd_string_array == NULL)
    return;

  if(lcd_string_array_current_position - 1 >= 0)
    lcd_string_array_current_position--;
  else
    lcd_string_array_current_position = lcd_string_array_size - 1;

  lcd_string_array_cycled = 1;
  lcd_string_display_escape(lcd_string_array[lcd_string_array_current_position]);
}

uint8_t lcd_string_array_autocycling_get()
{
  return lcd_string_array_autocycling;
}
