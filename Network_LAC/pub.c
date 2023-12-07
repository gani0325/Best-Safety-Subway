#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// 시리얼 코드
typedef struct
{
    int fd;
} SerialPort;

int serial_open2(SerialPort *port, const char *device)
{
    port->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (port->fd == -1)
    {
        perror("Unable to open port");
        return -1;
    }

    struct termios options;
    tcgetattr(port->fd, &options);
    cfsetispeed(&options, B9600);
    // cfsetospeed(&options, B9600);

    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    tcsetattr(port->fd, TCSANOW, &options);

    return 0;
}

void serial_close2(SerialPort *port)
{
    close(port->fd);
    port->fd = -1;
}

int serial_read2(SerialPort *port, char *buffer, size_t size)
{
    return read(port->fd, buffer, size);
}

int pub_main(int a)
{
    int rc = a;
    struct mosquitto *mosq;
    SerialPort port;

    if (serial_open2(&port, "/dev/ttyACM0") == -1)
    {
        return -1;
    }

    char buffer[256];
    int bytesRead;
    // 모스키토 초기화 시작
    //  초기화
    mosquitto_lib_init();

    // 모스키도 런타임 객체와 클라이언트 id 생성
    mosq = mosquitto_new("publisher-test", true, NULL);
    // 모스키토 초기화 여기까지

    // Connecting 브로커
    // 브로커와 연결 확인 Keep-alive(x)
    // 킵얼라이브(Keep-alive) 디바이스간의 데이터 링크가 잘 동작하고 있는지 확인하는 행위
    rc = mosquitto_connect(mosq, "192.168.0.51", 1883, 60);

    // 브로커와 연결이 x 출력
    if (rc != 0)
    {
        printf("Client could not connect to broker! Error Code: %d\n", rc);
        mosquitto_destroy(mosq);
        return -1;
    }
    printf("We are now connected to the broker!\n");

    // 시리얼
    while (1)
    {
        bytesRead = serial_read2(&port, buffer, sizeof(buffer));
        if (bytesRead > 0)
        {
            if (buffer[bytesRead - 1] == '\n')
            {
                buffer[bytesRead] = '\0';

                mosquitto_publish(mosq, NULL, "test/t1", 15, buffer, 0, false);
            }
        }
    }

    serial_close2(&port);
    return 0;

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);

    mosquitto_lib_cleanup();
    return 0;
}
// 모스키도 코드
int main()
{
    pub_main();
}