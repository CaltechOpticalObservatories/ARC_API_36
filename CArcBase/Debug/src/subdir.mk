################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CArcBase.cpp \
../src/CArcBaseDllMain.cpp \
../src/CArcStringList.cpp 

OBJS += \
./src/CArcBase.o \
./src/CArcBaseDllMain.o \
./src/CArcStringList.o 

CPP_DEPS += \
./src/CArcBase.d \
./src/CArcBaseDllMain.d \
./src/CArcStringList.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I"/home/streit/Documents/ARC_API/3.6/CArcBase/inc" -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


