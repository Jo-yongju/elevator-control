#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

/* 모터 이동 방향 + 속도 단계.
   SLOW는 목적지 한 층 전 감속 구간에서 쓴다. */
typedef enum {
    MOTOR_STOP = 0,
    MOTOR_SLOW_UP,
    MOTOR_UP,
    MOTOR_SLOW_DOWN,
    MOTOR_DOWN
} MotorDir;

/* 모터가 사용할 타이머 핸들을 등록한다. */
void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel);

/* 목표 방향/속도를 설정한다. 즉시 바뀌지 않고
   Motor_Update에서 한 스텝씩 가/감속해 다가간다. */
void Motor_SetDir(MotorDir dir);

/* 메인 루프에서 주기적으로 호출.
   현재 PWM을 목표값 쪽으로 한 스텝 움직인다. (오픈루프 가감속) */
void Motor_Update(void);

/* 현재 정지 상태(중립 도달)인지 확인. */
uint8_t Motor_IsStopped(void);

#endif /* MOTOR_H */
