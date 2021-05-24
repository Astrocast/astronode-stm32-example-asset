//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard
#include <stdbool.h>
#include <stdint.h>
#include <string.h>  // strlen

// ST
#include "stm32l4xx_hal.h"
#include "drivers.h"


//------------------------------------------------------------------------------
// Global variable definitions
//------------------------------------------------------------------------------
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

uint8_t g_number_of_message_to_send = 0;


//------------------------------------------------------------------------------
// Function declarations
//------------------------------------------------------------------------------
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
void SystemClock_Config(void);
void Error_Handler(void);


//------------------------------------------------------------------------------
// Function definitions
//------------------------------------------------------------------------------
bool is_message_available(void)
{
    if (g_number_of_message_to_send > 0)
    {
        g_number_of_message_to_send--;
        return true;
    }
    else
    {
        return false;
    }
}

bool is_evt_pin_high(void)
{
    return (HAL_GPIO_ReadPin(PORT_EVT_GPIO, PIN_EVT_GPIO) == GPIO_PIN_SET ? true : false);
}

void reset_astronode(void)
{
    HAL_GPIO_WritePin(PORT_RESET_GPIO, PIN_RESET_GPIO, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(PORT_RESET_GPIO, PIN_RESET_GPIO, GPIO_PIN_RESET);
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(PORT_RESET_GPIO, PIN_RESET_GPIO, GPIO_PIN_RESET);

    /*Configure GPIO pins PA11 */
    GPIO_InitStruct.Pin = PIN_RESET_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(PORT_RESET_GPIO, &GPIO_InitStruct);

    /*Configure GPIO pin : PC13 */
    GPIO_InitStruct.Pin = PIN_PUSH_BUTTON;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(PORT_PUSH_BUTTON, &GPIO_InitStruct);

    /* EXTI interrupt init*/
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

    /*Configure GPIO pin : PA12 */
    GPIO_InitStruct.Pin = PIN_EVT_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(PORT_EVT_GPIO, &GPIO_InitStruct);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN)
{
    g_number_of_message_to_send++;
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
    Error_Handler();
    }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
    Error_Handler();
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 10;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
    Error_Handler();
    }
    /** Initializes the CPU, AHB and APB busses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
    Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
    Error_Handler();
    }
    /** Configure the main internal regulator output voltage
     */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
    Error_Handler();
    }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{

}

void init_drivers(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
}

void send_debug_logs(char *p_tx_buffer)
{
    uint32_t length = strlen(p_tx_buffer);

    if (length > ASTRONODE_MAX_UART_BUFFER_LENGTH)
    {
        HAL_UART_Transmit(&huart2, (uint8_t *) "[ERROR] UART buffer reached max length.\n", 42, 1000);
        length = ASTRONODE_MAX_UART_BUFFER_LENGTH;
    }

    HAL_UART_Transmit(&huart2, (uint8_t *) p_tx_buffer, length, 1000);
    HAL_UART_Transmit(&huart2, (uint8_t *) "\n", 1, 1000);
}

void send_astronode_request(uint8_t *p_tx_buffer, uint32_t length)
{
    send_debug_logs("Message sent to the Astronode --> ");
    send_debug_logs((char *) p_tx_buffer);

    HAL_UART_Transmit(&huart1, p_tx_buffer, length, 1000);
}

bool is_astronode_character_received(uint8_t *p_rx_char)
{
    return (HAL_UART_Receive(&huart1, p_rx_char, 1, 100) == HAL_OK ? true : false);
}

uint32_t get_systick(void)
{
    return HAL_GetTick();
}

bool is_systick_timeout_over(uint32_t starting_value, uint16_t duration)
{
    return (get_systick() - starting_value > duration) ? true : false;
}