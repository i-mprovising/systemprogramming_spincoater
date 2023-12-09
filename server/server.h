#ifndef SOCKET_H
#define SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

void error_handling(char *message);

void* readTouch(void);

void* writeLCD(char **arr);
void lcdInit(void);
void clearLcd(void);

int GPIOWrite(int pin, int value);
int GPIORead(int pin);
int GPIODirection(int pin, int dir);
int GPIOUnexport(int pin);
int GPIOExport(int pin);

#endif