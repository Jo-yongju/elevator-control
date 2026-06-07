#ifndef HALL_SENSOR_H
#define HALL_SENSOR_H

#include "main.h"

/* 홀 센서로 현재 층을 읽는다.
   감지된 층(1~5)을 반환, 어느 센서도 안 잡히면 0. */
uint8_t Hall_DetectFloor(void);

#endif /* HALL_SENSOR_H */
