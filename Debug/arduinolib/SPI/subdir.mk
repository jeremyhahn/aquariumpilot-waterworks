################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../arduinolib/SPI/SPI.cpp 

OBJS += \
./arduinolib/SPI/SPI.o 

CPP_DEPS += \
./arduinolib/SPI/SPI.d 


# Each subdirectory must supply rules for building sources it contributes
arduinolib/SPI/%.o: ../arduinolib/SPI/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I"/storage/apps/eclipse-avr/workspace/ArduinoCore-1.0/src" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/src" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/Ethernet/utility" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/Ethernet" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib/SPI" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/arduinolib" -I"/storage/apps/eclipse-avr/workspace/aquariumpilot-waterworks/lib" -DARDUINO=100 -DDEBUG=true -Wall -g2 -gstabs -O0 -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -fno-exceptions -mmcu=atmega2560 -DF_CPU=1600000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


