#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>

// // 큐 노드 정의
typedef struct QueueNode {
    struct mosquitto_message *msg;
    struct QueueNode *next;
} QueueNode;


typedef struct {
    QueueNode *front;
    QueueNode *rear;
} MessageQueue;


void initializeQueue(MessageQueue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
}


int isQueueEmpty(const MessageQueue *queue) {
    return (queue->front == NULL);
}


void enqueueMessage(MessageQueue *queue, struct mosquitto_message *msg) {
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    if (!newNode) {
        // Memory allocation failed (optional)
        return;
    }

    newNode->msg = msg;
    newNode->next = NULL;

    if (isQueueEmpty(queue)) {
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}


struct mosquitto_message* dequeueMessage(MessageQueue *queue) {
    if (isQueueEmpty(queue)) {
        // Queue underflow handling (optional)
        return NULL;
    }

    QueueNode *temp = queue->front;
    struct mosquitto_message *msg = temp->msg;

    queue->front = queue->front->next;
    free(temp);

    if (queue->front == NULL) {
        queue->rear = NULL;
    }

    return msg;
}

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    printf("Connected to the broker with result code: %d\n", rc);
    mosquitto_subscribe(mosq, NULL, "test/t1", 0);
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    printf("Received message: %s\n", (char *)msg->payload);
}

int main() {
    int rc, id = 12;

    mosquitto_lib_init();

    struct mosquitto *mosq;
    MessageQueue messageQueue;

    initializeQueue(&messageQueue);

    mosq = mosquitto_new("subscribe-test", true, &id);
    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect(mosq, "192.168.0.51", 1883, 10);
    if (rc) {
        printf("Could not connect to Broker with return code %d\n", rc);
        return -1;
    }

    mosquitto_loop_start(mosq);

    while (1) {
        struct mosquitto_message *newMsg = (struct mosquitto_message *)malloc(sizeof(struct mosquitto_message));
        newMsg->payload = (void *)"Hello, MQTT!";
        newMsg->payloadlen = strlen("Hello, MQTT!");
        enqueueMessage(&messageQueue, newMsg);

        while (!isQueueEmpty(&messageQueue)) {
            struct mosquitto_message *queuedMsg = dequeueMessage(&messageQueue);
            on_message(mosq, NULL, queuedMsg);
            free(queuedMsg);
        }

        usleep(500000); // 500ms
    }

    printf("Press Enter to quit...\n");
    getchar();
    mosquitto_loop_stop(mosq, true);

    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
