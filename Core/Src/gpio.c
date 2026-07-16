/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(ICM_CS_GPIO_Port, ICM_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, nRF_CS_Pin|nRF_CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : ICM_CS_Pin */
  GPIO_InitStruct.Pin = ICM_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(ICM_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : nRF_CS_Pin nRF_CE_Pin */
  GPIO_InitStruct.Pin = nRF_CS_Pin|nRF_CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */
void nRF_CSN0_Set(void)
{
  HAL_GPIO_WritePin(nRF_CS_GPIO_Port, nRF_CS_Pin, GPIO_PIN_SET);
}

void nRF_CSN0_Reset(void)
{
  HAL_GPIO_WritePin(nRF_CS_GPIO_Port, nRF_CS_Pin, GPIO_PIN_RESET);
}

void nRF_CE_Set(void)
{
  HAL_GPIO_WritePin(nRF_CE_GPIO_Port, nRF_CE_Pin, GPIO_PIN_SET);
}

void nRF_CE_Reset(void)
{
  HAL_GPIO_WritePin(nRF_CE_GPIO_Port, nRF_CE_Pin, GPIO_PIN_RESET);
}


/* USER CODE END 2 */
