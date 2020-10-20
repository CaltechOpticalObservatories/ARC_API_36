################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ArcDeviceCAPI.cpp \
../src/ArcOSDefs.cpp \
../src/CArcDevice.cpp \
../src/CArcDeviceDllMain.cpp \
../src/CArcLog.cpp \
../src/CArcPCI.cpp \
../src/CArcPCIBase.cpp \
../src/CArcPCIe.cpp \
../src/TempCtrl.cpp 

OBJS += \
./src/ArcDeviceCAPI.o \
./src/ArcOSDefs.o \
./src/CArcDevice.o \
./src/CArcDeviceDllMain.o \
./src/CArcLog.o \
./src/CArcPCI.o \
./src/CArcPCIBase.o \
./src/CArcPCIe.o \
./src/TempCtrl.o 

CPP_DEPS += \
./src/ArcDeviceCAPI.d \
./src/ArcOSDefs.d \
./src/CArcDevice.d \
./src/CArcDeviceDllMain.d \
./src/CArcLog.d \
./src/CArcPCI.d \
./src/CArcPCIBase.d \
./src/CArcPCIe.d \
./src/TempCtrl.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I"/home/streit/Documents/ARC_API/3.6/CArcBase/inc" -I"/home/streit/Documents/ARC_API/3.6/CArcDevice/inc" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


