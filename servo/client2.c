#include "./module.h"

// #define SERVO1 1 // -> GPIO18, PWM0
#define SERVO1 0 // PWM
#define SERVO2 23 // wPi pin number for servo
#define SERVO3 24 
#define SERVO4 26
#define POUT 23 // PCM pin number for ultrasonic
#define PIN 24

#define PWM_PERIOD 20000000
#define SERVO_MAX_DUTY 13
#define SERVO_MIN_DUTY 2.5
#define LID_MIN 5
#define LID_MAX 10
#define SOCK 3

int connect_socket(int argc, char *argv[]) {
	// args : ip address, port number
	if (argc != 3) {
		printf("-> Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

    int sock;
	struct sockaddr_in serv_addr;

	sock = socket(PF_INET, SOCK_STREAM, 0); // domain, type, protocol
	if (sock == -1) error_handling("-> socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        error_handling("-> connect() error");
    }

	printf("* Connection established\n");

    return sock;
}

int init_values() {
    int min_duty = SERVO_MIN_DUTY * PWM_PERIOD / 100;
    PWMWriteDutyCycle(SERVO1, min_duty);
    softPwmWrite(SERVO2, min_duty/100000);
    softPwmWrite(SERVO3, min_duty/100000);
    softPwmWrite(SERVO4, min_duty/100000);
    GPIOWrite(POUT, 0);
    printf("* Move devices to inital spot\n");
    usleep(1000000);
    return 0;
}

int setup() {
    // 1. servo motor
    // use pwm for servo1
    PWMExport(SERVO1);
    PWMWritePeriod(SERVO1, PWM_PERIOD);
    PWMEnable(SERVO1);

    // use wiringpi softpwm for servo2,3,4
    wiringPiSetup();
    pinMode(SERVO2, OUTPUT);
    pinMode(SERVO3, OUTPUT);
    pinMode(SERVO4, OUTPUT);
    softPwmCreate(SERVO2, 0, 200);
    softPwmCreate(SERVO3, 0, 200);
    softPwmCreate(SERVO4, 0, 200);

    // 2. ultrasonic sensor
    if (-1 == GPIOExport(POUT) || -1 == GPIOExport(PIN)) {
        printf("-> gpio export error\n");
        return (1);
    }
    usleep(100000);

    if (-1 == GPIODirection(POUT, OUT) || -1 == GPIODirection(PIN, IN)) {
        printf("-> gpio direction error\n");
        return (2);
    }

    init_values();
    printf("* Setup done\n");

    return 0;
}

void* ultrasonic() {
    clock_t start_t, end_t;
    double time, distance;
    int count = 0; // 3초 이상 뚜껑이 감지됐을 경우 서버에 알리기 위한 count
    printf("* Detecting lid... %d < distance < %d\n", LID_MIN, LID_MAX);

    while (1) {
        if (-1 == GPIOWrite(POUT, 1)) {
            printf("gpio write/trigger err\n");
            return (3);
        }
        // 1sec == 1000000ultra_sec, 1ms = 1000ultra_sec
        usleep(10);
        GPIOWrite(POUT, 0);
        while (GPIORead(PIN) == 0) {
            start_t = clock();
        }
        while (GPIORead(PIN) == 1) {
            end_t = clock();
        }

        time = (double)(end_t - start_t) / CLOCKS_PER_SEC;  // ms
        distance = time / 2 * 34000;
        // printf("time : %.4lf\n", time);
        printf("\tdistance : %.2lfcm\n", distance);

        if (LID_MIN < distance && distance < LID_MAX) {
            count++;
            if (count > 6) {
                write(SOCK, "LID", sizeof("LID")); // 뚜껑이 감지된 것을 서버에 알림.  
                printf("* Lid is on\n");
                return 0;
            }
        } else {
            count = 0;
        }

        usleep(500000);
    }

}

int move_wafer(char dir, int dist) {
    int duty;
    duty = pixel2duty(dist);
    if (dir == 'U') { 
        printf("* Move down \n");
        duty = duty / 100000;
        softPwmWrite(SERVO1, duty);
    }
    else if (dir == 'D') {
        printf("* Move up \n");
        duty = duty / 100000;
        softPwmWrite(SERVO2, duty);
    }
    else if (dir == 'R') {
        printf("* Move left \n");
        duty = duty / 100000;
        softPwmWrite(SERVO3, duty);
    }
    else if (dir == 'L') {
        printf("* Move right \n");
        // softPwmWrite(SERVO4, duty);
        PWMWriteDutyCycle(SERVO4, duty);
    }
    usleep(1000000);
    return 0;
}

void* control_servo(char msg[]) {
    char *ptr = strtok(msg, " ");
    char d1, d2;
    int n1, n2;

    for(int i = 0; i < 4; i++) {
        if (i == 0) {
            d1 = *ptr;
        }
        if (i == 1) {
            n1 = atoi(ptr);
        }
        if (i == 2) {
            d2 = *ptr;
        }
        if (i == 3) {
            n2 = atoi(ptr);
        }
        ptr = strtok(NULL, " ");
    }

    move_wafer(d1, n1);
    move_wafer(d2, n2);

    init_values();

    ultrasonic();
    printf("* Exit servo thread\n");
    pthread_exit(0);
}

int pixel2duty(int pixel) {
    int dist, duty, percent;
    double deg;
    // pixel to distance
    dist = pixel*(4);

    // distance to degree : 3mm <-> 10도
    deg = (10/3) * dist;

    // degree to duty
    percent = SERVO_MIN_DUTY + (deg * (SERVO_MAX_DUTY - SERVO_MIN_DUTY) / 180.0);
    duty = PWM_PERIOD * percent / 100;

    return duty;
}

void* read_server() {
	char msg[100];
	int str_len;
    int flag = 0; // check if servo control thread has created
    pthread_t p_thread[1];
    int thr_id, status;
    printf("* Read from server\n");

	while(1) {
        str_len = read(SOCK, msg, sizeof(msg));
    
		if (str_len == -1) {
            error_handling("read() error");
            
        }
        else if (str_len > 0) {
            printf("* msg from server : %.*s\n", str_len, msg);
            if (strncmp(msg, "Stop", 4) * strncmp(msg, "Error", 5) == 0) {
                // 종료 신호
                if (flag == 1){
                    pthread_cancel(p_thread[0]);
                }
                pthread_exit(0);
            }
            else if(strncmp(msg, "Camera", 6)){
                // Camera
                thr_id = pthread_create(&p_thread[0], NULL, control_servo, msg);
                flag = 1;
            }
        }
	}
}

void (*breakCapture)(int);

void exit_process(int signo) {
    // unexport pins
    GPIOUnexport(PIN);
    GPIOUnexport(POUT);

    // write initial value to devices
    init_values();
    
    printf("* Exit \n");
    usleep(500000);
    exit(0);
}

int main(int argc, char *argv[]) {
    connect_socket(argc, argv);
    write(SOCK, "Servo", sizeof("Servo"));

    setup();

    // signal handle to unexport pins
    setsid();
    umask(0);
    breakCapture = signal(SIGINT, exit_process);

	// create threads
	pthread_t p_thread[1];
	int thr_id;
	int status;

    // read server
    thr_id = pthread_create(&p_thread[0], NULL, read_server, NULL);
	if (thr_id < 0) {
		perror("-> thread create error : ");
		exit(1);
	}
    
    pthread_join(p_thread[0], (void **)&status);
    exit_process(0);
    
    return 0;
}