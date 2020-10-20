################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ArcFitsFileCAPI.cpp \
../src/CArcFitsFile.cpp \
../src/CArcFitsFileDllMain.cpp 

OBJS += \
./src/ArcFitsFileCAPI.o \
./src/CArcFitsFile.o \
./src/CArcFitsFileDllMain.o 

CPP_DEPS += \
./src/ArcFitsFileCAPI.d \
./src/CArcFitsFile.d \
./src/CArcFitsFileDllMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I"/home/streit/Documents/ARC_API/3.6"/CArcBase/inc -I/home/streit/Documents/ARC_API/3.6/cfitsio-3450/include -I"/home/streit/Documents/ARC_API/3.6/CArcFitsFile/inc" -O0 -Wall -c -fmessage-length=0 -D_REENTRANT -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


