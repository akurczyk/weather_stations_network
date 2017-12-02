#include "stm32f4xx_hal.h"
#include "usart.h"
#include "stdlib.h"
#include "string.h"

void esp_write_line(UART_HandleTypeDef* handler, char* text);
void esp_read_line(UART_HandleTypeDef* handler, char* buffer, uint16_t buffer_size);
char esp_read_char(UART_HandleTypeDef* handler);
uint8_t esp_send_cmd(UART_HandleTypeDef* uart, char* command);
void esp_send_data(UART_HandleTypeDef* uart, char* content);
