################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../rf-node/rf-node.c \
../rf-node/vwire.c 

OBJS += \
./rf-node/rf-node.o \
./rf-node/vwire.o 

C_DEPS += \
./rf-node/rf-node.d \
./rf-node/vwire.d 


# Each subdirectory must supply rules for building sources it contributes
rf-node/%.o: ../rf-node/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=attiny85 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


