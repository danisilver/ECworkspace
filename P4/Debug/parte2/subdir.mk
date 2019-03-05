################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../parte2/D8Led.c \
../parte2/button.c \
../parte2/gpio.c \
../parte2/intcontroller.c \
../parte2/keyboard.c \
../parte2/leds.c \
../parte2/main.c \
../parte2/timer.c \
../parte2/uart.c \
../parte2/utils.c 

S_UPPER_SRCS += \
../parte2/init.S 

OBJS += \
./parte2/D8Led.o \
./parte2/button.o \
./parte2/gpio.o \
./parte2/init.o \
./parte2/intcontroller.o \
./parte2/keyboard.o \
./parte2/leds.o \
./parte2/main.o \
./parte2/timer.o \
./parte2/uart.o \
./parte2/utils.o 

C_DEPS += \
./parte2/D8Led.d \
./parte2/button.d \
./parte2/gpio.d \
./parte2/intcontroller.d \
./parte2/keyboard.d \
./parte2/leds.d \
./parte2/main.d \
./parte2/timer.d \
./parte2/uart.d \
./parte2/utils.d 

S_UPPER_DEPS += \
./parte2/init.d 


# Each subdirectory must supply rules for building sources it contributes
parte2/%.o: ../parte2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

parte2/%.o: ../parte2/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


