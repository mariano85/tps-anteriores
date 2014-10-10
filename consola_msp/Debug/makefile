################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/consola/commons/collections/subdir.mk
-include src/consola/commons/subdir.mk
-include src/consola/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: consola_msp

dependents:
	-cd /home/alphard/workspace/commons/Debug && $(MAKE) all

# Tool invocations
consola_msp: $(OBJS) $(USER_OBJS) /home/alphard/workspace/commons/Debug/libcommons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -lpthread -pthread -L"/home/alphard/workspace/commons/Debug" -o "consola_msp" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) consola_msp
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/alphard/workspace/commons/Debug/libcommons.so:

-include ../makefile.targets
