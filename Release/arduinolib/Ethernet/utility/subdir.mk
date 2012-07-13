################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../arduinolib/Ethernet/utility/socket.cpp \
../arduinolib/Ethernet/utility/w5100.cpp 

OBJS += \
./arduinolib/Ethernet/utility/socket.o \
./arduinolib/Ethernet/utility/w5100.o 

CPP_DEPS += \
./arduinolib/Ethernet/utility/socket.d \
./arduinolib/Ethernet/utility/w5100.d 


# Each subdirectory must supply rules for building sources it contributes
arduinolib/Ethernet/utility/%.o: ../arduinolib/Ethernet/utility/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -Wall -Os -fpack-struct -fshort-enums -funsigned-char -funsigned-bitfields -fno-exceptions -mmcu=atmega2560 -DF_CPU=1600000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


