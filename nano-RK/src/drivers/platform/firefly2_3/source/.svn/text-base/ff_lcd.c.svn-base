/******************************************************************************
*
*  Filename: ff_lcd.c
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
*******************************************************************************/

#include <nrk.h>
#include <ff_lcd.h>
#include <lcd_driver.h>

// Switches
#define LCD_SW1 NRK_DEBUG_0
#define LCD_SW2 NRK_DEBUG_1

// Buzzer
#define LCD_BUZZER NRK_DEBUG_2

// LEDs
#define LCD_LED1 NRK_DEBUG_3
#define LCD_LED2 NRK_UART0_RXD
#define LCD_LED3 NRK_UART0_TXD

nrk_time_t lcd_wait;
uint8_t lcd_switch_last_state[2];
uint8_t lcd_led_state[3];

void lcd_wait_us(uint32_t micro_secs)
{
  lcd_wait.secs = 0;
  lcd_wait.nano_secs = micro_secs * 1000;
  nrk_wait(lcd_wait);
}

uint8_t lcd_pin_get(uint8_t pin)
{
  return nrk_gpio_get(pin);
}

void lcd_pin_set(uint8_t pin, uint8_t value)
{
  if(value > 0)
    nrk_gpio_set(pin);
  else
    nrk_gpio_clr(pin);
}

void lcd_setup()
{
  // Initialize variables
  lcd_wait.secs = lcd_wait.nano_secs = 0;
  lcd_switch_last_state[0] = lcd_switch_last_state[1] = 0;
  lcd_led_state[0] = lcd_led_state[1] = lcd_led_state[2] = 0;

  // Set switch pins to input
  nrk_gpio_direction(LCD_SW1, NRK_PIN_INPUT);
  nrk_gpio_direction(LCD_SW2, NRK_PIN_INPUT);

  // Set buzzer pin to output
  nrk_gpio_direction(LCD_BUZZER, NRK_PIN_OUTPUT);

  // Set LED pins to output
  nrk_gpio_direction(LCD_LED1, NRK_PIN_OUTPUT);
  nrk_gpio_direction(LCD_LED2, NRK_PIN_OUTPUT);
  nrk_gpio_direction(LCD_LED3, NRK_PIN_OUTPUT);

  // Initialize screen
  lcd_initialize();

  // Clear LEDs
  lcd_led_set(1, 0);
  lcd_led_set(2, 0);
  lcd_led_set(3, 0);
}

uint8_t lcd_switch_get(uint8_t sw)
{
  uint8_t v = 0;

  switch(sw)
  {
    case 1:
      v = lcd_pin_get(LCD_SW1);
      break;
    case 2:
      v = lcd_pin_get(LCD_SW2);
      break;
  }

  return v;
}

uint8_t lcd_switch_pressed(uint8_t sw)
{
  uint8_t v = 0;

  if(sw != 1 && sw != 2)
    return v;

  if(lcd_switch_get(sw) > lcd_switch_last_state[sw - 1])
    v = 1;

  lcd_switch_last_state[sw - 1] = lcd_switch_get(sw);

  return v;
}

uint8_t lcd_switch_released(uint8_t sw)
{
  uint8_t v = 0;

  if(sw != 1 && sw != 2)
    return v;

  if(lcd_switch_get(sw) < lcd_switch_last_state[sw - 1])
    v = 1;

  lcd_switch_last_state[sw - 1] = lcd_switch_get(sw);

  return v;
}

void lcd_buzz(uint32_t n)
{
  uint32_t i;
  
  for(i = 0; i < n; i++)
  {
    lcd_pin_set(LCD_BUZZER, 0);
    lcd_wait_us(125);
    lcd_pin_set(LCD_BUZZER, 1);
    lcd_wait_us(125);
  }
}

uint8_t lcd_led_get(uint8_t led)
{
  if(led != 1 && led != 2 && led != 3)
    return 0;

  return lcd_led_state[led - 1];
}

void lcd_led_set(uint8_t led, uint8_t value)
{
  if(led != 1 && led != 2 && led != 3)
    return;

  switch(led)
  {
    case 1:
      lcd_pin_set(LCD_LED1, value);
      break;
    case 2:
      lcd_pin_set(LCD_LED2, value);
      break;
    case 3:
      lcd_pin_set(LCD_LED3, value);
      break;
  }

  lcd_led_state[led - 1] = value;
}

void lcd_led_toggle(uint8_t led)
{
  if(lcd_led_get(led) > 0)
    lcd_led_set(led, 0);
  else
    lcd_led_set(led, 1);
}
