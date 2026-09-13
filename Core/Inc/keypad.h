#ifndef KEYPAD_H
#define KEYPAD_H

#include "main.h"

/* 키패드를 한 번 스캔한다.
   눌린 키가 있으면 해당 문자를, 없으면 0을 반환. */
char Keypad_Scan(void);

#endif /* KEYPAD_H */
