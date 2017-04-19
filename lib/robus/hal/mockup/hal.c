#include "hal.h"

#include <stdio.h>
#include <pthread.h>

#include "zhelpers.h"


void *sock;
void *context;

pthread_t tid[1];

#define RECV_BUFFER_SIZE 256
unsigned char buffer[RECV_BUFFER_SIZE];

#define BROKER_PORT 9001


void *hal_receive(void *args);


void hal_init(void) {
    char addr[256];

    context = zmq_ctx_new();

    // First, ask the broker for a dedicated communication port.
    void *broker = zmq_socket(context, ZMQ_REQ);

    snprintf(addr, sizeof(addr), "tcp://localhost:%d", BROKER_PORT);
    zmq_connect(broker, addr);

    s_send(broker, "register");
    char *msg = s_recv(broker);
    int port = atoi(msg);
    free(msg);
    zmq_close(broker);

    // Then, open this communication that acts as a broadcast.
    sock = zmq_socket(context, ZMQ_PAIR);
    snprintf(addr, sizeof(addr), "tcp://localhost:%d", port);
    zmq_connect(sock, addr);

    // Finally, spawns a thread that poll messages and trigger the data cb.
    int err = pthread_create(&(tid[0]), NULL, &hal_receive, NULL);
    if (err != 0) {
      printf("Can't create thread :[%s]\n", strerror(err));
    }
}


unsigned char hal_transmit(unsigned char* data, unsigned short size) {
    // Send the message to the broker that will broadcast it to all modules.
    int sent_size = zmq_send(sock, data, size, 0);

    if (sent_size != size) {
        printf("Transmission failed!\n");
        return 1;
    }

    return 0;
}

void *hal_receive(void *args) {
    while (1) {
        int recv_size = zmq_recv(sock, buffer, RECV_BUFFER_SIZE, 0);

        if (recv_size != -1) {
            for (unsigned short i = 0; i < recv_size; i++) {
               ctx.data_cb(buffer+i);
            }
        } else {
            printf("Something bad happened...\n");
        }
    }
    return NULL;
}
