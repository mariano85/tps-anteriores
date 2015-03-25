CC := gcc
CFLAGS := -Wall -w 
INCLUDES := -I "../"
LIBRARIES := -lcommons -lparser
OUT := Kernel
PREPROCESOR := -DCOMPILAR_DESDE_CONSOLA

MKERNEL := kernel.o plp.o pcp.o io.o process_queue_manager.o
MODULOS = kernel.o plp.o pcp.o io.o process_queue_manager.o

# All
all: clean kernel
	
# Compilar kernel
kernel: $(MKERNEL)
	$(CC) $(PREPROCESOR) -g $(CFLAGS) $(INCLUDES) $(MKERNEL) $(LIBRARIES) -lpthread -o $(OUT) 
	
kernel.o: kernel.c
	$(CC) $(PREPROCESOR) -c $(CFLAGS) $(INCLUDES) kernel.c $(LIBRARIES)

plp.o: plp.c
	$(CC) $(PREPROCESOR) -c $(CFLAGS) $(INCLUDES) plp.c $(LIBRARIES)
	
pcp.o: pcp.c
	$(CC) $(PREPROCESOR) -c $(CFLAGS) $(INCLUDES) pcp.c $(LIBRARIES)

io.o: io.c
	$(CC) $(PREPROCESOR) -c $(CFLAGS) $(INCLUDES) io.c $(LIBRARIES)

process_queue_manager.o: process_queue_manager.c
	$(CC) $(PREPROCESOR) -c $(CFLAGS) $(INCLUDES) process_queue_manager.c $(LIBRARIES)
	
# Clean
clean:
	rm -f $(MODULOS) $(OUT)
