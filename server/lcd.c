// #include <wiringPiI2C.h>
// #include <wiringPi.h>
#include "./server.h"

#define I2C_ADDR   0x27 // I2C device address

#define LCD_CHR  1
#define LCD_CMD  0

#define LINE1  0x80
#define LINE2 0xC0
#define LCD_BACKLIGHT 0x08
#define ENABLE 0b00000100 

void lcdByte(int bits, int mode);
void lcd_toggle_enable(int bits);

int fd;

// clr lcd go home loc 0x80
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

// Toggle enable pin on LCD display
void lcd_toggle_enable(int bits) {
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}

void lcdInit() {
  lcdByte(0x33, LCD_CMD); // Initialise
  lcdByte(0x32, LCD_CMD); // Initialise
  lcdByte(0x06, LCD_CMD); // Cursor move direction
  lcdByte(0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcdByte(0x28, LCD_CMD); // Data length, number of lines, font size
  lcdByte(0x01, LCD_CMD); // Clear display
  delayMicroseconds(500);
}

void* writeLCD(char **arr) {

  if (wiringPiSetup () == -1) exit (1);
  fd = wiringPiI2CSetup(I2C_ADDR);

  //printf("fd = %d ", fd);
  clearLcd();
  // while (1)   {
  // delay(2000);
  lcdLoc(LINE1);
  writeLine(arr[0]);
  lcdLoc(LINE2);
  writeLine(arr[1]);
  // usleep(10000 * 10000);
  // }
}