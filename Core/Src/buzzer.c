#include "buzzer.h"

#define DOOR_OPEN_BEEP_MS   300U
#define DOOR_CLOSE_BEEP_MS  100U
#define DOOR_CLOSE_BEEP_COUNT 3U

typedef enum {
    BUZZER_IDLE = 0,
    BUZZER_OPEN_BEEP,
    BUZZER_CLOSE_BEEP_ON,
    BUZZER_CLOSE_BEEP_OFF
} BuzzerState;

static BuzzerState state;
static uint8_t closeBeepsRemaining;
static uint32_t nextTransition;

static void setOutput(GPIO_PinState output)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, output);
}

static uint8_t timeReached(uint32_t now, uint32_t deadline)
{
    return ((int32_t)(now - deadline) >= 0) ? 1U : 0U;
}

void Buzzer_Init(void)
{
    state = BUZZER_IDLE;
    closeBeepsRemaining = 0U;
    nextTransition = 0U;
    setOutput(GPIO_PIN_RESET);
}

void Buzzer_PlayDoorOpen(void)
{
    setOutput(GPIO_PIN_SET);
    state = BUZZER_OPEN_BEEP;
    nextTransition = HAL_GetTick() + DOOR_OPEN_BEEP_MS;
}

void Buzzer_PlayDoorClose(void)
{
    closeBeepsRemaining = DOOR_CLOSE_BEEP_COUNT;
    setOutput(GPIO_PIN_SET);
    state = BUZZER_CLOSE_BEEP_ON;
    nextTransition = HAL_GetTick() + DOOR_CLOSE_BEEP_MS;
}

void Buzzer_Stop(void)
{
    setOutput(GPIO_PIN_RESET);
    state = BUZZER_IDLE;
    closeBeepsRemaining = 0U;
}

void Buzzer_Update(void)
{
    uint32_t now = HAL_GetTick();

    if ((state == BUZZER_IDLE) || !timeReached(now, nextTransition)) {
        return;
    }

    switch (state) {
    case BUZZER_OPEN_BEEP:
        Buzzer_Stop();
        break;

    case BUZZER_CLOSE_BEEP_ON:
        setOutput(GPIO_PIN_RESET);
        closeBeepsRemaining--;
        if (closeBeepsRemaining == 0U) {
            state = BUZZER_IDLE;
        } else {
            state = BUZZER_CLOSE_BEEP_OFF;
            nextTransition = now + DOOR_CLOSE_BEEP_MS;
        }
        break;

    case BUZZER_CLOSE_BEEP_OFF:
        setOutput(GPIO_PIN_SET);
        state = BUZZER_CLOSE_BEEP_ON;
        nextTransition = now + DOOR_CLOSE_BEEP_MS;
        break;

    default:
        Buzzer_Stop();
        break;
    }
}

uint8_t Buzzer_IsBusy(void)
{
    return (state != BUZZER_IDLE) ? 1U : 0U;
}
