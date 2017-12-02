#include "bmp180.h"

bmp_state bmp_init(I2C_HandleTypeDef* i2c)
{
	bmp_state state;
	state.i2c = i2c;
	state.AC1 = 0;
	state.AC2 = 0;
	state.AC3 = 0;
	state.AC4 = 0;
	state.AC5 = 0;
	state.AC6 = 0;
	state.B1 = 0;
	state.B2 = 0;
	state.MB = 0;
	state.MC = 0;
	state.MD = 0;
	state.UT = 0;
	state.UP = 0;
	state.B5 = 0.0;
	return state;
}

uint8_t bmp_read_data(bmp_state* state, uint8_t reg)
{
	uint8_t tmp = 0;
	HAL_I2C_Mem_Read(state->i2c, 0xEF, reg, 1, &tmp, 1, 100);
	return tmp;
}

void bmp_write_data(bmp_state* state, uint8_t reg, uint8_t value)
{
	HAL_I2C_Mem_Write(state->i2c, 0xEE, reg, 1, &value, 1, 100);
}

void bmp_read_compensation_data(bmp_state* state)
{
	state->AC1 = (bmp_read_data(state, 0xAA) << 8) + bmp_read_data(state, 0xAB);
	state->AC2 = (bmp_read_data(state, 0xAC) << 8) + bmp_read_data(state, 0xAD);
	state->AC3 = (bmp_read_data(state, 0xAE) << 8) + bmp_read_data(state, 0xAF);
	state->AC4 = (bmp_read_data(state, 0xB0) << 8) + bmp_read_data(state, 0xB1);
	state->AC5 = (bmp_read_data(state, 0xB2) << 8) + bmp_read_data(state, 0xB3);
	state->AC6 = (bmp_read_data(state, 0xB4) << 8) + bmp_read_data(state, 0xB5);
	state->B1 = (bmp_read_data(state, 0xB6) << 8) + bmp_read_data(state, 0xB7);
	state->B2 = (bmp_read_data(state, 0xB8) << 8) + bmp_read_data(state, 0xB9);
	state->MB = (bmp_read_data(state, 0xBA) << 8) + bmp_read_data(state, 0xBB);
	state->MC = (bmp_read_data(state, 0xBC) << 8) + bmp_read_data(state, 0xBD);
	state->MD = (bmp_read_data(state, 0xBE) << 8) + bmp_read_data(state, 0xBF);
}

void bmp_read_temp_and_pressure(bmp_state* state)
{
	bmp_write_data(state, 0xF4, 0x2E);
	osDelay(5);
	state->UT = (bmp_read_data(state, 0xF6) << 8) + bmp_read_data(state, 0xF7);

	uint8_t OSS = 3; // "ultra high resolution"
	bmp_write_data(state, 0xF4, 0x34 + (OSS << 6));
	if(OSS == 0) osDelay(5);
	else if(OSS == 1) osDelay(8);
	else if(OSS == 2) osDelay(14);
	else osDelay(26);
	state->UP = ((bmp_read_data(state, 0xF6) << 16) + (bmp_read_data(state, 0xF7) << 8) + bmp_read_data(state, 0xF8)) >> 8-OSS;
}

double bmp_get_temperature(bmp_state* state)
{
	int32_t X1, X2, T;
	X1 = (state->UT - state->AC6) * state->AC5 / 32768;
	X2 = state->MC * 2048 / (X1 + state->MD);
	state->B5 = X1 + X2;
	T = (state->B5 + 8) / 16.0;
	return T / 10.0;
}

double bmp_get_pressure(bmp_state* state)
{
	int32_t B6, X1, X2, X3, B3, P;
	uint32_t B4, B7;
	uint8_t OSS = 3;

	B6 = state->B5 - 4000;
	X1 = (state->B2 * (B6 * B6 / 4096)) / 2048;
	X2 = state->AC2 * B6 / 2048;
	X3 = X1 + X2;
	B3 = (((state->AC1 * 4 + X3) << OSS) + 2) / 4;
	X1 = state->AC3 * B6 / 8192;
	X2 = (state->B1 * (B6 * B6 / 4096)) / 65536;
	X3 = ((X1 + X2) + 2) / 4;

	B4 = state->AC4 * (uint32_t)(X3 + 32768) / 32768;
	B7 = ((uint32_t)state->UP - B3) * (50000 >> OSS);
	if(B7 < 0x80000000) P = (B7 * 2) / B4;
	else P = (B7 / B4) * 2;

	X1 = (P / 256) * (P / 256);
	X1 = (X1 * 3038) / 65536;
	X2 = (-7357 * P) / 65536;
	P = P + (X1 + X2 + 3791) / 16;
	return P / 100.0;
}

double bmp_get_altitude(double p, double p0)
{
	return 44330 * (1 - pow((p/p0), (1/5.255)));
}
