################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/consola/commons/bitarray.c \
../src/consola/commons/config.c \
../src/consola/commons/error.c \
../src/consola/commons/log.c \
../src/consola/commons/process.c \
../src/consola/commons/sockets.c \
../src/consola/commons/string.c \
../src/consola/commons/temporal.c \
../src/consola/commons/txt.c 

OBJS += \
./src/consola/commons/bitarray.o \
./src/consola/commons/config.o \
./src/consola/commons/error.o \
./src/consola/commons/log.o \
./src/consola/commons/process.o \
./src/consola/commons/sockets.o \
./src/consola/commons/string.o \
./src/consola/commons/temporal.o \
./src/consola/commons/txt.o 

C_DEPS += \
./src/consola/commons/bitarray.d \
./src/consola/commons/config.d \
./src/consola/commons/error.d \
./src/consola/commons/log.d \
./src/consola/commons/process.d \
./src/consola/commons/sockets.d \
./src/consola/commons/string.d \
./src/consola/commons/temporal.d \
./src/consola/commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
src/consola/commons/%.o: ../src/consola/commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/alphard/workspace/commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


