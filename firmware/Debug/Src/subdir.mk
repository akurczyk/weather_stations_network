################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/bme280.c \
../Src/bmp180.c \
../Src/data_buffer.c \
../Src/esp.c \
../Src/freertos.c \
../Src/gpio.c \
../Src/gps.c \
../Src/i2c.c \
../Src/main.c \
../Src/stm32f4xx_hal_msp.c \
../Src/stm32f4xx_hal_timebase_TIM.c \
../Src/stm32f4xx_it.c \
../Src/system_stm32f4xx.c \
../Src/usart.c 

OBJS += \
./Src/bme280.o \
./Src/bmp180.o \
./Src/data_buffer.o \
./Src/esp.o \
./Src/freertos.o \
./Src/gpio.o \
./Src/gps.o \
./Src/i2c.o \
./Src/main.o \
./Src/stm32f4xx_hal_msp.o \
./Src/stm32f4xx_hal_timebase_TIM.o \
./Src/stm32f4xx_it.o \
./Src/system_stm32f4xx.o \
./Src/usart.o 

C_DEPS += \
./Src/bme280.d \
./Src/bmp180.d \
./Src/data_buffer.d \
./Src/esp.d \
./Src/freertos.d \
./Src/gpio.d \
./Src/gps.d \
./Src/i2c.d \
./Src/main.d \
./Src/stm32f4xx_hal_msp.d \
./Src/stm32f4xx_hal_timebase_TIM.d \
./Src/stm32f4xx_it.d \
./Src/system_stm32f4xx.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32F411xE -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Inc" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Drivers/STM32F4xx_HAL_Driver/Inc" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Drivers/CMSIS/Include" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Inc" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Middlewares/Third_Party/FreeRTOS/Source/include" -I"C:/Users/olo/Desktop/weather_stations_network/firmware/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


