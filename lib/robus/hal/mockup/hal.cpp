#include "hal.h"

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include <queue>
#include <string>

#include "zhelpers.h"


void *broker;
void *recv_sock;
void *send_sock;
void *context;

std::queue<std::string> msg_queue;
std::queue<unsigned char> broadcast_queue;

pthread_t tid[3];

#define RECV_BUFFER_SIZE 256

#define BROKER_PORT 9001


void hal_init(void) {
}


unsigned char hal_transmit(unsigned char* data, unsigned short size) {
    // Send the message to the broker that will broadcast it to all modules.

    s_send(send_sock, (char *)"broadcast");
    int sent_size = zmq_send(send_sock, data, size, 0);

    if (sent_size != size) {
        printf("Transmission failed!\n");
        return 1;
    }

    return 0;
}

void hal_timeout(int factor) {
    usleep(10000 * factor);
}

void *hal_receive(void *args) {
  while (1) {
    unsigned char *header_buf = (unsigned char *)malloc(sizeof(unsigned char) * RECV_BUFFER_SIZE);
    unsigned char *msg_buf = (unsigned char *)malloc(sizeof(unsigned char) * RECV_BUFFER_SIZE);

    int header_size = zmq_recv(recv_sock, header_buf, RECV_BUFFER_SIZE, 0);
    int msg_size = zmq_recv(recv_sock, msg_buf, RECV_BUFFER_SIZE, 0);

    if (strcmp((char *)header_buf, "broadcast") == 0) {
      for (unsigned short i = 0; i < msg_size; i++) {
        broadcast_queue.push(msg_buf[i]);
      }

    } else if (strcmp((char *)header_buf, "ptp left") == 0) {
      msg_queue.push("ptp left");
      msg_queue.push(std::string((char *)msg_buf));

    } else if (strcmp((char *)header_buf, "ptp right") == 0) {
      msg_queue.push("ptp right");
      msg_queue.push(std::string((char *)msg_buf));

    }

    free(header_buf);
    free(msg_buf);
  }
  return NULL;
}

void *broadcast_msg_handler(void *args) {
  while (1) {
    if (broadcast_queue.size() < 1) {
      usleep(1000);
      continue;
    }
    unsigned char c = broadcast_queue.front();
    broadcast_queue.pop();

    ctx.data_cb(&c);
  }
  return NULL;
}

void *ptp_msg_handler(void *args) {
  while (1) {
    if (msg_queue.size() < 2) {
      usleep(1000);
      continue;
    }

    std::string header = msg_queue.front();
    msg_queue.pop();

    std::string msg = msg_queue.front();
    msg_queue.pop();

    if (msg == "poke") {
      if (header == "ptp left") {
        poke_detected(BRANCH_B);

      } else if (header == "ptp right") {
        poke_detected(BRANCH_A);
      }

    } else if (msg == "keepline") {
      if (header == "ptp left") {
        ptp_detected(BRANCH_B);

      } else if (header == "ptp right") {
        ptp_detected(BRANCH_A);
      }

    } else if (msg == "reset") {
      if (header == "ptp left") {
        ptp_released(BRANCH_B);

      } else if (header == "ptp right") {
        ptp_released(BRANCH_A);

      }
    } else {
      printf("Unknown ptp message %s\n", msg.c_str());
    }
  }
  return NULL;
}


void send_poke(branch_t branch) {
    if (branch == BRANCH_A) {
        s_send(send_sock, (char *)"ptp right");
        s_send(send_sock, (char *)"poke");
    }
    else if (branch == BRANCH_B) {
      s_send(send_sock, (char *)"ptp left");
      s_send(send_sock, (char *)"poke");
    }
}

void set_PTP(branch_t branch) {
    if (branch == BRANCH_A) {
        s_send(send_sock, (char *)"ptp right");
        s_send(send_sock, (char *)"keepline");
    }
    else if (branch == BRANCH_B) {
        s_send(send_sock, (char *)"ptp left");
        s_send(send_sock, (char *)"keepline");
    }
}

void reset_PTP(branch_t branch) {
    if (branch == BRANCH_A) {
        s_send(send_sock, (char *)"ptp right");
        s_send(send_sock, (char *)"reset");
    }
    else if (branch == BRANCH_B) {
        s_send(send_sock, (char *)"ptp left");
        s_send(send_sock, (char *)"reset");
    }
}


void plug(side_t side) {
    char addr[256];
    char msg[256];

    context = zmq_ctx_new();

    // First, ask the broker for a dedicated communication port.
    broker = zmq_socket(context, ZMQ_REQ);

    snprintf(addr, sizeof(addr), "tcp://localhost:%d", BROKER_PORT);
    zmq_connect(broker, addr);

    switch (side) {
      case LEFT_SIDE:
        snprintf(msg, 256, "register left");
        break;
      case RIGHT_SIDE:
        snprintf(msg, 256, "register right");
        break;
      case NO_SIDE:
        snprintf(msg, 256, "register none");
        break;
    }

    s_send(broker, msg);

    char *resp = s_recv(broker);
    std::string s(resp);
    int send_port = atoi(s.substr(0, s.find(" ")).c_str());
    int recv_port = atoi(s.substr(s.find(" ")+1, sizeof(resp)).c_str());
    free(resp);

    // Then, open this communication that acts as a broadcast.
    recv_sock = zmq_socket(context, ZMQ_PAIR);
    snprintf(addr, sizeof(addr), "tcp://localhost:%d", recv_port);
    zmq_connect(recv_sock, addr);

    send_sock = zmq_socket(context, ZMQ_PAIR);
    snprintf(addr, sizeof(addr), "tcp://localhost:%d", send_port);
    zmq_connect(send_sock, addr);


    int err;
    err = pthread_create(&(tid[0]), NULL, &ptp_msg_handler, NULL);
    if (err != 0) {
        printf("Can't create thread: [%s]\n", strerror(err));
    }

    err = pthread_create(&(tid[1]), NULL, &broadcast_msg_handler, NULL);
    if (err != 0) {
        printf("Can't create thread: [%s]\n", strerror(err));
    }

    // Finally, spawns a thread that poll messages and trigger the data cb.
    err = pthread_create(&(tid[2]), NULL, &hal_receive, NULL);
    if (err != 0) {
        printf("Can't create thread:[%s]\n", strerror(err));
    }
}
