#include "./server.h"

#define PIN 12

#define IN 0
#define OUT 1

void* touchToLcd(char **msg) {
    int pid;
    pid = fork();
    if (pid == -1)  error_handling("fork() failed");
    if (pid != 0) { //parent
        // printf("child pid=%d\n", pid);
        return NULL;  //parent thread 종료
    }
    if (*msg) {
        writeLCD(msg);

        for (int i = 0; i < 2; i++) free(msg[i]);
        free(msg);
    }
    exit(0); //child 종료

}

void* readTouch(void) {

    int prev_state = 1;
    int count = 0;
    pthread_t thread;
    char **msg;

    if (GPIOExport(PIN) == -1) {
        error_handling("GPIOExport error\n");
    }

    if (GPIODirection(PIN, IN) == -1) {
        error_handling("GPIODirection error\n");
    }

    msg = (char**)malloc(sizeof(char **) * 2);
    for (int i = 0; i < 2; i++) {
        msg[i] = (char *)malloc(sizeof(char *) * 256);
    }

    msg[0] = "Waiting for press";
    msg[1] = "touch sensor ...";
    // printf("Waiting for press touch sensor ...\n");
    pthread_create(&thread, NULL, touchToLcd, msg);
    if (thread < 0)
        error_handling("pthread create error\n");

    do {
        count = 0;
        for (int i = 0; i < 13; i++) {
            usleep(1000 * 100);
            if (GPIORead(PIN) != 0) count++;
            // printf("count = %d\n", count);
        }
        // if (count == 0) printf("Nothing Press!!\n");
        if (count > 8) {
            msg[0] = "Stop!!";
            msg[1] = "More than 3 Sec ..";
            // printf("Stop!! More than 3 Sec ..\n");
            pthread_create(&thread, NULL, touchToLcd, msg);
            if (thread < 0)
                error_handling("pthread create error\n");
        }
        else {
            msg[0] = "Start!!";
            msg[1] = "Less than 3 Sec ...";
            // printf("Start!! Less than 3 Sec ...\n");
            pthread_create(&thread, NULL, touchToLcd, msg);
            if (thread < 0)
                error_handling("pthread create error\n");
        }

        // printf("Waiting for press touch sensor ...\n");

    } while (1);

    if (GPIOUnexport(PIN) == -1) {
        error_handling("GPIOUnexport error\n");
    }
    exit(0);
}