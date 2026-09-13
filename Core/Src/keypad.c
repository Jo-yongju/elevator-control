#include "keypad.h"

#define ROWS 4
#define COLS 4

/* Row: 출력, Col: 입력(Pull-up). 핀 매핑은 보드 배선에 맞춰 둔다. */
static GPIO_TypeDef *const rowPort[ROWS] = {
    KEY_ROW1_GPIO_Port, KEY_ROW2_GPIO_Port, KEY_ROW3_GPIO_Port, KEY_ROW4_GPIO_Port
};
static const uint16_t rowPin[ROWS] = {
    KEY_ROW1_Pin, KEY_ROW2_Pin, KEY_ROW3_Pin, KEY_ROW4_Pin
};

static GPIO_TypeDef *const colPort[COLS] = {
    KEY_COL1_GPIO_Port, KEY_COL2_GPIO_Port, KEY_COL3_GPIO_Port, KEY_COL4_GPIO_Port
};
static const uint16_t colPin[COLS] = {
    KEY_COL1_Pin, KEY_COL2_Pin, KEY_COL3_Pin, KEY_COL4_Pin
};

static const char keymap[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static char scanRawKey(void)
{
    char detected = 0;

    for (int row = 0; row < ROWS; row++) {
        /* 모든 Row를 HIGH, 검사할 Row 하나만 LOW로 떨어뜨린다. */
        for (int r = 0; r < ROWS; r++) {
            HAL_GPIO_WritePin(rowPort[r], rowPin[r], GPIO_PIN_SET);
        }
        HAL_GPIO_WritePin(rowPort[row], rowPin[row], GPIO_PIN_RESET);
        HAL_Delay(1);  /* 라인 안정화 */

        for (int col = 0; col < COLS; col++) {
            if (HAL_GPIO_ReadPin(colPort[col], colPin[col]) == GPIO_PIN_RESET) {
                detected = keymap[row][col];
                break;
            }
        }
        if (detected != 0) break;
    }

    /* 스캔이 끝난 뒤에는 모든 Row를 비활성 상태로 되돌린다. */
    for (int row = 0; row < ROWS; row++) {
        HAL_GPIO_WritePin(rowPort[row], rowPin[row], GPIO_PIN_SET);
    }

    return detected;
}

char Keypad_Scan(void)
{
    static char heldKey;
    char currentKey = scanRawKey();

    if (currentKey == 0) {
        heldKey = 0;
        return 0;
    }

    if (currentKey == heldKey) {
        return 0;
    }

    heldKey = currentKey;
    return currentKey;
}
