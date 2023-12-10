#include "./server.h"

RASBERRY_PI raspi;

int main(int argc, char *argv[]) {
    pthread_t ethread, sthread;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        error_handling("argc error");
    }
    connectSocket(atoi(argv[1])); // connect socket
    lcdInit(); // setup LCD
    touchInit(); // setup Touch Sensor

    printf("Camera = %d DCmotor = %d Servo = %d\n", raspi.camera, raspi.dc, raspi.servo);
    pthread_create(&ethread, NULL, error_thread, NULL);
    if (ethread < 0)
        error_handling("pthread create error\n");
    writeLCD("Press To Start", "Spin Coater");
    if (!touchRead()) return (0); // 3초 이상 눌렀을 때
    writeLCD("Start!", "Spin Coater");
    usleep(1000 * 2000);

    pthread_create(&sthread, NULL, stop_thread, NULL);
    if (sthread < 0)
        error_handling("pthread create error\n");

    printf("===== Spin Coater Start =====\n");
    start_spin(); // spin coater start
    cleanup();

    return (0);
}