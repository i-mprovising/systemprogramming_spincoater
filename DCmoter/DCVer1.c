#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define DC_PIN "18"         // GPIO 18번 (DC 모터)
#define BUFFER_SIZE 1024

int CSocket;  // Client Socket 변수

//pthread_mutex_t motionMutex = PTHREAD_MUTEX_INITIALIZER; // 뮤텍스 초기화

void exportPin(const char *pin) {
    int fd;
    fd = open("/sys/class/gpio/export", O_WRONLY);
    write(fd, pin, strlen(pin));
    close(fd);
}

void setPinDirection(const char *pin, const char *direction) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", pin);
    int fd = open(path, O_WRONLY);
    write(fd, direction, strlen(direction));
    close(fd);
}

void setPinValue(const char *pin, const char *value) {
    char path[50];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin);
    int fd = open(path, O_WRONLY);
    write(fd, value, strlen(value));
    close(fd);
}

void* motorThread(void* arg) {
    while (1) {
        char buffer[BUFFER_SIZE];
        read(CSocket, buffer, BUFFER_SIZE);
        int Check_stop = 0, Ending = 0;

        if (strcmp(buffer, "Start") == 0) {
            printf("start\n");
            setPinValue(DC_PIN, "1");
            //usleep(5000000);
            int start_time = (unsigned)time(NULL);
            start_time += 5;

            while(1) {
                //read(CSocket, buffer, BUFFER_SIZE);
                if(strcmp(buffer, "Stop") == 0){
                    printf("STOP\n");
                    setPinValue(DC_PIN, "0");
                    Check_stop = 1;
                    break;
                }
                else{
                    int endTime = (unsigned)time(NULL);
                    int times = start_time - endTime;

                    printf("%d\n",times);

                    if(times == 0) {
                        printf("end\n");
                        setPinValue(DC_PIN, "0");

                        write(CSocket, "Success\n", strlen("Success\n"));
                        printf("msg_Check\n");
                        Ending = 1;
                        break;
                    }
                }
            }
        }
        else if (strcmp(buffer, "End") == 0) {
            printf("ProgramEnd\n");
            break;
        }

        if(Check_stop == 1){
            //printf("Bye\n");
            break;
        }
        else if(Ending == 1){
            break;
        }
    }
    
    //printf("WhileOUt\n");
    close(CSocket);
    return NULL;
}

int main(int argc, char *argv[]) {
    struct sockaddr_in SAddr; // 서버 소켓 주소

    CSocket = socket(AF_INET, SOCK_STREAM, 0); // 클라이언트 소켓 생성

    memset(&SAddr, 0, sizeof(SAddr)); // server 통신 주소 메모리 초기화
    SAddr.sin_family = AF_INET; // IPv4
    SAddr.sin_addr.s_addr = inet_addr(argv[1]);
    SAddr.sin_port = htons(atoi(argv[2]));

    if( -1 == connect( CSocket, (struct sockaddr *)&SAddr, sizeof(SAddr)) ){
        printf( "접속 실패\n"); 
        exit(1);
    }

    printf("Connect!!\n");
    write(CSocket, "DCmotor", sizeof("DCmotor"));
    
    exportPin(DC_PIN);
    setPinDirection(DC_PIN, "out");
    /*
    printf("set\n");
    setPinValue(DC_PIN, "0");
    */

    pthread_t motor_tid;

    pthread_create(&motor_tid, NULL, motorThread, NULL);

    pthread_join(motor_tid, NULL);


    return 0;
}