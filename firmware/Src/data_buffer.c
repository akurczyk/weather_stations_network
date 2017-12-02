#include "data_buffer.h"

db_state db_init()
{
	db_state state;

	state.next_id = 0;
	state.full = 0;

	osSemaphoreDef(dbSemaphore);
	state.semaphore = osSemaphoreCreate(osSemaphore(dbSemaphore), 1);

	return state;
}

uint8_t db_add_entry(db_state* state, db_entry entry)
{
	if(osSemaphoreWait(state->semaphore, UINT32_MAX) == osOK)
	{
		memcpy(&(state->data_array[state->next_id++]), &entry, sizeof(db_entry));
		if(state->next_id == DB_BUFFER_SIZE)
		{
			state->next_id = 0;
			state->full = 1;
		}

		osSemaphoreRelease(state->semaphore);
		return 1;
	}
	return 0;
}

uint8_t db_lock_for_adding(db_state* state)
{
	if(osSemaphoreWait(state->semaphore, UINT32_MAX) == osOK)
		return 1;
	else
		return 0;
}

void db_unlock_for_adding(db_state* state)
{
	osSemaphoreRelease(state->semaphore);
}

void db_read_entry_as_json(db_state* state, uint16_t position, char* output)
{
	db_entry entry = state->data_array[position];
	sprintf(output, DB_ENTRY_TEMPLATE, entry.date_day, entry.date_mounth, entry.date_year,
			entry.time_hour, entry.time_min, entry.time_sec, entry.latitude, entry.longitude,
			entry.temperature, entry.pressure, entry.altitude, entry.humidity);
}

void db_reset_counters(db_state* state)
{
	state->next_id = 0;
	state->full = 0;
	osSemaphoreRelease(state->semaphore);
}
