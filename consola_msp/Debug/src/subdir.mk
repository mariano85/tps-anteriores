################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/llamada_de_prueba.c \
../src/msp_de_mentiritas.c 

OBJS += \
./src/llamada_de_prueba.o \
./src/msp_de_mentiritas.o 

C_DEPS += \
./src/llamada_de_prueba.d \
./src/msp_de_mentiritas.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/alphard/workspace/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


