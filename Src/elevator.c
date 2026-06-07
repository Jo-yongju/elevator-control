#include "elevator.h"
#include "motor.h"
#include "hall_sensor.h"

#define DOOR_OPEN_MS    3000   /* 문 열고 머무는 시간 */
#define MOVE_TIMEOUT_MS 8000   /* 이 시간 안에 다음 층 못 잡으면 비상 정지 */

static EvState  state;
static uint8_t  curFloor;
static uint8_t  targetFloor;
static EvDir    dir;                  /* SCAN 진행 방향 */
static uint8_t  requests[FLOOR_MAX];  /* 층별 호출 플래그 */
static uint32_t doorTimer;            /* 문 연 시각 */
static uint32_t moveTimer;            /* 마지막으로 층이 바뀐 시각 */

/* --- SCAN 스케줄링 보조 함수 --- */

/* 현재 층 기준 위쪽에 호출이 있는가? */
static uint8_t hasRequestAbove(void)
{
    for (int f = curFloor + 1; f <= FLOOR_MAX; f++) {
        if (requests[f - 1]) return 1;
    }
    return 0;
}

/* 현재 층 기준 아래쪽에 호출이 있는가? */
static uint8_t hasRequestBelow(void)
{
    for (int f = curFloor - 1; f >= FLOOR_MIN; f--) {
        if (requests[f - 1]) return 1;
    }
    return 0;
}

static uint8_t anyRequest(void)
{
    for (int i = 0; i < FLOOR_MAX; i++) {
        if (requests[i]) return 1;
    }
    return 0;
}

/* SCAN: 진행 방향을 유지한 채 그 방향의 가장 가까운 호출을 목표로 잡는다.
   그 방향에 더 없으면 반대로 전환한다. 목표를 못 정하면 0 반환. */
static uint8_t pickTargetScan(void)
{
    /* 방향이 없으면(정지 상태) 가까운 쪽부터 정한다. */
    if (dir == DIR_NONE) {
        if (hasRequestAbove())      dir = DIR_UP;
        else if (hasRequestBelow()) dir = DIR_DOWN;
        else                        return 0;
    }

    if (dir == DIR_UP) {
        for (int f = curFloor + 1; f <= FLOOR_MAX; f++) {
            if (requests[f - 1]) return f;
        }
        /* 위로 더 없음 -> 아래로 전환 */
        if (hasRequestBelow()) {
            dir = DIR_DOWN;
            for (int f = curFloor - 1; f >= FLOOR_MIN; f--) {
                if (requests[f - 1]) return f;
            }
        }
    } else { /* DIR_DOWN */
        for (int f = curFloor - 1; f >= FLOOR_MIN; f--) {
            if (requests[f - 1]) return f;
        }
        if (hasRequestAbove()) {
            dir = DIR_UP;
            for (int f = curFloor + 1; f <= FLOOR_MAX; f++) {
                if (requests[f - 1]) return f;
            }
        }
    }
    return 0;
}

void Elevator_Init(void)
{
    state = EV_IDLE;
    curFloor = FLOOR_MIN;
    targetFloor = 0;
    dir = DIR_NONE;
    doorTimer = 0;
    moveTimer = 0;
    for (int i = 0; i < FLOOR_MAX; i++) requests[i] = 0;
}

void Elevator_Request(uint8_t floor)
{
    if (floor < FLOOR_MIN || floor > FLOOR_MAX) return;
    requests[floor - 1] = 1;
}

uint8_t Elevator_CurrentFloor(void) { return curFloor; }
EvState Elevator_State(void)        { return state; }

void Elevator_Update(void)
{
    switch (state) {

    case EV_IDLE: {
        if (!anyRequest()) {
            dir = DIR_NONE;   /* 할 일 없으면 방향 초기화 */
            break;
        }
        /* 현재 층에 호출이 있으면 바로 문 연다. */
        if (requests[curFloor - 1]) {
            requests[curFloor - 1] = 0;
            state = EV_DOOR_OPEN;
            doorTimer = HAL_GetTick();
            break;
        }
        /* SCAN으로 다음 목표 결정 */
        uint8_t t = pickTargetScan();
        if (t != 0) {
            targetFloor = t;
            /* 목표가 바로 옆 층이면 가속할 거리가 없으니 처음부터 저속. */
            uint8_t adjacent = (t == curFloor + 1) || (t == curFloor - 1);
            if (dir == DIR_UP) {
                Motor_SetDir(adjacent ? MOTOR_SLOW_UP : MOTOR_UP);
            } else {
                Motor_SetDir(adjacent ? MOTOR_SLOW_DOWN : MOTOR_DOWN);
            }
            moveTimer = HAL_GetTick();
            state = EV_MOVING;
        }
        break;
    }

    case EV_MOVING: {
        /* 타임아웃: 일정 시간 안에 층 변화가 없으면 고장으로 간주. */
        if (HAL_GetTick() - moveTimer > MOVE_TIMEOUT_MS) {
            Motor_SetDir(MOTOR_STOP);
            state = EV_FAULT;
            break;
        }

        uint8_t detected = Hall_DetectFloor();
        if (detected != 0 && detected != curFloor) {
            curFloor = detected;
            moveTimer = HAL_GetTick();  /* 층 바뀌었으니 타임아웃 리셋 */

            /* 멈출 층인가? 목표층이거나, 지나는 길에 그 층 호출이 있으면 정지.
               (지나가며 함께 처리 = SCAN의 핵심) */
            uint8_t stopHere = (curFloor == targetFloor) || requests[curFloor - 1];

            if (stopHere) {
                Motor_SetDir(MOTOR_STOP);
                requests[curFloor - 1] = 0;
                targetFloor = 0;
                state = EV_DOOR_OPEN;
                doorTimer = HAL_GetTick();
            }
            /* 아직 안 멈춘다면: 목적지 '한 층 전'에 도달했는지 보고 감속.
               FS90R은 속도를 못 읽으므로, 홀 센서 이벤트를 트리거로
               미리 정해둔 저속 펄스로 전환한다 (이벤트 기반 속도 프로파일링). */
            else if (dir == DIR_UP && curFloor == targetFloor - 1) {
                Motor_SetDir(MOTOR_SLOW_UP);
            }
            else if (dir == DIR_DOWN && curFloor == targetFloor + 1) {
                Motor_SetDir(MOTOR_SLOW_DOWN);
            }
        }
        break;
    }

    case EV_DOOR_OPEN: {
        /* 도어 인터락: 문이 열린 동안에는 어떤 경우에도 모터를 정지로 묶는다. */
        Motor_SetDir(MOTOR_STOP);
        if (HAL_GetTick() - doorTimer >= DOOR_OPEN_MS) {
            state = EV_DOOR_CLOSING;
        }
        break;
    }

    case EV_DOOR_CLOSING: {
        /* 모터가 완전히 멈춘 것을 확인한 뒤에만 다음 이동을 허용. */
        if (Motor_IsStopped()) {
            state = EV_IDLE;
        }
        break;
    }

    case EV_FAULT: {
        /* 비상 정지 상태: 모터를 묶어두고 사람이 개입할 때까지 대기. */
        Motor_SetDir(MOTOR_STOP);
        break;
    }

    default:
        state = EV_IDLE;
        break;
    }
}
