################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/Users/DW/Documents/Arduino/libraries/DateTime/DateTime.cpp 

LINK_OBJ += \
./libraries/DateTime/DateTime.cpp.o 

CPP_DEPS += \
./libraries/DateTime/DateTime.cpp.d 


# Each subdirectory must supply rules for building sources it contributes
libraries/DateTime/DateTime.cpp.o: /Users/DW/Documents/Arduino/libraries/DateTime/DateTime.cpp
	@echo 'Building file: $<'
	@echo 'Starting C++ compile'
	"/Applications/sloeber.app/Contents/Eclipse//arduinoPlugin/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1/bin/arm-none-eabi-g++" -mcpu=cortex-m0plus -mthumb -c -g -Os -Wall -Wextra -std=gnu++11 -ffunction-sections -fdata-sections -fno-threadsafe-statics -nostdlib --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -DF_CPU=48000000L -DARDUINO=10802 -DARDUINO_SAMD_FEATHER_M0 -DARDUINO_ARCH_SAMD -DARDUINO_SAMD_ZERO -D__SAMD21G18A__ -DUSB_VID=0x239A -DUSB_PID=0x800B -DUSBCON '-DUSB_MANUFACTURER="Adafruit"' '-DUSB_PRODUCT="Feather M0"' "-I/Applications/sloeber.app/Contents/Eclipse//arduinoPlugin/packages/arduino/tools/CMSIS/4.5.0/CMSIS/Include/" "-I/Applications/sloeber.app/Contents/Eclipse//arduinoPlugin/packages/arduino/tools/CMSIS-Atmel/1.1.0/CMSIS/Device/ATMEL/"  -I"/Applications/sloeber.app/Contents/Eclipse/arduinoPlugin/packages/adafruit/hardware/samd/1.0.17/cores/arduino" -I"/Applications/sloeber.app/Contents/Eclipse/arduinoPlugin/packages/adafruit/hardware/samd/1.0.17/variants/arduino_zero" -I"/Applications/sloeber.app/Contents/Eclipse/arduinoPlugin/libraries/SD/1.1.1" -I"/Applications/sloeber.app/Contents/Eclipse/arduinoPlugin/libraries/SD/1.1.1/src" -I"/Applications/sloeber.app/Contents/Eclipse/arduinoPlugin/packages/adafruit/hardware/samd/1.0.17/libraries/SPI" -I"/Applications/sloeber.app/Contents/Eclipse/arduinoPlugin/packages/adafruit/hardware/samd/1.0.17/libraries/Wire" -I"/Users/DW/Documents/Arduino/libraries/Adafruit_LIS3DH-master" -I"/Users/DW/Documents/Arduino/libraries/Adafruit_Sensor-master" -I"/Users/DW/Documents/Arduino/libraries/RTCZero" -I"/Users/DW/Documents/Arduino/libraries/RTCZero/src" -I"/Users/DW/Eclipse/libraries/simplot" -I"/Applications/sloeber.app/Contents/Eclipse/arduinoPlugin/libraries/Adafruit_Unified_Sensor/1.0.2" -I"/Users/DW/Documents/Arduino/libraries/DateTime" -I"/Users/DW/Documents/Arduino/libraries/DateTimeStrings" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -D__IN_ECLIPSE__=1 -x c++ "$<"  -o  "$@"
	@echo 'Finished building: $<'
	@echo ' '


