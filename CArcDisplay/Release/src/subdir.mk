# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ArcDisplayCAPI.cpp \
../src/CArcDisplay.cpp \
../src/CArcDisplayDllMain.cpp 

OBJS += \
./src/ArcDisplayCAPI.o \
./src/CArcDisplay.o \
./src/CArcDisplayDllMain.o 

CPP_DEPS += \
./src/ArcDisplayCAPI.d \
./src/CArcDisplay.d \
./src/CArcDisplayDllMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I"/opt/ARC_API/3.6"/CArcBase/inc -I"/opt/ARC_API/3.6/CArcDisplay/inc" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


