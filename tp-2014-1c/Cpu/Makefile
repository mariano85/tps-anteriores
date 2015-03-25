CC := gcc
CFLAGS := -Wall
INCLUDES := -I "../"
LIBRARIES := -lcommons -lpthread -lparser
PREPROCESOR := -DCOMPILAR_DESDE_CONSOLA

OUT := Cpu
MCPU := Cpu.o Primitivas.o
MODULOS = Cpu.o Primitivas.o

# Compilar Cpu
Cpu: $(MCPU)
	$(CC) $(PREPROCESOR) -g $(CFLAGS) $(INCLUDES) $(MCPU) $(LIBRARIES) -o $(OUT) 

Cpu.o: Cpu.c
	$(CC) $(PREPROCESOR) $(CFLAGS) $(INCLUDES) -c Cpu.c $(LIBRARIES)
	
Primitivas.o: Primitivas.c
	$(CC) $(PREPROCESOR) -c $(CFLAGS) $(INCLUDES) Primitivas.c $(LIBRARIES)

# All
all: clean Cpu

# Clean
clean:
	 rm -f $(MODULOS) $(OUT)

