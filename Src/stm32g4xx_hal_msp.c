#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* htim_pwm)
{
    if (htim_pwm->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (htim->Instance == TIM3)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();

        /* TIM3_CH2 -> PC7, AF2 */
        GPIO_InitStruct.Pin = GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* htim_pwm)
{
    if (htim_pwm->Instance == TIM3)
    {
        __HAL_RCC_TIM3_CLK_DISABLE();
    }
}
