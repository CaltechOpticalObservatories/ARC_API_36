# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ArcDeinterlaceCAPI.cpp \
../src/CArcDeinterlace.cpp \
../src/CArcDeinterlaceDllMain.cpp \
../src/CArcPluginManager.cpp \
../src/IArcPlugin.cpp 

OBJS += \
./src/ArcDeinterlaceCAPI.o \
./src/CArcDeinterlace.o \
./src/CArcDeinterlaceDllMain.o \
./src/CArcPluginManager.o \
./src/IArcPlugin.o 

CPP_DEPS += \
./src/ArcDeinterlaceCAPI.d \
./src/CArcDeinterlace.d \
./src/CArcDeinterlaceDllMain.d \
./src/CArcPluginManager.d \
./src/IArcPlugin.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I"/opt/ARC_API/3.6/CArcDeinterlace/inc" -I"/opt/ARC_API/3.6/CArcBase/inc" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


