################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/matt/Dropbox/Projects/workspace/sensornet/sensornet.c 

OBJS += \
./sensornet/sensornet.o 

C_DEPS += \
./sensornet/sensornet.d 


# Each subdirectory must supply rules for building sources it contributes
sensornet/sensornet.o: /home/matt/Dropbox/Projects/workspace/sensornet/sensornet.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-gcc -I"/home/matt/Dropbox/Projects/workspace/rf-node-tx-simple" -I"/home/matt/Dropbox/Projects/workspace/sensornet" -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -std=gnu99 -funsigned-char -funsigned-bitfields -mmcu=attiny85 -DF_CPU=8000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


