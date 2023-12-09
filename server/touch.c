#include "./server.h"

#define PIN 12

#define IN 0
#define OUT 1
#define WAIT_MS 200

typedef struct touch_state {
    int press; // 누르고 있는 동안 1, 놓으면 0
    int down; // 눌려질 때(rising edge)만 1, 누르고 있는 동안은 0
    int up; // 놓여질 때(falling edge)만 1
} TOUCH_STATE;

static TOUCH_STATE touch;

static void touch_probe(int pin, TOUCH_STATE *tch) {
    int old_press = tch->press;

    tch->press = (GPIORead(pin) != 0) ? 1 : 0;
    tch->down = (old_press == 0 && tch->press == 1) ? 1 : 0; // rising edge (tch down)
    tch->up = (old_press == 1 && tch->press == 0) ? 1 : 0; // falling edge (tch up)
}

void touchInit(void) {
    if (GPIOExport(PIN) == -1) {
        error_handling("GPIOExport error\n");
    }

    if (GPIODirection(PIN, IN) == -1) {
        error_handling("GPIODirection error\n");
    }
}

void touchDeinit(void) {
    if (GPIOUnexport(PIN) == -1) {
        error_handling("GPIOUnexport error\n");
    }
}

int touchRead(void) {
    int sec, count = 0;

    touch.press = touch.down = touch.up = 0;
    while (!touch.up) {
        usleep(WAIT_MS * 1000); // 0.1 sec
        touch_probe(PIN, &touch);
        if (touch.down) count = 0;
        else if (touch.press) count++;  
    }
    sec = count * WAIT_MS / 1000;
    if (sec < 3) return 1;
    writeLCD("Stop!!", "More than 3 Sec ...");
    return 0;
}
