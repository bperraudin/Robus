/*
 * test_mngmnt.h
 *
 * Created: 11/02/2015 12:40:48
 *  Author: Nicolas Rabault
 */

#ifndef test_mngmnt_H_
#define test_mngmnt_H_

unsigned char test(unsigned char result);
int test_end(void);
void test_sequences(unsigned char (*test_func)(void));

#endif /* test_mngmnt_H_ */
