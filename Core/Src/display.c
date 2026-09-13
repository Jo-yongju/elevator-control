#include "display.h"

/* 세그먼트 A~DP가 연결된 핀. 공통 애노드라 0=ON. */
static GPIO_TypeDef *const segPort[8] = {
    SEG_A_GPIO_Port, SEG_B_GPIO_Port, SEG_C_GPIO_Port, SEG_D_GPIO_Port,
    SEG_E_GPIO_Port, SEG_F_GPIO_Port, SEG_G_GPIO_Port, SEG_DP_GPIO_Port
};
static const uint16_t segPin[8] = {
    SEG_A_Pin, SEG_B_Pin, SEG_C_Pin, SEG_D_Pin,
    SEG_E_Pin, SEG_F_Pin, SEG_G_Pin, SEG_DP_Pin
};

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
