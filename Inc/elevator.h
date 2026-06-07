#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "main.h"

#define FLOOR_MIN 1
#define FLOOR_MAX 5

/* 엘리베이터가 가질 수 있는 상태 */
typedef enum {
    EV_IDLE,         /* 호출 대기 */
    EV_MOVING,       /* 목표 층으로 이동 중 */
    EV_DOOR_OPEN,    /* 문 열고 승객 대기 */
    EV_DOOR_CLOSING, /* 닫는 중 */
    EV_FAULT         /* 비상 정지 (타임아웃 등) */
} EvState;

/* 진행 방향 (SCAN 알고리즘용) */
typedef enum {
    DIR_NONE = 0,
    DIR_UP,
    DIR_DOWN
} EvDir;

void Elevator_Init(void);
void Elevator_Request(uint8_t floor);
void Elevator_Update(void);
uint8_t Elevator_CurrentFloor(void);

/* 현재 상태를 외부에서 확인 (디스플레이/디버그용). */
EvState Elevator_State(void);

#endif /* ELEVATOR_H */
