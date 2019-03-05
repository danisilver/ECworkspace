################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../alumnos/D8Led.c \
../alumnos/button.c \
../alumnos/gpio.c \
../alumnos/intcontroller.c \
../alumnos/keyboard.c \
../alumnos/leds.c \
../alumnos/main.c \
../alumnos/main2.c \
../alumnos/timer.c \
../alumnos/utils.c 

S_UPPER_SRCS += \
../alumnos/init.S 

OBJS += \
./alumnos/D8Led.o \
./alumnos/button.o \
./alumnos/gpio.o \
./alumnos/init.o \
./alumnos/intcontroller.o \
./alumnos/keyboard.o \
./alumnos/leds.o \
./alumnos/main.o \
./alumnos/main2.o \
./alumnos/timer.o \
./alumnos/utils.o 

C_DEPS += \
./alumnos/D8Led.d \
./alumnos/button.d \
./alumnos/gpio.d \
./alumnos/intcontroller.d \
./alumnos/keyboard.d \
./alumnos/leds.d \
./alumnos/main.d \
./alumnos/main2.d \
./alumnos/timer.d \
./alumnos/utils.d 

S_UPPER_DEPS += \
./alumnos/init.d 


# Each subdirectory must supply rules for building sources it contributes
alumnos/%.o: ../alumnos/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

alumnos/%.o: ../alumnos/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


