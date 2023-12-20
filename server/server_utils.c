#include "./server.h"
#include <sys/socket.h>

int writestr(int fd, char *str) {
    return write(fd, str, strlen(str));
}

void cleanup(void) {
    clearLcd();
    touchDeinit();
    shutdown(raspi.camera, SHUT_RDWR);
    shutdown(raspi.dc, SHUT_RDWR);
    shutdown(raspi.servo, SHUT_RDWR);
    close(raspi.camera);
    close(raspi.dc);
    close(raspi.servo);
    close(raspi.server);
}

void error_handling(char *message) {
    fputs(message, stderr);
    fputc('\n', stderr);
    cleanup();
    exit(1); //프로그램 종료
}

void *error_thread(void *param) {
    int len;
    char msg[10];

    while (1) {
      // printf("Error By DCmotor = %d\n", sizeof(msg));
        len = read(raspi.dc, msg, sizeof(msg));
        msg[len - 1] = '\0';
      // printf("strcmp(msg, Error) = %d\n", strcmp(msg, "Error"));
        if (!strcmp(msg, "Error")) {
            write(raspi.camera, msg, len);
            write(raspi.dc, msg, len);
            write(raspi.servo, msg, len);
            writeLCD("!!Error!!", "By Motion Sensor");
            usleep(1000 * 3000);
            error_handling("Error By Motion Sensor");
        }
    }
}

void *stop_thread(void *param) {
    while (touchRead()); // 길게 누를 때까지
    writestr(raspi.camera, "Stop");
    writestr(raspi.servo, "Stop");
    writestr(raspi.dc, "Stop");
    usleep(3000 * 1000);
    error_handling("Stop By Press Button");
}