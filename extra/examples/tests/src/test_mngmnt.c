/*
 * test_mngmnt.c
 *
 * Created: 11/02/2015 12:40:48
 *  Author: Nicolas Rabault
 */

#include "test_mngmnt.h"
#include <stdio.h>

int test_nb = 0;
int test_failed = 0;
int test_seq_nb = 0;


unsigned char test(unsigned char result) {
    test_nb++;
    if (!result) {
        test_failed++;
        printf("\x1B[31mtest %d FAIlED\n\x1B[0m", ++test_seq_nb);
        return 1;
    }
    printf("\x1B[32mtest %d SUCCES\n\x1B[0m", ++test_seq_nb);
    return 0;
}

int test_end(void) {
    if (test_failed) {
        printf("\n\x1B[31m%d/%d test failed.\n\x1B[0m", test_failed, test_nb);
        return 1;
    }
    printf("\n\x1B[32mall %d test pass\n\x1B[0m", test_nb);
    return 0;
}

void test_sequences(unsigned char (*test_func)(void)) {
    static int sequence_nb = 0;
    test_seq_nb = 0;
    if (test_func())
        printf("\x1B[31mSequence number %d failed.\n\x1B[0m", sequence_nb++);
    else
        printf("\x1B[32mSequence number %d succes.\x1B[0m\n", sequence_nb++);
}
