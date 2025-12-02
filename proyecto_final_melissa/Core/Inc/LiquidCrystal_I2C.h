#ifndef LIQUIDCRYSTAL_I2C_H
#define LIQUIDCRYSTAL_I2C_H

#include "stm32f4xx_hal.h"

// Define commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// Flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// Flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// Flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// Flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// Flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

// PCF8574 pin mapping (verify with your module!)
// Typical: P0=RS, P1=RW, P2=E, P3=BL, P4-P7=D4-D7
#define En 0x04  // Enable bit (P2)
#define Rw 0x02  // Read/Write bit (P1)
#define Rs 0x01  // Register select bit (P0)

// Struct to hold LCD information
typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t _Addr;
    uint8_t _displayfunction;
    uint8_t _displaycontrol;
    uint8_t _displaymode;
    uint8_t _numlines;
    uint8_t _cols;
    uint8_t _rows;
    uint8_t _backlightval;
} LiquidCrystal_I2C_t;

// Function prototypes
void LiquidCrystal_I2C_init(LiquidCrystal_I2C_t *lcd, I2C_HandleTypeDef *hi2c, 
                           uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows);
void LiquidCrystal_I2C_begin(LiquidCrystal_I2C_t *lcd, uint8_t cols, uint8_t rows, uint8_t charsize);
void LiquidCrystal_I2C_clear(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_home(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_noDisplay(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_display(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_noBlink(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_blink(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_noCursor(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_cursor(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_scrollDisplayLeft(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_scrollDisplayRight(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_leftToRight(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_rightToLeft(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_noBacklight(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_backlight(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_autoscroll(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_noAutoscroll(LiquidCrystal_I2C_t *lcd);
void LiquidCrystal_I2C_createChar(LiquidCrystal_I2C_t *lcd, uint8_t location, uint8_t charmap[]);
void LiquidCrystal_I2C_setCursor(LiquidCrystal_I2C_t *lcd, uint8_t col, uint8_t row);
void LiquidCrystal_I2C_write(LiquidCrystal_I2C_t *lcd, uint8_t value);
void LiquidCrystal_I2C_command(LiquidCrystal_I2C_t *lcd, uint8_t value);
void LiquidCrystal_I2C_print(LiquidCrystal_I2C_t *lcd, const char *str);

#endif
