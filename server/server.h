#ifndef SOCKET_H
#define SOCKET_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define IN 0
#define OUT 1

typedef struct rasberry_pi { // buffer 구조체
    int camera;
    int servo;
    int dc;
    int server;
} RASBERRY_PI;

extern RASBERRY_PI raspi;

// spin_coater.c
void match_sensor(int sock);
void start_spin(void);
void connectSocket(int port);

// touch.c
void touchInit(void);
void touchDeinit(void);
int touchRead(void);

// server_utils.c
int writestr(int fd, char *str);
void cleanup(void);
void error_handling(char *message);
void *error_thread(void *param);
void *stop_thread(void *param);

//lcd.c
void lcdInit(void);
void clearLcd(void);
void writeLCD(char *msg1, char *msg2);

//gpio.c
int GPIOExport(int pin);
int GPIOUnexport(int pin);
int GPIODirection(int pin, int dir);
int GPIOWrite(int pin, int value);
int GPIORead(int pin);

#endif