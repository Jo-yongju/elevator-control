#include "display.h"

/* 세그먼트 A~DP가 연결된 핀. 공통 애노드라 0=ON. */
static GPIO_TypeDef *segPort[8] = {GPIOA, GPIOA, GPIOB, GPIOA, GPIOA, GPIOA, GPIOB, GPIOB};
static const uint16_t segPin[8] = {GPIO_PIN_4, GPIO_PIN_0, GPIO_PIN_9, GPIO_PIN_6,
                                   GPIO_PIN_7, GPIO_PIN_1, GPIO_PIN_8, GPIO_PIN_6};

/* 숫자별 세그먼트 on/off 패턴 (1=점등) */
static const uint8_t pattern[10][8] = {
    {1, 1, 1, 1, 1, 1, 0, 0}, /* 0 */
    {0, 1, 1, 0, 0, 0, 0, 0}, /* 1 */
    {1, 1, 0, 1, 1, 0, 1, 0}, /* 2 */
    {1, 1, 1, 1, 0, 0, 1, 0}, /* 3 */
    {0, 1, 1, 0, 0, 1, 1, 0}, /* 4 */
    {1, 0, 1, 1, 0, 1, 1, 0}, /* 5 */
    {1, 0, 1, 1, 1, 1, 1, 0}, /* 6 */
    {1, 1, 1, 0, 0, 0, 0, 0}, /* 7 */
    {1, 1, 1, 1, 1, 1, 1, 0}, /* 8 */
    {1, 1, 1, 1, 0, 1, 1, 0}  /* 9 */
};

void Display_ShowDigit(uint8_t num)
{
    if (num > 9) return;

    for (int i = 0; i < 8; i++) {
        /* 1=점등인데 공통 애노드라 핀은 RESET일 때 켜진다. */
        GPIO_PinState state = pattern[num][i] ? GPIO_PIN_RESET : GPIO_PIN_SET;
        HAL_GPIO_WritePin(segPort[i], segPin[i], state);
    }
}
