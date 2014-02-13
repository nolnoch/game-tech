################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
ASM_SRCS += \
../media/materials/programs/._Example_FresnelPS.asm \
../media/materials/programs/._OffsetMapping_specular.asm \
../media/materials/programs/Example_FresnelPS.asm \
../media/materials/programs/OffsetMapping_specular.asm 

OBJS += \
./media/materials/programs/._Example_FresnelPS.o \
./media/materials/programs/._OffsetMapping_specular.o \
./media/materials/programs/Example_FresnelPS.o \
./media/materials/programs/OffsetMapping_specular.o 


# Each subdirectory must supply rules for building sources it contributes
media/materials/programs/%.o: ../media/materials/programs/%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	as  -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


