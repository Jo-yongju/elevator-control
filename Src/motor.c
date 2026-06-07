#include "motor.h"

/* 서보 펄스 폭(타이머 카운트). 1500=중립.
   위/아래로 벌어질수록 빠르고, 중립에 가까울수록 느리다. */
#define PULSE_STOP        1500
#define PULSE_UP          2100
#define PULSE_DOWN         900
#define PULSE_SLOW_UP     1700   /* 감속 구간: 중립에 가까운 저속 */
#define PULSE_SLOW_DOWN   1300

#define RAMP_STEP           20   /* 한 번에 바꿀 펄스 양 (작을수록 부드러움) */
#define RAMP_PERIOD_MS      20   /* 가감속 갱신 주기 */

static TIM_HandleTypeDef *s_htim;
static uint32_t s_channel;

static int16_t curPulse;     /* 지금 출력 중인 펄스 */
static int16_t targetPulse;  /* 도달하려는 펄스 */
static uint32_t lastRamp;    /* 마지막 가감속 시각 */

static void applyPulse(int16_t p)
{
    __HAL_TIM_SET_COMPARE(s_htim, s_channel, (uint32_t)p);
}

void Motor_Init(TIM_HandleTypeDef *htim, uint32_t channel)
{
    s_htim = htim;
    s_channel = channel;
    curPulse = PULSE_STOP;
    targetPulse = PULSE_STOP;
    lastRamp = 0;
    applyPulse(curPulse);
}

void Motor_SetDir(MotorDir dir)
{
    switch (dir) {
        case MOTOR_UP:        targetPulse = PULSE_UP;        break;
        case MOTOR_SLOW_UP:   targetPulse = PULSE_SLOW_UP;   break;
        case MOTOR_DOWN:      targetPulse = PULSE_DOWN;      break;
        case MOTOR_SLOW_DOWN: targetPulse = PULSE_SLOW_DOWN; break;
        default:              targetPulse = PULSE_STOP;      break;
    }
}

void Motor_Update(void)
{
    if (HAL_GetTick() - lastRamp < RAMP_PERIOD_MS) return;
    lastRamp = HAL_GetTick();

    if (curPulse == targetPulse) return;

    /* 목표 쪽으로 RAMP_STEP 만큼만 이동.
       현재 속도를 측정하지 않는 오픈루프 방식이다 (PID 아님). */
    if (curPulse < targetPulse) {
        curPulse += RAMP_STEP;
        if (curPulse > targetPulse) curPulse = targetPulse;
    } else {
        curPulse -= RAMP_STEP;
        if (curPulse < targetPulse) curPulse = targetPulse;
    }
    applyPulse(curPulse);
}

uint8_t Motor_IsStopped(void)
{
    return (curPulse == PULSE_STOP && targetPulse == PULSE_STOP);
}
