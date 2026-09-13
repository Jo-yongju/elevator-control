#include "elevator.h"
#include "buzzer.h"
#include "hall_sensor.h"
#include "motor.h"

#define DOOR_OPEN_MS    3000   /* 문 열고 머무는 시간 */
#define MOVE_TIMEOUT_MS 8000   /* 이 시간 안에 다음 층 못 잡으면 비상 정지 */

static EvState  state;
static uint8_t  curFloor;
static uint8_t  targetFloor;
static EvDir    dir;                  /* SCAN 진행 방향 */
static uint8_t  requests[FLOOR_MAX];  /* 층별 호출 플래그 */
static uint32_t doorTimer;            /* 문 연 시각 */
static uint32_t moveTimer;            /* 마지막으로 층이 바뀐 시각 */
static uint8_t  arrivalPending;       /* 정지 pulse 도달을 기다리는 상태 */

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

static void enterDoorOpen(void)
{
    state = EV_DOOR_OPEN;
    doorTimer = HAL_GetTick();
    Buzzer_PlayDoorOpen();
}

static void enterFault(void)
{
    Motor_StopImmediate();
    Buzzer_Stop();
    targetFloor = 0;
    arrivalPending = 0;
    state = EV_FAULT;
}

/* 이동 중 같은 방향의 더 가까운 요청이 생기면 해당 층을 먼저 처리한다. */
static void updateTargetWhileMoving(void)
{
    if (dir == DIR_UP) {
        for (int floor = curFloor + 1; floor < targetFloor; floor++) {
            if (requests[floor - 1]) {
                targetFloor = (uint8_t)floor;
                if (targetFloor == curFloor + 1U) {
                    Motor_SetDir(MOTOR_SLOW_UP);
                }
                return;
            }
        }
    } else if (dir == DIR_DOWN) {
        for (int floor = curFloor - 1; floor > targetFloor; floor--) {
            if (requests[floor - 1]) {
                targetFloor = (uint8_t)floor;
                if (targetFloor + 1U == curFloor) {
                    Motor_SetDir(MOTOR_SLOW_DOWN);
                }
                return;
            }
        }
    }
}

static uint8_t isInvalidFloorTransition(uint8_t detected)
{
    if (dir == DIR_UP) {
        return ((detected < curFloor) ||
                ((targetFloor != 0U) && (detected > targetFloor))) ? 1U : 0U;
    }
    if (dir == DIR_DOWN) {
        return ((detected > curFloor) ||
                ((targetFloor != 0U) && (detected < targetFloor))) ? 1U : 0U;
    }
    return 1U;
}

void Elevator_Init(void)
{
    uint8_t detectedFloor = Hall_DetectFloor();

    state = EV_IDLE;
    curFloor = (detectedFloor != 0U) ? detectedFloor : FLOOR_MIN;
    targetFloor = 0;
    dir = DIR_NONE;
    doorTimer = 0;
    moveTimer = 0;
    arrivalPending = 0;
    for (int i = 0; i < FLOOR_MAX; i++) requests[i] = 0;
}

void Elevator_Request(uint8_t floor)
{
    if (floor < FLOOR_MIN || floor > FLOOR_MAX) return;
    requests[floor - 1] = 1;
}

void Elevator_RequestDoorOpen(void)
{
    if (state == EV_IDLE) {
        if (!Motor_IsStopped()) {
            enterFault();
            return;
        }
        enterDoorOpen();
    }
    else if (state == EV_DOOR_OPEN) {
        /* 이미 열려 있으면 승객 대기 시간만 연장한다. */
        doorTimer = HAL_GetTick();
    }
    else if (state == EV_DOOR_CLOSING) {
        /* 닫힘 경고 중 열림 요청이 들어오면 다시 열림 상태로 전환한다. */
        enterDoorOpen();
    }
}

void Elevator_RequestDoorClose(void)
{
    if (state == EV_DOOR_OPEN) {
        state = EV_DOOR_CLOSING;
        Buzzer_PlayDoorClose();
    }
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
        /* 현재 층 호출은 모터 정지를 확인한 뒤 처리한다. */
        if (requests[curFloor - 1]) {
            if (!Motor_IsStopped()) {
                enterFault();
                break;
            }
            requests[curFloor - 1] = 0;
            enterDoorOpen();
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
        if (arrivalPending) {
            Motor_SetDir(MOTOR_STOP);
            if (Motor_IsStopped()) {
                arrivalPending = 0;
                enterDoorOpen();
            }
            break;
        }

        /* 타임아웃: 일정 시간 안에 층 변화가 없으면 고장으로 간주. */
        if (HAL_GetTick() - moveTimer > MOVE_TIMEOUT_MS) {
            enterFault();
            break;
        }

        updateTargetWhileMoving();

        uint8_t detected = Hall_DetectFloor();
        if (detected != 0 && detected != curFloor) {
            if (isInvalidFloorTransition(detected)) {
                enterFault();
                break;
            }

            curFloor = detected;
            moveTimer = HAL_GetTick();  /* 층 바뀌었으니 타임아웃 리셋 */

            /* 멈출 층인가? 목표층이거나, 지나는 길에 그 층 호출이 있으면 정지.
               (지나가며 함께 처리 = SCAN의 핵심) */
            uint8_t stopHere = (curFloor == targetFloor) || requests[curFloor - 1];

            if (stopHere) {
                Motor_SetDir(MOTOR_STOP);
                requests[curFloor - 1] = 0;
                targetFloor = 0;
                arrivalPending = 1;
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
        /* 논리적 Door 상태에서는 이동을 허용하지 않는다. */
        Motor_SetDir(MOTOR_STOP);
        if (!Motor_IsStopped()) {
            enterFault();
            break;
        }
        if (HAL_GetTick() - doorTimer >= DOOR_OPEN_MS) {
            state = EV_DOOR_CLOSING;
            Buzzer_PlayDoorClose();
        }
        break;
    }

    case EV_DOOR_CLOSING: {
        Motor_SetDir(MOTOR_STOP);
        if (!Motor_IsStopped()) {
            enterFault();
            break;
        }
        if (!Buzzer_IsBusy()) {
            state = EV_IDLE;
        }
        break;
    }

    case EV_FAULT: {
        /* PWM을 중립값으로 유지하고 사람이 개입할 때까지 대기한다. */
        Motor_StopImmediate();
        Buzzer_Stop();
        break;
    }

    default:
        enterFault();
        break;
    }
}
