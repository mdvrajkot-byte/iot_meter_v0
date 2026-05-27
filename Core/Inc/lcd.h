#ifndef __LCD_H
#define __LCD_H

#include "main.h"

#define LCD_ICON_WIFI     0
#define LCD_ICON_GSM      1
#define LCD_ICON_BATTERY  2

void lcd_init(void);
void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_send_string(char *str);
void lcd_put_cur(int row, int col);
void lcd_clear(void);

void lcd_backlight(uint8_t state);                 
void lcd_print_int(int num);                         
void lcd_print_float(float num, int decimal_places); 
void lcd_create_custom_char(uint8_t location, uint8_t *charmap); 

void lcd_blink_display(int count, uint16_t speed_ms);
void lcd_scroll_text(int row, char *str, uint16_t speed_ms);

#endif /* __LCD_H */