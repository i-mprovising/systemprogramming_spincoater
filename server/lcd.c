// #include "./server.h"
// #include <wiringPiI2C.h>
// #include <wiringPi.h>

#define I2C_ADDR   0x27 // I2C device address

#define LCD_CHR  1
#define LCD_CMD  0

#define LINE1  0x80
#define LINE2 0xC0
#define LCD_BACKLIGHT 0x08
#define ENABLE 0b00000100 

int fd;

void lcdByte(int bits, int mode);
void lcd_toggle_enable(int bits);

// clear lcd
void clearLcd(void) {
    lcdByte(0x01, LCD_CMD);
    lcdByte(0x02, LCD_CMD);
}

void lcdLoc(int line) {
    lcdByte(line, LCD_CMD);
}

void writeLine(const char *s) {
    while ( *s ) lcdByte(*(s++), LCD_CHR);
}

void lcdByte(int bits, int mode) {

    int bits_high;
    int bits_low;
    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

    // High bits
    wiringPiI2CReadReg8(fd, bits_high);
    lcd_toggle_enable(bits_high);

    // Low bits
    wiringPiI2CReadReg8(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

// Toggle enable
void lcd_toggle_enable(int bits) {
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits | ENABLE));
    delayMicroseconds(500);
    wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
    delayMicroseconds(500);
}

void lcdInit() {
    if (wiringPiSetup() == -1) error_handling("WiringPi Error");
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcdByte(0x33, LCD_CMD); // 초기화
    lcdByte(0x32, LCD_CMD);
    lcdByte(0x06, LCD_CMD);
    lcdByte(0x0C, LCD_CMD);
    lcdByte(0x28, LCD_CMD);
    lcdByte(0x01, LCD_CMD);
    delayMicroseconds(500);
}

void writeLCD(char *msg1, char *msg2) {
    clearLcd();
    lcdLoc(LINE1);
    writeLine(msg1);
    lcdLoc(LINE2);
    writeLine(msg2);
}
