CC := gcc
CFLAGS := -w
INCLUDES := -I "../"
LIBRARIES := -lcommons -lpthread
PREPROCESOR := -DCOMPILAR_DESDE_CONSOLA

MUMV := UMV.o
MODULOS = UMV.o

# Compilar UMV
UMV: $(MUMV)
	$(CC) $(PREPROCESOR) -g $(CFLAGS) $(INCLUDES) $(MUMV) $(LIBRARIES) -o UMV 
	chmod 777 *

UMV.o: UMV.c
	$(CC) $(PREPROCESOR) $(CFLAGS) $(INCLUDES) -c UMV.c $(LIBRARIES)
	
# All
all: clean UMV

# Clean
clean:
	 rm -f $(MODULOS)
