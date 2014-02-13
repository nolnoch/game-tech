################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../OgreApp-MinimalOgre.o \
../OgreApp-SoundManager.o 

CPP_SRCS += \
../BaseApplication.cpp \
../MinimalOgre.cpp \
../SoundManager.cpp \
../TutorialApplication.cpp 

OBJS += \
./BaseApplication.o \
./MinimalOgre.o \
./SoundManager.o \
./TutorialApplication.o 

CPP_DEPS += \
./BaseApplication.d \
./MinimalOgre.d \
./SoundManager.d \
./TutorialApplication.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/usr/include/boost -I/usr/include/OIS -I"/home/nolnoch/ogre3d/ogre_src_v1-7-4/OgreMain/include" -I"/home/nolnoch/ogre3d/ogre_src_v1-7-4/Samples/Common/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


