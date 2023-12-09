#include "./server.h"
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct rasberry_pi { // buffer 구조체
  int camera;
  int servo;
  int dc;
} RASBERRY_PI;

static RASBERRY_PI raspi;

void start_spin(void) {
  char *msg;
  int len;
  msg = (char *)malloc(sizeof(char) * 256);
  if (!msg) error_handling("malloc error");
  
  write(raspi.camera, "Start", sizeof("Start"));
  len = read(raspi.camera, msg, 256); //움직일 거리 카메라에서 받아오기
  write(raspi.servo, msg, len); //움직일 거리 서보모터에게 전달
  len = read(raspi.servo, msg, 256); //Lid 감지(초음파)
  if (strcmp(msg, "LID")) {
    write(raspi.camera, msg, len);
    write(raspi.dc, "Start", sizeof("Start"));
  }
  len = read(raspi.dc, msg, 256);
  write(raspi.camera, "End", sizeof("End"));
  write(raspi.servo, "End", sizeof("End"));
  if (*msg) free(msg);
}

void* match_sensor(int sock) {
  int len;
  char *msg;
  
  msg = (char *)malloc(sizeof(char) * 256);
  if (!msg) error_handling("malloc error");
  // printf("In match_sensor sock = %d\n", sock);
  len = read(sock, msg, 256);
  msg[len - 1] = '\0';
  if (!strcmp(msg, "Camera")) raspi.camera = sock;
  if (!strcmp(msg, "DCmotor")) raspi.dc = sock;
  if (!strcmp(msg, "Servo")) raspi.servo = sock;
  if (*msg) free(msg);
  pthread_exit(0);
}

void* error_thread(void) {
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
      error_handling("Error By Motion Sensor");
    }
  }
}

// void* stop_thread(void) {
//   int pid, len;
//   char msg[5];


// }

int main(int argc, char *argv[]) {

  pthread_t lcdthread, touthread;
  int serv_sock, clnt_sock = -1;
  struct sockaddr_in serv_addr, clnt_addr;
  socklen_t clnt_addr_size;
  pthread_t thread, ethread, sthread;

  if (argc != 2) {
    printf("Usage : %s <port>\n", argv[0]);
    error_handling("argc error");
  }

  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (serv_sock == -1) error_handling("socket() error");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("bind() error");

  if (listen(serv_sock, 5) == -1) error_handling("listen() error");

  printf("Connection established\n");
  printf("serv_sock = %d\n", serv_sock);

  // lcdInit(); // setup LCD

  // pthread_create(&touthread, NULL, readTouch, NULL);
  // if (touthread < 0)
  //   error_handling("pthread create error\n");
  // pthread_join(touthread, NULL);

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
  printf("Camera = %d DCmotor = %d Servo = %d\n", raspi.camera, raspi.dc, raspi.servo);
  printf("error_thread start\n");
  pthread_create(&ethread, NULL, error_thread, NULL);
    if (ethread < 0)
      error_handling("pthread create error\n");

  // pthread_create(&sthread, NULL, stop_thread, NULL);
  //   if (sthread < 0)
  //     error_handling("pthread create error\n");

  printf("Spin Coater Start\n");
  start_spin();

  close(raspi.camera);
  close(raspi.dc);
  close(raspi.servo);
  close(serv_sock);

  return (0);
}