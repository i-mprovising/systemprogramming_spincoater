#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUFFER_MAX 1024

char dist[20];
int startflag = 0;

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 40
#define DIRECTION_MAX 128

#define RED 17
#define GREEN 27
#define BLUE 22

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
void camera()
{
    FILE *fp;
    system("raspistill -t 1000 -o captures/cam.jpg");
    sleep(3);

    /* Python 스크립트 실행 */
    fp = popen("python3 getdist.py", "r");
    if (fp == NULL)
    {
        error_handling("Failed to run command");
    }

    /* 스크립트 출력을 읽어 dist로 전달 */
    fgets(dist, sizeof(dist), fp);

    /* 파일 포인터 닫기 */
    pclose(fp);
}

static int GPIOExport(int pin)
{
#define GPIO_BUFFER 3
    char buffer[GPIO_BUFFER];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open export for writing!\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, GPIO_BUFFER, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

static int GPIODirection(int pin, int dir)
{
    static const char s_directions_str[] = "in\0out";

    char path[DIRECTION_MAX];
    int fd;

    snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio direction for writing!\n");
        return (-1);
    }

    if (-1 ==
        write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3))
    {
        fprintf(stderr, "Failed to set direction!\n");
        return (-1);
    }

    close(fd);
    return (0);
}

static int GPIOWrite(int pin, int value)
{
    static const char s_values_str[] = "01";

    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open gpio value for writing!\n");
        return (-1);
    }

    if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1))
    {
        fprintf(stderr, "Failed to write value!\n");
        return (-1);
    }

    close(fd);
    return (0);
}

static int GPIOUnexport(int pin)
{
    char buffer[GPIO_BUFFER];
    ssize_t bytes_written;
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (-1 == fd)
    {
        fprintf(stderr, "Failed to open unexport for writing!\n");
        return (-1);
    }

    bytes_written = snprintf(buffer, GPIO_BUFFER, "%d", pin);
    write(fd, buffer, bytes_written);
    close(fd);
    return (0);
}

// 소켓으로 전송
void *thread_me_to_socket(void *arg)
{
    int sock = *(int *)arg;
    int x, y;
    char buffer[BUFFER_MAX];
    char piece[1];

    while (1)
    {
        if (startflag == 1)
        {
            // int n = read(sock, buffer, BUFFER_MAX);
            printf("Start!\n");
            camera();
            printf("camera check!\n");
            char *token = strtok(dist, "/"); // "/"를 구분자로 사용하여 문자열 분할
            x = atoi(token);
            token = strtok(NULL, "/"); // 다음 분할 위치로 이동
            y = atoi(token);

            if (x < 0)
            { // 위
                printf("up");
                x = 0 - x;
                strcpy(buffer, "U ");
                sprintf(piece, "%d", x);
                strcat(buffer, piece);
                strcat(buffer, " ");
            }
            else
            {
                printf("down");
                strcpy(buffer, "D ");
                sprintf(piece, "%d", x);
                strcat(buffer, piece);
                strcat(buffer, " ");
            }

            if (y < 0)
            { // 왼
                printf("left");
                y = 0 - y;
                strcat(buffer, "L");
                strcat(buffer, " ");
                sprintf(piece, "%d", y);
                strcat(buffer, piece);
                strcat(buffer, " ");
            }
            else
            {

                printf("right");
                strcat(buffer, "R");
                strcat(buffer, " ");
                sprintf(piece, "%d", y);
                strcat(buffer, piece);
                strcat(buffer, " ");
            }
            write(sock, buffer, sizeof(buffer));
            printf("Send!\n");
            startflag = 0;
            break;
        }
    }

    return NULL;
}

// 소켓 내용 수용
void *thread_socket_to_me(void *arg)
{
    int sock = *(int *)arg;
    char buffer[BUFFER_MAX];
    while (1)
    {
        int n = read(sock, buffer, BUFFER_MAX);
        write(STDOUT_FILENO, buffer, n);
        if (n <= 0) // 연결 해제 시 종료
        {
            if (GPIOWrite(RED, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(GREEN, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(BLUE, 1) == -1)
            {
                // return 3;
            }
            usleep(2000*1000);
            if (GPIOWrite(BLUE, 0) == -1)
            {
                // return 3;
            }
            printf("Server Disconnection!\n");
            exit(0);
        }
        if (strcmp(buffer, "Start") == 0)
        {
            printf("Start!\n");
            if (GPIOWrite(RED, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(BLUE, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(GREEN, 1) == -1)
            {
                // return 3;
            }
            startflag = 1;
        }
        else if (strcmp(buffer, "Stop") == 0)
        { // led 파란색
            if (GPIOWrite(RED, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(GREEN, 0) == -1)
            {
                // return 3;
            }
            printf("Stop!\n");
            if (GPIOWrite(BLUE, 1) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(BLUE, 0) == -1)
            {
                // return 3;
            }
        }
        else if (strcmp(buffer, "Error") == 0)
        { // led 빨간색
            if (GPIOWrite(GREEN, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(BLUE, 0) == -1)
            {
                // return 3;
            }
            printf("Error!\n");
            if (GPIOWrite(RED, 1) == -1)
            {
                // return 3;
            }
        }
        else if (strcmp(buffer, "LID") == 0)
        { // led 빨파빨파
            if (GPIOWrite(RED, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(GREEN, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(BLUE, 0) == -1)
            {
                // return 3;
            }
            int i = 0;
            printf("Danger!\n");
            while (i < 7)
            {
                if (GPIOWrite(RED, 1) == -1)
                {
                    // return 3;
                }
                if (GPIOWrite(BLUE, 1) == -1)
                {
                    // return 3;
                }
                usleep(500 * 1000);
                if (GPIOWrite(RED, 0) == -1)
                {
                    // return 3;
                }
                if (GPIOWrite(BLUE, 0) == -1)
                {
                    // return 3;
                }
                usleep(500 * 1000);
                i++;
                i++;
            }
        }
        else if (strcmp(buffer, "wait") == 0)
        { // led 노란색(깜빡깜빡)
            if (GPIOWrite(RED, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(GREEN, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(BLUE, 0) == -1)
            {
                // return 3;
            }
            int i = 0;
            printf("Wait!\n");
            while (i < 5)
            {
                if (GPIOWrite(RED, 1) == -1)
                {
                    // return 3;
                }
                if (GPIOWrite(GREEN, 1) == -1)
                {
                    // return 3;
                }
                usleep(500 * 1000);
                if (GPIOWrite(RED, 0) == -1)
                {
                    // return 3;
                }
                if (GPIOWrite(GREEN, 0) == -1)
                {
                    // return 3;
                }
                usleep(500 * 1000);
                i++;
            }
        }
        else if (strcmp(buffer, "End") == 0)
        { // led 초록색
            if (GPIOWrite(RED, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(GREEN, 0) == -1)
            {
                // return 3;
            }
            if (GPIOWrite(BLUE, 0) == -1)
            {
                // return 3;
            }
            int i = 0;
            printf("End!\n");
            while (i < 5)
            {
                if (GPIOWrite(GREEN, 1) == -1)
                {
                    // return 3;
                }
                usleep(1000 * 1000);
                i++;
            }
            if (GPIOWrite(GREEN, 0) == -1)
            {
                // return 3;
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[])
{

    char path[1035];
    int sock;
    struct sockaddr_in serv_addr;
    pthread_t thread1, thread2;
    char buffer[BUFFER_MAX];

    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    // 소켓 생성
    sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Error creating socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        error_handling("connect() error");
    }

    // 연결 성공 시 문구 출력
    printf("Connection established\n");
    write(sock, "Camera", sizeof("Camera"));

    if (GPIOExport(RED) == -1)
    {
        return 1;
    }
    if (GPIOExport(GREEN) == -1)
    {
        return 1;
    }
    if (GPIOExport(BLUE) == -1)
    {
        return 1;
    }

    if (GPIODirection(RED, OUT) == -1)
    {
        return 2;
    }
    if (GPIODirection(GREEN, OUT) == -1)
    {
        return 2;
    }
    if (GPIODirection(BLUE, OUT) == -1)
    {
        return 2;
    }

    pthread_create(&thread1, NULL, thread_me_to_socket, &sock);
    pthread_create(&thread2, NULL, thread_socket_to_me, &sock);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    close(sock);
    return 0;
}
