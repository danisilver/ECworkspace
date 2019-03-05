################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../parte1/D8Led.c \
../parte1/button.c \
../parte1/gpio.c \
../parte1/intcontroller.c \
../parte1/keyboard.c \
../parte1/leds.c \
../parte1/main.c \
../parte1/timer.c \
../parte1/utils.c 

S_UPPER_SRCS += \
../parte1/init.S 

OBJS += \
./parte1/D8Led.o \
./parte1/button.o \
./parte1/gpio.o \
./parte1/init.o \
./parte1/intcontroller.o \
./parte1/keyboard.o \
./parte1/leds.o \
./parte1/main.o \
./parte1/timer.o \
./parte1/utils.o 

C_DEPS += \
./parte1/D8Led.d \
./parte1/button.d \
./parte1/gpio.d \
./parte1/intcontroller.d \
./parte1/keyboard.d \
./parte1/leds.d \
./parte1/main.d \
./parte1/timer.d \
./parte1/utils.d 

S_UPPER_DEPS += \
./parte1/init.d 


# Each subdirectory must supply rules for building sources it contributes
parte1/%.o: ../parte1/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC C Compiler'
	arm-none-eabi-gcc -O0 -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -mapcs-frame -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

parte1/%.o: ../parte1/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Sourcery Windows GCC Assembler'
	arm-none-eabi-gcc -x assembler-with-cpp -Wall -Wa,-adhlns="$@.lst" -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -mcpu=arm7tdmi -g3 -gdwarf-2 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


