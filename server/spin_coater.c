#include "./server.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

void match_sensor(int sock) {
    int len;
    char msg[256];
    
    len = read(sock, msg, sizeof(msg));
    msg[len - 1] = '\0';
    if (!strcmp(msg, "Camera")) {
        raspi.camera = sock;
        printf("Camera Connect : %d\n", sock);
    }
    else if (!strcmp(msg, "DCmotor")) {
        raspi.dc = sock;
        printf("DCmotor Connect : %d\n", sock);
    }
    else if (!strcmp(msg, "Servo")) {
        raspi.servo = sock;
        printf("Servo Connect : %d\n", sock);
    }
}

void start_spin(void) {
    int len;
    char msg[256];

    writestr(raspi.camera, "Start");
    writeLCD("Find Center", "by Camera!");
    len = read(raspi.camera, msg, sizeof(msg)); //움직일 거리 카메라에서 받아오기
    msg[len - 1] = '\0';

    printf("** By Camera = %s\n", msg);
    write(raspi.servo, msg, len); //움직일 거리 서보모터에게 전달
    writeLCD("Move to Center", "by Servo Motor!");

    len = read(raspi.servo, msg, sizeof(msg)); //Lid 감지(초음파)
    msg[len - 1] = '\0';
    if (!strcmp(msg, "LID")) {
        printf("** By Servo = %s\n", msg);
        writeLCD("Start Spin", "by DC Motor!");
        write(raspi.camera, msg, len);
        writestr(raspi.dc, "Start");
        writestr(raspi.servo, "End");
    }
    len = read(raspi.dc, msg, sizeof(msg));
    msg[len - 1] = '\0';
    printf("DCmotor = %s\n", msg);
    if (!strcmp(msg, "Success")) {
        printf("** By DCmotor = %s\n", msg);
        writeLCD("End Spin Coater", "Thank you!");
        writestr(raspi.camera, "End");
        writestr(raspi.dc, "End");
        usleep(1000 * 3000);
    }
}

void connectSocket(int port) {
    int serv_sock, clnt_sock = -1;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size;

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

    int i = 0;
    while (i < 3) {
        clnt_addr_size = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
        if (clnt_sock == -1) error_handling("accept() error");
        match_sensor(clnt_sock);
        i ++;
    }
}