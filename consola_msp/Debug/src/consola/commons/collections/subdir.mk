################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/consola/commons/collections/dictionary.c \
../src/consola/commons/collections/list.c \
../src/consola/commons/collections/queue.c 

OBJS += \
./src/consola/commons/collections/dictionary.o \
./src/consola/commons/collections/list.o \
./src/consola/commons/collections/queue.o 

C_DEPS += \
./src/consola/commons/collections/dictionary.d \
./src/consola/commons/collections/list.d \
./src/consola/commons/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
src/consola/commons/collections/%.o: ../src/consola/commons/collections/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/alphard/workspace/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


