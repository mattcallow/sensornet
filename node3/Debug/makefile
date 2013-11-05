################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
rf-node-tx-simple.lss \

SIZEDUMMY += \
sizedummy \


# All Target
all: rf-node-tx-simple.elf secondary-outputs

# Tool invocations
rf-node-tx-simple.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C Linker'
	avr-gcc -Wl,-Map,rf-node-tx-simple.map -mmcu=attiny85 -o "rf-node-tx-simple.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

rf-node-tx-simple.lss: rf-node-tx-simple.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S rf-node-tx-simple.elf  >"rf-node-tx-simple.lss"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: rf-node-tx-simple.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=attiny85 rf-node-tx-simple.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(ASM_DEPS)$(ELFS)$(LSS)$(S_DEPS)$(SIZEDUMMY)$(S_UPPER_DEPS) rf-node-tx-simple.elf
	-@echo ' '

secondary-outputs: $(LSS) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
