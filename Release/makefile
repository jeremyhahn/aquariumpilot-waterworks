################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include arduinolib/SPI/subdir.mk
-include arduinolib/Ethernet/utility/subdir.mk
-include arduinolib/Ethernet/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
LSS += \
aquariumpilot-waterstation.lss \

FLASH_IMAGE += \
aquariumpilot-waterstation.hex \

EEPROM_IMAGE += \
aquariumpilot-waterstation.eep \

SIZEDUMMY += \
sizedummy \


# All Target
all: aquariumpilot-waterstation.elf secondary-outputs

# Tool invocations
aquariumpilot-waterstation.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: AVR C++ Linker'
	avr-g++ -Wl,-Map,aquariumpilot-waterstation.map,--cref -mmcu=atmega2560 -o "aquariumpilot-waterstation.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

aquariumpilot-waterstation.lss: aquariumpilot-waterstation.elf
	@echo 'Invoking: AVR Create Extended Listing'
	-avr-objdump -h -S aquariumpilot-waterstation.elf  >"aquariumpilot-waterstation.lss"
	@echo 'Finished building: $@'
	@echo ' '

aquariumpilot-waterstation.hex: aquariumpilot-waterstation.elf
	@echo 'Create Flash image (ihex format)'
	-avr-objcopy -R .eeprom -O ihex aquariumpilot-waterstation.elf  "aquariumpilot-waterstation.hex"
	@echo 'Finished building: $@'
	@echo ' '

aquariumpilot-waterstation.eep: aquariumpilot-waterstation.elf
	@echo 'Create eeprom image (ihex format)'
	-avr-objcopy -j .eeprom --no-change-warnings --change-section-lma .eeprom=0 -O ihex aquariumpilot-waterstation.elf  "aquariumpilot-waterstation.eep"
	@echo 'Finished building: $@'
	@echo ' '

sizedummy: aquariumpilot-waterstation.elf
	@echo 'Invoking: Print Size'
	-avr-size --format=avr --mcu=atmega2560 aquariumpilot-waterstation.elf
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EEPROM_IMAGE)$(ELFS)$(FLASH_IMAGE)$(LSS)$(S_DEPS)$(CXX_DEPS)$(S_UPPER_DEPS)$(C++_DEPS)$(ASM_DEPS)$(CC_DEPS)$(CPP_DEPS)$(C_UPPER_DEPS)$(SIZEDUMMY) aquariumpilot-waterstation.elf
	-@echo ' '

secondary-outputs: $(LSS) $(FLASH_IMAGE) $(EEPROM_IMAGE) $(SIZEDUMMY)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
