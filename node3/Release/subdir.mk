################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../rf-node-tx-simple.c \
/home/matt/Dropbox/Projects/workspace/sensornet/sensornet.c 

OBJS += \
./rf-node-tx-simple.o \
./sensornet.o 

C_DEPS += \
./rf-node-tx-simple.d \
./sensornet.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -I"/home/matt/Dropbox/Projects/workspace/rf-node-tx-simple/../sensornet" -I"/home/matt/Dropbox/Projects/workspace/rf-node-tx-simple" -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=attiny85 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

sensornet.o: /home/matt/Dropbox/Projects/workspace/sensornet/sensornet.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -I"/home/matt/Dropbox/Projects/workspace/rf-node-tx-simple/../sensornet" -I"/home/matt/Dropbox/Projects/workspace/rf-node-tx-simple" -Wall -Os -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=attiny85 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


