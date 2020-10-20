################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ArcDeinterlaceCAPI.cpp \
../src/CArcDeinterlace.cpp \
../src/CArcDeinterlaceDllMain.cpp 

OBJS += \
./src/ArcDeinterlaceCAPI.o \
./src/CArcDeinterlace.o \
./src/CArcDeinterlaceDllMain.o 

CPP_DEPS += \
./src/ArcDeinterlaceCAPI.d \
./src/CArcDeinterlace.d \
./src/CArcDeinterlaceDllMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


