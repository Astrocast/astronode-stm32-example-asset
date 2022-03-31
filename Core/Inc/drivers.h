#ifndef DRIVERS_H
#define DRIVERS_H


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
// Standard
#include <stdint.h>
#include <stdbool.h>


//------------------------------------------------------------------------------
// Definitions
//------------------------------------------------------------------------------
#define ASTRONODE_MAX_UART_BUFFER_LENGTH 800

#define PIN_PUSH_BUTTON     GPIO_PIN_13
#define PORT_PUSH_BUTTON    GPIOC
#define PIN_EVT_GPIO        GPIO_PIN_12
#define PORT_EVT_GPIO       GPIOA
#define PIN_USART1_TX       GPIO_PIN_9
#define PIN_USART1_RX       GPIO_PIN_10
#define PORT_USART1         GPIOA
#define PIN_USART2_TX       GPIO_PIN_2
#define PIN_USART2_RX       GPIO_PIN_3
#define PORT_USART2         GPIOA
#define PIN_RESET_GPIO      GPIO_PIN_11
#define PORT_RESET_GPIO     GPIOA


//------------------------------------------------------------------------------
// Type definitions
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Function declarations
//------------------------------------------------------------------------------
void init_drivers(void);


void send_debug_logs(char *p_data);

void send_astronode_request(uint8_t *p_data, uint32_t length);

bool is_astronode_character_received(uint8_t *p_rx_char);


bool is_message_available(void);

bool is_evt_pin_high(void);

void reset_astronode(void);


uint32_t get_systick(void);

bool is_systick_timeout_over(uint32_t starting_value, uint16_t duration);

#endif /* DRIVERS_H */
