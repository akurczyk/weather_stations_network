#include "FreeRTOS.h"
#include "i2c.h"

typedef struct bme_state {
	I2C_HandleTypeDef* i2c;
    uint16_t T1;
    int16_t T2;
    int16_t T3;
    uint16_t P1;
    int16_t P2;
    int16_t P3;
    int16_t P4;
    int16_t P5;
    int16_t P6;
    int16_t P7;
    int16_t P8;
    int16_t P9;
    uint8_t H1;
    int16_t H2;
    uint8_t H3;
    int16_t H4;
    int16_t H5;
    int8_t H6;
    int32_t adc_T;
    int32_t adc_P;
    int32_t adc_H;
    int32_t t_fine;
} bme_state;

bme_state bme_init(I2C_HandleTypeDef* i2c);
uint8_t bme_read_data(bme_state* state, uint8_t reg);
void bme_write_data(bme_state* state, uint8_t reg, uint8_t value);
uint8_t bme_setup(bme_state* state);
void bme_read_compensation_data(bme_state* state);
void bme_set_sampling(bme_state* state);
void bme_read_temp_press_and_hum(bme_state* state);
double bme_get_temperature(bme_state* state);
double bme_get_pressure(bme_state* state);
double bme_get_humidity(bme_state* state);
double bme_get_altitude(double p, double p0);
