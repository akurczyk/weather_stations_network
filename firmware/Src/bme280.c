#include "bme280.h"

bme_state bme_init(I2C_HandleTypeDef* i2c)
{
	bme_state state;
	state.i2c = i2c;
	state.T1 = 0;
	state.T2 = 0;
	state.T3 = 0;
	state.P1 = 0;
	state.P2 = 0;
	state.P3 = 0;
	state.P4 = 0;
	state.P5 = 0;
	state.P6 = 0;
	state.P7 = 0;
	state.P8 = 0;
	state.P9 = 0;
	state.H1 = 0;
	state.H2 = 0;
	state.H3 = 0;
	state.H4 = 0;
	state.H5 = 0;
	state.H6 = 0;
	return state;
}

uint8_t bme_read_data(bme_state* state, uint8_t reg)
{
	uint8_t tmp = 0;
	HAL_I2C_Mem_Read(state->i2c, 0xED, reg, 1, &tmp, 1, 100);
	return tmp;
}

void bme_write_data(bme_state* state, uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(state->i2c, 0xEC, reg, 1, &value, 1, 100);
}

uint8_t bme_setup(bme_state* state)
{
	if(bme_read_data(state, 0xD0) != 0x60) return 0;

	bme_write_data(state, 0xE0, 0xB6);

	osDelay(300);
	while((bme_read_data(state, 0xF3) & 0x01) != 0) osDelay(100);

	bme_read_compensation_data(state);
	bme_set_sampling(state);

	return 1;
}

void bme_read_compensation_data(bme_state* state)
{
	state->T1 = (bme_read_data(state, 0x89) << 8) + bme_read_data(state, 0x88);
	state->T2 = (bme_read_data(state, 0x8B) << 8) + bme_read_data(state, 0x8A);
	state->T3 = (bme_read_data(state, 0x8D) << 8) + bme_read_data(state, 0x8C);

	state->P1 = (bme_read_data(state, 0x8F) << 8) + bme_read_data(state, 0x8E);
    state->P2 = (bme_read_data(state, 0x91) << 8) + bme_read_data(state, 0x90);
    state->P3 = (bme_read_data(state, 0x93) << 8) + bme_read_data(state, 0x92);
    state->P4 = (bme_read_data(state, 0x95) << 8) + bme_read_data(state, 0x94);
    state->P5 = (bme_read_data(state, 0x97) << 8) + bme_read_data(state, 0x96);
    state->P6 = (bme_read_data(state, 0x99) << 8) + bme_read_data(state, 0x98);
    state->P7 = (bme_read_data(state, 0x9B) << 8) + bme_read_data(state, 0x9A);
    state->P8 = (bme_read_data(state, 0x9D) << 8) + bme_read_data(state, 0x9C);
    state->P9 = (bme_read_data(state, 0x9F) << 8) + bme_read_data(state, 0x9E);

    state->H1 = bme_read_data(state, 0xA1);
    state->H2 = (bme_read_data(state, 0xE2) << 8) + bme_read_data(state, 0xE1);
    state->H3 = bme_read_data(state, 0xE3);
    state->H4 = (bme_read_data(state, 0xE4) << 4) | (bme_read_data(state, 0xE5) & 0x0F);
    state->H5 = (bme_read_data(state, 0xE6) << 4) | (bme_read_data(state, 0xE5) >> 4);
    state->H6 = bme_read_data(state, 0xE7);
}

void bme_set_sampling(bme_state* state)
{
	bme_write_data(state, 0xF2, 0x05);
    bme_write_data(state, 0xF5, 0x00);
    bme_write_data(state, 0xF4, 0xB7);
}

void bme_read_temp_press_and_hum(bme_state* state)
{
	state->adc_T = (bme_read_data(state, 0xFA) << 16) + (bme_read_data(state, 0xFB) << 8) + bme_read_data(state, 0xFC);
	state->adc_P = (bme_read_data(state, 0xF7) << 16) + (bme_read_data(state, 0xF8) << 8) + bme_read_data(state, 0xF9);
	state->adc_H = (bme_read_data(state, 0xFD) << 8) + bme_read_data(state, 0xFE);
}

double bme_get_temperature(bme_state* state)
{
	int32_t var1, var2;

	if (state->adc_T == 0x800000) return 0.0;
	state->adc_T >>= 4;

	var1 = ((((state->adc_T >> 3) - ((int32_t)state->T1 << 1))) * ((int32_t)state->T2)) >> 11;
	var2 = (((((state->adc_T >> 4) - ((int32_t)state->T1)) * ((state->adc_T>>4) - ((int32_t)state->T1))) >> 12) * ((int32_t)state->T3)) >> 14;
	state->t_fine = var1 + var2;

	double T = (state->t_fine * 5 + 128) >> 8;
	return T/100;
}

double bme_get_pressure(bme_state* state)
{
	int64_t var1, var2, p;

	if(state->adc_P == 0x800000) return 0.0;
	state->adc_P >>= 4;

	var1 = ((int64_t)state->t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)state->P6;
	var2 = var2 + ((var1*(int64_t)state->P5) << 17);
	var2 = var2 + (((int64_t)state->P4)<<35);
	var1 = ((var1 * var1 * (int64_t)state->P3)>>8) + ((var1 * (int64_t)state->P2)<<12);
	var1 = (((((int64_t)1)<<47)+var1))*((int64_t)state->P1)>>33;

	if(var1 == 0) return 0;
	p = 1048576 - state->adc_P;
	p = (((p<<31) - var2)*3125) / var1;
	var1 = (((int64_t)state->P9) * (p>>13) * (p>>13)) >> 25;
	var2 = (((int64_t)state->P8) * p) >> 19;

	p = ((p + var1 + var2) >> 8) + (((int64_t)state->P7)<<4);
	return ((double)p / 256.0) / 100.0;
}

double bme_get_humidity(bme_state* state)
{
	if(state->adc_H == 0x8000) return 0.0;

	int32_t v_x1_u32r;

	v_x1_u32r = (state->t_fine - ((int32_t)76800));

	v_x1_u32r = (((((state->adc_H << 14) - (((int32_t) state->H4) << 20)
			- (((int32_t) state->H5) * v_x1_u32r)) + ((int32_t) 16384)) >> 15)
			* (((((((v_x1_u32r * ((int32_t) state->H6)) >> 10)
					* (((v_x1_u32r * ((int32_t) state->H3)) >> 11)
							+ ((int32_t) 32768))) >> 10) + ((int32_t) 2097152))
					* ((int32_t) state->H2) + 8192) >> 14));

	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)state->H1)) >> 4));

	v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
	v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;

	float h = (v_x1_u32r >> 12);
	return (h / 1024.0) / 100.0;
}

double bme_get_altitude(double p, double p0)
{
	return 44330 * (1 - pow((p/p0), (1/5.255)));
}
