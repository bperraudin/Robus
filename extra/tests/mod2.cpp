#ifndef UNIT_TEST

#include <robus.h>
#include <sys_msg.h>

void rx_cb(vm_t *vm, msg_t *msg) {

}

int main(void) {
    robus_init();

    vm_t my_vm = robus_module_create(rx_cb, SERVO_TYPE, "mod2");

    while (1) {
   }

    return 0;
}

#endif /* UNIT_TEST */
