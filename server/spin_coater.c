#include "./server.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

void* match_sensor(int sock) {
    int len;
    char msg[256];
    
    len = read(sock, msg, sizeof(msg));
    msg[len - 1] = '\0';
    if (!strcmp(msg, "Camera")) raspi.camera = sock;
    else if (!strcmp(msg, "DCmotor")) raspi.dc = sock;
    else if (!strcmp(msg, "Servo")) raspi.servo = sock;
}

void start_spin(void) {
    int len;
    char msg[256];
    
    writestr(raspi.camera, "Start");
    writeLCD("Find Center", "by Camera");
    len = read(raspi.camera, msg, sizeof(msg)); //움직일 거리 카메라에서 받아오기
    write(raspi.servo, msg, len); //움직일 거리 서보모터에게 전달
    writeLCD("Move to Center", "by Servo Motor");
    len = read(raspi.servo, msg, sizeof(msg)); //Lid 감지(초음파)
    if (strcmp(msg, "LID")) {
        writeLCD("Start Spin Coater", "by DC Motor");
        write(raspi.camera, msg, len);
        writestr(raspi.dc, "Start");
    }
    len = read(raspi.dc, msg, sizeof(msg));
    writeLCD("End Spin Coater", "Thank you~!");
    writestr(raspi.camera, "End");
    writestr(raspi.servo, "End");
}

void connectSocket(int port) {
    int serv_sock, clnt_sock = -1;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;
    pthread_t thread;

    serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (serv_sock == -1) error_handling("socket() error");
    raspi.server = serv_sock;

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        error_handling("bind() error");

    if (listen(serv_sock, 5) == -1) error_handling("listen() error");

    printf("Connection established\n");
    printf("serv_sock = %d\n", serv_sock);

    while (clnt_sock < 6) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) error_handling("accept() error");
        printf("clnt_sock = %d\n", clnt_sock);
        pthread_create(&thread, NULL, match_sensor, clnt_sock);
        if (thread < 0)
            error_handling("pthread create error\n");
        pthread_join(thread, NULL);
    }
}