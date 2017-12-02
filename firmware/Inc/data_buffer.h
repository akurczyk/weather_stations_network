#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "stdint.h"
#include "stdio.h"
#include "string.h"

#define DB_BUFFER_SIZE 200
#define DB_ENTRY_TEMPLATE "{ \"date_time\": \"%02d-%02d-20%02d %02d:%02d:%02d\", \"latitude\": \"%+6.2f\", \"longitude\": \"%+6.2f\", "\
	                      "\"temperature\": %.1f, \"pressure\": %.0f, \"altitude\": %.0f, \"humidity\": %.2f }\r\n"

typedef struct db_entry {
	uint8_t date_day;
	uint8_t date_mounth;
	uint8_t date_year;
	uint8_t time_hour;
	uint8_t time_min;
	uint8_t time_sec;

	double latitude;
	char latitude_direction;
	double longitude;
	char longitude_direction;

	double temperature;
	double pressure;
	double altitude;
	double humidity;
} db_entry;

typedef struct db_state {
	db_entry data_array[DB_BUFFER_SIZE];
	uint16_t next_id;
	uint8_t full;
	osSemaphoreId semaphore;
} db_state;

db_state db_init();
uint8_t db_add_entry(db_state* state, db_entry entry);
uint8_t db_lock_for_adding(db_state* state);
void db_unlock_for_adding(db_state* state);
void db_read_entry_as_json(db_state* state, uint16_t position, char* output);
void db_reset_counters(db_state* state);
