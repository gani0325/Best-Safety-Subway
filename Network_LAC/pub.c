#include <stdio.h>
#include <mosquitto.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

// 순환큐
#define BUFFER_SIZE 256

typedef struct Node
{
    char data;
    struct Node *next;
} Node;

typedef struct
{
    Node *front;
    Node *rear;
} CircularQueue;

// Serial code
typedef struct
{
    int fd;
    CircularQueue circularBuffer;
} SerialPort;

void initializeCircularQueue(CircularQueue *queue)
{
    queue->front = NULL;
    queue->rear = NULL;
}

int isCircularQueueEmpty(const CircularQueue *queue)
{
    return (queue->front == NULL);
}

void enqueueCircularQueue(CircularQueue *queue, char element)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (!newNode)
    {
        // 메모리 할당 실패(선택사항)
        return;
    }

    newNode->data = element;
    newNode->next = NULL;

    if (isCircularQueueEmpty(queue))
    {
        queue->front = newNode;
        queue->rear = newNode;
    }
    else
    {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

char dequeueCircularQueue(CircularQueue *queue)
{
    if (isCircularQueueEmpty(queue))
    {
        //// 버퍼 언더플로우 처리(선택 사항)
        return '\0';
    }

    Node *temp = queue->front;
    char data = temp->data;

    queue->front = queue->front->next;
    free(temp);

    if (queue->front == NULL)
    {
        queue->rear = NULL;
    }

    return data;
}

// MQTT code
struct MosqParams
{
    const char *client_id;
    const char *broker_address;
    int broker_port;
    const char *topic;
};

int pub_main(const struct MosqParams *params, CircularQueue *serialBuffer)
{
    int rc;
    struct mosquitto *mosq;
    SerialPort port;

    if (serial_open(&port, "/dev/ttyACM0") == -1)
    {
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int bytesRead;

    // MQTT 초기화
    mosquitto_lib_init();
    mosq = mosquitto_new(params->client_id, true, NULL);

    // 브로커에 연결
    rc = mosquitto_connect(mosq, params->broker_address, params->broker_port, 60);

    // 연결이 성공했는지 확인
    if (rc != 0)
    {
        printf("Client could not connect to broker! Error Code: %d\n", rc);
        mosquitto_destroy(mosq);
        return -1;
    }
    printf("We are now connected to the broker!\n");

    // 직렬 통신 루프
    while (1)
    {
        bytesRead = serial_read(&port, buffer, sizeof(buffer));
        if (bytesRead > 0)
        {
            for (int i = 0; i < bytesRead; ++i)
            {
                enqueueCircularQueue(serialBuffer, buffer[i]);
                if (buffer[i] == '\n')
                {
                    // // 데이터를 처리하고 MQTT에 게시합니다.
                    char mqttBuffer[BUFFER_SIZE];
                    int index = 0;
                    while (!isCircularQueueEmpty(serialBuffer))
                    {
                        mqttBuffer[index++] = dequeueCircularQueue(serialBuffer);
                    }
                    mqttBuffer[index] = '\0';
                    mosquitto_publish(mosq, NULL, params->topic, strlen(mqttBuffer), mqttBuffer, 0, false);
                }
            }
        }
    }


    serial_close(&port);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}

int main()
{
    CircularQueue serialBuffer;
    initializeCircularQueue(&serialBuffer);

    struct MosqParams params = {
        .client_id = "publisher-test",
        .broker_address = "192.168.0.51",
        .broker_port = 1883,
        .topic = "test/t1"};

    pub_main(&params, &serialBuffer);

    return 0;
}
