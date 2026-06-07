#include "keypad.h"

#define ROWS 4
#define COLS 4

/* Row: 출력, Col: 입력(Pull-up). 핀 매핑은 보드 배선에 맞춰 둔다. */
static GPIO_TypeDef *rowPort[ROWS] = {GPIOC, GPIOC, GPIOA, GPIOB};
static const uint16_t rowPin[ROWS] = {GPIO_PIN_5, GPIO_PIN_4, GPIO_PIN_10, GPIO_PIN_3};

static GPIO_TypeDef *colPort[COLS] = {GPIOB, GPIOB, GPIOB, GPIOA};
static const uint16_t colPin[COLS] = {GPIO_PIN_5, GPIO_PIN_4, GPIO_PIN_10, GPIO_PIN_8};

static const char keymap[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

char Keypad_Scan(void)
{
    for (int row = 0; row < ROWS; row++) {
        /* 모든 Row를 HIGH, 검사할 Row 하나만 LOW로 떨어뜨린다. */
        for (int r = 0; r < ROWS; r++) {
            HAL_GPIO_WritePin(rowPort[r], rowPin[r], GPIO_PIN_SET);
        }
        HAL_GPIO_WritePin(rowPort[row], rowPin[row], GPIO_PIN_RESET);
        HAL_Delay(1);  /* 라인 안정화 */

        for (int col = 0; col < COLS; col++) {
            if (HAL_GPIO_ReadPin(colPort[col], colPin[col]) == GPIO_PIN_RESET) {
                return keymap[row][col];
            }
        }
    }
    return 0;
}
