################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../funciones_manejo_cola.c \
../kernel.c \
../loader.c \
../planificador.c 

OBJS += \
./funciones_manejo_cola.o \
./kernel.o \
./loader.o \
./planificador.o 

C_DEPS += \
./funciones_manejo_cola.d \
./kernel.d \
./loader.d \
./planificador.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/workspace/ansisop-panel" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


