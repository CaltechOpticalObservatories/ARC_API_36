# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/CArcAPIClient.cpp \
../src/CArcAPIClientDllMain.cpp 

OBJS += \
./src/CArcAPIClient.o \
./src/CArcAPIClientDllMain.o 

CPP_DEPS += \
./src/CArcAPIClient.d \
./src/CArcAPIClientDllMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I"/opt/ARC_API/3.6/CArcAPIClient/inc" -I"/opt/ARC_API/3.6"/CArcBase/inc -I"/opt/ARC_API/3.6/CArcDevice/inc" -I"/opt/ARC_API/3.6"/ArcAPIClientServerCommon -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


