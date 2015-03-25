CC := gcc
CFLAGS := -Wall
PREPROCESOR := -DCOMPILAR_DESDE_CONSOLA
INCLUDES := -I "../"
LIBRARIES := -lcommons -lpthread

MPROGRAMA := Programa.o
MODULOS = Programa.o

# Compilar Programa
personaje: $(MPROGRAMA)
	$(CC) $(PREPROCESOR) -g $(CFLAGS) $(INCLUDES) $(MPROGRAMA) $(LIBRARIES) -o Programa 
	chmod -R 777 *

Programa.o: Programa.c
	$(CC) $(PREPROCESOR) $(CFLAGS) $(INCLUDES) -c Programa.c $(LIBRARIES)
	
# All
all: clean Programa

# Clean
clean:
	 rm -f $(MODULOS)
