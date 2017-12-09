#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "stdarg.h"
#include "usart.h"
#include "bme280.h"
#include "gps.h"
#include "data_buffer.h"
#include "esp.h"

//#define DEBUG
#define BME_I2C_INST    &hi2c1
#define DEBUG_UART_INST &huart2
#define GPS_UART_INST   &huart1
#define ESP_UART_INST   &huart2

#define WIFI_NAME "BRAMA"
#define WIFI_PASS "zaq1@WSX"

db_state db;
volatile gps_state gps;
volatile uint8_t recv_char;
osThreadId sensorsTaskHandle;
osThreadId gpsRecvTaskHandle;
osThreadId wifiTaskHandle;

void MX_FREERTOS_Init(void);
void StartSensorsTask(void const* argument);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* uart);
void StartWifiTask(void const* argument);
void print_dbg(char* template, ...);

void MX_FREERTOS_Init(void) {
	db = db_init();

	gps = gps_init(GPS_UART_INST);
	HAL_UART_Receive_IT(gps.uart, &recv_char, 1);

	osThreadDef(sensorsTask, StartSensorsTask, osPriorityNormal, 0, 1024);
	sensorsTaskHandle = osThreadCreate(osThread(sensorsTask), NULL);

	osThreadDef(wifiTask, StartWifiTask, osPriorityNormal, 0, 1024);
	wifiTaskHandle = osThreadCreate(osThread(wifiTask), NULL);
}

void StartSensorsTask(void const* argument)
{
	//bmp_state bmp = bmp_init(BMP_I2C_INST);
	//bmp_read_compensation_data(&bmp);

	bme_state bme = bme_init(BME_I2C_INST);
	bme_setup(&bme);
	osDelay(10*1000);

	for(;;)
	{
		db_entry entry;

		//bmp_read_temp_and_pressure(&bmp);
		//entry.temperature = bmp_get_temperature(&bmp);
		//entry.pressure = bmp_get_pressure(&bmp);
		//entry.altitude = bmp_get_altitude(entry.pressure, 1013.25);
		//entry.humidity = 0.5;

		bme_read_temp_press_and_hum(&bme);
		entry.temperature = bme_get_temperature(&bme);
		entry.pressure = bme_get_pressure(&bme);
		entry.altitude = bme_get_altitude(entry.pressure, 1013.25);
		entry.humidity = bme_get_humidity(&bme);

		entry.date_day = gps.date_day;
		entry.date_mounth = gps.date_mounth;
		entry.date_year = gps.date_year;
		entry.time_hour = gps.time_hour;
		entry.time_min = gps.time_min;
		entry.time_sec = gps.time_sec;
		entry.latitude = gps.latitude;
		entry.latitude_direction = gps.latitude_direction;
		entry.longitude = gps.longitude;
		entry.longitude_direction = gps.longitude_direction;

#ifdef DEBUG
		print_dbg("Temperature: %+05.1f *C\r\n", entry.temperature);
		print_dbg("Pressure:    %04.0f hPa\r\n", entry.pressure);
		print_dbg("Altitude:    %+07.2f m n.p.m.\r\n", entry.altitude);
		print_dbg("Humidity:    %03.2f%%\r\n", entry.humidity);
		print_dbg("Date:        %02d-%02d-20%02d\r\n", entry.date_day, entry.date_mounth, entry.date_year);
		print_dbg("Time:        %02d:%02d:%02d\r\n", entry.time_hour, entry.time_min, entry.time_sec);
		print_dbg("Latitude:    %f %c\r\n", entry.latitude, entry.latitude_direction);
		print_dbg("Longitude:   %f %c\r\n", entry.longitude, entry.longitude_direction);
		print_dbg("\r\n");
#endif

		db_add_entry(&db, entry);

		osDelay(60*1000);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* uart)
{
	if (uart == gps.uart) {
		gps_recv_char(&gps, recv_char);
		HAL_UART_Receive_IT(gps.uart, &recv_char, 1);
	}
}

void StartWifiTask(void const* argument)
{
	osDelay(2*1000);

	char output[300];
	output[0] = "\0";

	esp_send_cmd(ESP_UART_INST, "AT+RST");
	esp_send_cmd(ESP_UART_INST, "AT+CWMODE_CUR=1");

	for(;;)
	{
		if(db.full == 1 || db.next_id != 0)
		{

			if(db_lock_for_adding(&db))
			{
				char conn_str[100];
				sprintf(conn_str, "AT+CWJAP_CUR=\"%s\",\"%s\"", WIFI_NAME, WIFI_PASS);

				reconnect:
				if(!esp_send_cmd(ESP_UART_INST, conn_str)) goto reconnect;
				if(!esp_send_cmd(ESP_UART_INST, "AT+CIPMUX=0")) goto reconnect;

				for(uint16_t i = 0; i < db.next_id || (db.full == 1 && i < DB_BUFFER_SIZE); i++)
				{
					db_read_entry_as_json(&db, i, &output);

					if(!esp_send_cmd(ESP_UART_INST, "AT+CIPSTART=\"TCP\",\"192.168.12.1\",8080")) goto reconnect;

					char content_length_str[100];
					sprintf(content_length_str, "Content-Length: %d\r\n", strlen(output)+strlen("json_data="));

					esp_send_data(ESP_UART_INST, "POST /recv_data HTTP/1.1\r\n");
					esp_send_data(ESP_UART_INST, "Host: 192.168.12.1:8080\r\n");
					esp_send_data(ESP_UART_INST, "Connection: close\r\n");
					esp_send_data(ESP_UART_INST, "Content-Type: application/x-www-form-urlencoded\r\n");
					esp_send_data(ESP_UART_INST, content_length_str);
					esp_send_data(ESP_UART_INST, "\r\n");
					esp_send_data(ESP_UART_INST, "json_data=");
					esp_send_data(ESP_UART_INST, output);
					esp_send_data(ESP_UART_INST, "\r\n");

					osDelay(2000);
					esp_send_cmd(ESP_UART_INST, "AT+CIPCLOSE");

					//db_read_entry_as_json(&db, i, &output);
					//print_dbg(&output);
				}

				//print_dbg("\r\n");

				db_reset_counters(&db);
				db_unlock_for_adding(&db);
			}
		}

		osDelay(2000);
	}
}

void print_dbg(char* template, ...)
{
    va_list args;
    va_start(args, template);

    char buf[300];
    vsprintf(buf, template, args);
    HAL_UART_Transmit(DEBUG_UART_INST, buf, strlen(buf), 100);

    va_end(args);
}
