#include "lcd.h"
#include "cmsis_os.h" // FreeRTOS API માટે

// પિન કન્ટ્રોલ શોર્ટ મેક્રોઝ (CubeMX Labels મુજબ)
#define LCD_RS_0() HAL_GPIO_WritePin(mcu_lcd_rs_GPIO_Port, mcu_lcd_rs_Pin, GPIO_PIN_RESET)
#define LCD_RS_1() HAL_GPIO_WritePin(mcu_lcd_rs_GPIO_Port, mcu_lcd_rs_Pin, GPIO_PIN_SET)
#define LCD_EN_0() HAL_GPIO_WritePin(mcu_lcd_en_GPIO_Port, mcu_lcd_en_Pin, GPIO_PIN_RESET)
#define LCD_EN_1() HAL_GPIO_WritePin(mcu_lcd_en_GPIO_Port, mcu_lcd_en_Pin, GPIO_PIN_SET)

// 4-બિટ ફિઝિકલ ડેટા ટ્રાન્સફર (RTOS Safe)
void lcd_write_4bit(uint8_t data)
{
    HAL_GPIO_WritePin(mcu_lcd_d4_GPIO_Port, mcu_lcd_d4_Pin, (data >> 0) & 0x01);
    HAL_GPIO_WritePin(mcu_lcd_d5_GPIO_Port, mcu_lcd_d5_Pin, (data >> 1) & 0x01);
    HAL_GPIO_WritePin(mcu_lcd_d6_GPIO_Port, mcu_lcd_d6_Pin, (data >> 2) & 0x01);
    HAL_GPIO_WritePin(mcu_lcd_d7_GPIO_Port, mcu_lcd_d7_Pin, (data >> 3) & 0x01);
    for (volatile int i = 0; i < 50; i++)
    {
        
    }
    LCD_EN_1();
    osDelay(1); 
    LCD_EN_0();
    osDelay(1);
    for (volatile int i = 0; i < 100; i++);
}

void lcd_send_cmd(char cmd)
{
    LCD_RS_0();
    lcd_write_4bit((cmd >> 4) & 0x0F);
    lcd_write_4bit(cmd & 0x0F);
}

void lcd_send_data(char data)
{
    LCD_RS_1();
    lcd_write_4bit((data >> 4) & 0x0F);
    lcd_write_4bit(data & 0x0F);
}

void lcd_clear(void)
{
    lcd_send_cmd(0x01);
    osDelay(2);
}

void lcd_put_cur(int row, int col)
{
    switch (row)
    {
        case 0: lcd_send_cmd(0x80 + col); break;
        case 1: lcd_send_cmd(0xC0 + col); break;
    }
}

void lcd_send_string(char *str)
{
    while (*str) lcd_send_data(*str++);
}

// 🚀 બેકલાઈટ કન્ટ્રોલ
void lcd_backlight(uint8_t state)
{
    if(state)
        HAL_GPIO_WritePin(mcu_lcd_led_GPIO_Port, mcu_lcd_led_Pin, GPIO_PIN_SET); 
    else
        HAL_GPIO_WritePin(mcu_lcd_led_GPIO_Port, mcu_lcd_led_Pin, GPIO_PIN_RESET);
}

// 🚀 ઓપ્ટિમાઇઝ્ડ ઇન્ટિજર પ્રિન્ટિંગ (No sprintf)
void lcd_print_int(int num)
{
    char buf[12];
    int i = 0;

    if (num == 0) {
        lcd_send_data('0');
        return;
    }

    if (num < 0) {
        lcd_send_data('-');
        num = -num;
    }

    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }

    while (i > 0) {
        lcd_send_data(buf[--i]);
    }
}

// 🚀 ઓપ્ટિમાઇઝ્ડ ફ્લોટ પ્રિન્ટિંગ (No sprintf, Error-Free Rounding)
void lcd_print_float(float num, int decimal_places)
{
    if (num < 0.0f) {
        lcd_send_data('-');
        num = -num;
    }

    float rounding = 0.5f;
    for (int i = 0; i < decimal_places; i++) {
        rounding /= 10.0f;
    }
    num += rounding;

    int integer_part = (int)num;
    lcd_print_int(integer_part);

    if (decimal_places > 0) {
        lcd_send_data('.');
        float fractional_part = num - (float)integer_part;
        
        for (int i = 0; i < decimal_places; i++) {
            fractional_part *= 10.0f;
            int digit = (int)fractional_part;
            lcd_print_int(digit);
            fractional_part -= (float)digit;
        }
    }
}

// 🚀 CGRAM કસ્ટમ આઇકોન મેકર
void lcd_create_custom_char(uint8_t location, uint8_t *charmap)
{
    location &= 0x07;
    lcd_send_cmd(0x40 + (location << 3));
    for (int i = 0; i < 8; i++)
    {
        lcd_send_data(charmap[i]);
    }
}

// એડવાન્સ ઇનિશિયલાઇઝેશન સિક્વન્સ
void lcd_init(void)
{
    osDelay(100);
    lcd_write_4bit(0x03);
    osDelay(5);
    lcd_write_4bit(0x03);
    osDelay(1);
    lcd_write_4bit(0x03);
    osDelay(10);
    
    lcd_write_4bit(0x02);
    osDelay(10);

    lcd_send_cmd(0x28);
    osDelay(1);
    lcd_send_cmd(0x0C);
    osDelay(1);
    lcd_send_cmd(0x06);
    osDelay(1);
    lcd_clear();
    
    lcd_backlight(1); // ડિફોલ્ટ ઓન
}

/**
 * @brief આખી સ્ક્રીનના લખાણને બ્લિંક (ઝબકાવવા) કરવા માટે
 * @param count: કેટલી વાર બ્લિંક કરાવવું છે
 * @param speed_ms: ઓન-ઓફ વચ્ચેનો સમય (મિલીસેકન્ડમાં)
 */
void lcd_blink_display(int count, uint16_t speed_ms)
{
    for (int i = 0; i < count; i++)
    {
        lcd_send_cmd(0x08); // Display OFF (લખાણ ગાયબ થશે)
        osDelay(speed_ms);
        lcd_send_cmd(0x0C); // Display ON (લખાણ પાછું આવશે)
        osDelay(speed_ms);
    }
}

/**
 * @brief લાંબા લખાણને ડાબેથી જમણે સ્ક્ર્રોલ (રોટેટ) કરાવવા માટે
 * @param row: કઈ લાઇન પર સ્ક્ર્રોલ કરવું છે (0 અથવા 1)
 * @param str: જે લખાણ સ્คร્રોલ કરવું હોય તે સ્ટ્રિંગ
 * @param speed_ms: સ્ક્ર્રોલિંગની સ્પીડ (દર કેટલા સમયે અક્ષર ખસવો જોઈએ)
 */
void lcd_scroll_text(int row, char *str, uint16_t speed_ms)
{
    int len = 0;
    // સાદા લોજિકથી સ્ટ્રિંગની લંબાઈ (Length) ગણવી (No strlen)
    while (str[len] != '\0') {
        len++;
    }

    // જો લખાણ ૧૬ અક્ષર કે તેથી નાનું હોય, તો સ્ક્ર્રોલ કરવાની જરૂર નથી, સીધું પ્રિન્ટ કરો
    if (len <= 16) {
        lcd_put_cur(row, 0);
        lcd_send_string(str);
        return;
    }

    // સ્ક્ર્રોલિંગ લોજિક
    for (int i = 0; i <= (len - 16); i++)
    {
        lcd_put_cur(row, 0);
        // એક સમયે માત્ર ૧૬ કેરેક્ટર જ સ્ક્રીન પર મોકલવા
        for (int j = 0; j < 16; j++)
        {
            lcd_send_data(str[i + j]);
        }
        osDelay(speed_ms); // સ્પીડ કન્ટ્રોલ
    }
}