################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../arduinolib/Wire/utility/twi.c 

OBJS += \
./arduinolib/Wire/utility/twi.o 

C_DEPS += \
./arduinolib/Wire/utility/twi.d 


# Each subdirectory must supply rules for building sources it contributes
arduinolib/Wire/utility/%.o: ../arduinolib/Wire/utility/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: AVR Compiler'
	avr-g++ -I/usr/share/arduino/hardware/arduino/cores/arduino -I/usr/share/arduino/hardware/arduino/variants/mega -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/src" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/Ethernet/utility" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/Ethernet" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/EEPROM" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/Wire/utility" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/Wire" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/SPI" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/lib" -DARDUINO=100 -DDEBUG=true -Wall -g2 -gstabs -O0 -ffunction-sections -fdata-sections -std=gnu99 -mmcu=atmega2560 -DF_CPU=1600000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


