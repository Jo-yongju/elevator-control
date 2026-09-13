#include "hall_sensor.h"

#define FLOORS        5
#define DEBOUNCE_MS  20   /* 이 시간 동안 같은 값이 유지돼야 인정 */

static GPIO_TypeDef *const hallPort[FLOORS] = {
    HALL_1_GPIO_Port, HALL_2_GPIO_Port, HALL_3_GPIO_Port,
    HALL_4_GPIO_Port, HALL_5_GPIO_Port
};
static const uint16_t hallPin[FLOORS] = {
    HALL_1_Pin, HALL_2_Pin, HALL_3_Pin, HALL_4_Pin, HALL_5_Pin
};

/* 한 센서가 '눌림(RESET)' 상태인지 확인한다. */
static uint8_t isActive(int i)
{
    return (HAL_GPIO_ReadPin(hallPort[i], hallPin[i]) == GPIO_PIN_RESET);
}

uint8_t Hall_DetectFloor(void)
{
    for (int i = 0; i < FLOORS; i++) {
        if (isActive(i)) {
            /* 흔들림(채터링) 제거: 잠깐 뒤에 다시 봐서
               여전히 같은 상태일 때만 진짜 감지로 인정한다. */
            uint32_t start = HAL_GetTick();
            while (HAL_GetTick() - start < DEBOUNCE_MS) {
                if (!isActive(i)) {
                    return 0;  /* 중간에 풀리면 노이즈로 판단 */
                }
            }
            return i + 1;  /* DEBOUNCE_MS 내내 유지됨 -> 확정 */
        }
    }
    return 0;
}
