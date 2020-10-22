# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/ArcImageCAPI.cpp \
../src/CArcImage.cpp \
../src/CArcImageDllMain.cpp 

OBJS += \
./src/ArcImageCAPI.o \
./src/CArcImage.o \
./src/CArcImageDllMain.o 

CPP_DEPS += \
./src/ArcImageCAPI.d \
./src/CArcImage.d \
./src/CArcImageDllMain.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++17 -I"/opt/ARC_API/3.6"/CArcBase/inc -I"/opt/ARC_API/3.6/CArcImage/inc" -O3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


