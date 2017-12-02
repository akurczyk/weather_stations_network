#include "esp.h"

void esp_write_line(UART_HandleTypeDef* handler, char* text)
{
	HAL_UART_Transmit(handler, text, strlen(text), 1000);
	HAL_UART_Transmit(handler, "\r\n", 2, 100);
}

void esp_read_line(UART_HandleTypeDef* handler, char* buffer, uint16_t buffer_size)
{
	HAL_StatusTypeDef status;
	char current_char;
	uint16_t char_counter = 0;

	while (char_counter < buffer_size - 1)
	{
		status = HAL_UART_Receive(handler, &current_char, 1, 1);
		if (status == HAL_OK)
		{
			if (current_char == '\r' || current_char == '\n')
				if (char_counter == 0) continue;
				else break;
			*(buffer + char_counter++) = current_char;
		}
	}

	*(buffer + char_counter) = '\0';
}

char esp_read_char(UART_HandleTypeDef* handler)
{
	char buffer = '\0';
	HAL_UART_Receive(handler, &buffer, 1, 1000);
	return buffer;
}

uint8_t esp_send_cmd(UART_HandleTypeDef* uart, char* command)
{
	char response[30];
	response[0] = '\0';

	esp_write_line(uart, command);
	__HAL_UART_FLUSH_DRREGISTER(&huart1);

	while (strcmp(response, "OK") != 0
			&& strcmp(response, "ready") != 0
			&& strcmp(response, "ERROR") != 0)
		esp_read_line(uart, response, 30);

	if (strcmp(response, "ERROR") == 0) return 0;
	else return 1;
}

void esp_send_data(UART_HandleTypeDef* uart, char* content)
{
	char cmd[17];
	sprintf(cmd, "AT+CIPSEND=%d", strlen(content));
	esp_write_line(uart, cmd);
	osDelay(50);
	HAL_UART_Transmit(uart, content, strlen(content), 5000);

	char response[30];
	response[0] = '\0';

	__HAL_UART_FLUSH_DRREGISTER(&huart1);

	while (strcmp(response, "OK") != 0
			&& strcmp(response, "SEND OK") != 0
			&& strcmp(response, "SEND FAIL") != 0)
		esp_read_line(uart, response, 30);
}
