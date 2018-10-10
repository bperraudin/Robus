/**
 * \file robus.h
 * \brief Robus communication main include file.
 * \author Nicolas Rabault
 * \version 0.1
 * \date 18 Fevrier 2017
 *
 * Include this file to use the robus communication protocol.
 *
 */

#ifndef _ROBUS_H_
#define _ROBUS_H_

#define MAX_ALIAS_SIZE 16
#define MAX_DATA_MSG_SIZE 256
#define MAX_MULTICAST_ADDRESS 16

enum {
    NULLBOARD,
    DEV_BOARD,
    MOTOR_BOARD
}module_list_t;

/**
 * \enum target_mode_t
 * \brief Message addressing mode enum.
 *
 * This structure is used to get the message addressing mode list.
 */
typedef enum {
    ID,        /*!< Unique or virtual ID, used to send something to only one module. */
    IDACK,     /*!< Unique or virtual ID with reception Acknoledgment (ACK). */
    TYPE,      /*!< Type mode, used to send something to all module of the same type. */
    BROADCAST, /*!< Broadcast mode, used to send something to everybody. */
    MULTICAST  /*!< Multicast mode, used to send something to multiple modules. */
}target_mode_t;

/**
 * \struct msg_t
 * \brief Message structure.
 *
 * This structure is used to receive or send messages between modules in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            unsigned short protocol : 4;       /*!< Protocol version. */
            unsigned short target : 12;        /*!< Target address, it can be (ID, Multicast/Broadcast, Type). */
            unsigned short target_mode : 4;    /*!< Select targeting mode (ID, ID+ACK, Multicast/Broadcast, Type). */
            unsigned short source : 12;        /*!< Source address, it can be (ID, Multicast/Broadcast, Type). */
            unsigned char cmd;                 /*!< msg definition. */
            unsigned char size;                /*!< Size of the data field. */
        };
        unsigned char unmap[6];                /*!< Uncmaped form. */
    };
}header_t;

/**
 * \struct msg_t
 * \brief Message structure.
 *
 * This structure is used to receive or send messages between modules in slave
 * and master mode.
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)){
    union {
        struct __attribute__((__packed__)){
            header_t header;              /*!< Header filed. */
            unsigned char data[MAX_DATA_MSG_SIZE];      /*!< Data with size known. */
        };
        unsigned char stream[sizeof(header_t) + MAX_DATA_MSG_SIZE]; /*!< unmaped option. */
    };
    union {
        unsigned short crc;
        volatile unsigned char ack;
    };
}msg_t;

/**
 * \struct vm_t
 * \brief Virtual Module Structure
 *
 * This structure is used to manage virtual modules
 * please refer to the documentation
 */
typedef struct __attribute__((__packed__)) vm_t{
    // Callback
    void (*rx_cb) (struct vm_t* vm, msg_t *msg);

    // Module infomations
    unsigned short id;       /*!< Module ID. */
    unsigned char type;     /*!< Module type. */
    char alias[MAX_ALIAS_SIZE];/*!< Module alias. */

    // Variables
    msg_t* msg_pt;          /*!< Message pointer. */
    unsigned char max_multicast_target; /*!< Position pointer of the last multicast target. */
    unsigned short multicast_target_bank[MAX_MULTICAST_ADDRESS]; /*!< multicast target bank. */
    unsigned char message_available; /*!< signal a new message available */
    unsigned char data_to_read; /*!< data ready to be readed */
}vm_t;

typedef void (*RX_CB) (vm_t* vm, msg_t *msg);

/**
 * \fn void robus_init(void)
 * \brief Initialisation of the Robus communication lib.
 *
 */
void robus_init(void);

/**
 * \fn vm_t* robus_module_create(RX_CB rx_cb, unsigned char type, unsigned char *alias)
 * \brief Initialisation of the Robus communication lib.
 *
 * \param rx_cb function pointer into the rx callback.
 * \param type type reference of this module hardware.
 * \param alias string (15 caracters max).
 *
 * \return virtual module object pointer.
 *
 */
vm_t* robus_module_create(RX_CB rx_cb, unsigned char type, const char *alias);

/**
 * \fn unsigned char robus_send(vm_t* vm, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who send.
 * \param msg Message to send to the slave.
 *
 * \return send or not
 */
unsigned char robus_send(vm_t* vm, msg_t *msg);

/**
 * \fn unsigned char robus_send_sys(vm_t* vm, msg_t *msg)
 * \brief  Send system message function.
 *
 * \param virtual module who send.
 * \param msg Message to send to the slave.
 *
 * \return send or not
 */
unsigned char robus_send_sys(vm_t* vm, msg_t *msg);

/**
 * \fn unsigned char robus_read(vm_t* vm, msg_t *msg)
 * \brief  Send message function.
 *
 * \param virtual module who receive.
 *
 * \return a data
 */
unsigned char robus_read(vm_t* vm);

/**
 * \fn void save_alias(vm_t* vm, char* alias)
 * \brief  Save Alias in EEprom.
 *
 * \param concerned virtual module.
 * \param name string.
 *
 */
void save_alias(vm_t* vm, char* alias);

#endif /* _ROBUS_H_ */