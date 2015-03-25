CC := gcc
FLAGS := -w -fPIC -lcommons
INCLUDES := -I "../"
LIB := libparser
PREPROCESOR := -DCOMPILAR_DESDE_CONSOLA

shared:

	@echo 'Creo $(LIB).so, luego borro los .o generados'
	$(CC) $(PREPROCESOR) $(FLAGS) $(INCLUDES) -c *.c
	$(CC) $(PREPROCESOR) -shared -Wl,-soname,$(LIB).so.1 -o $(LIB).so *.o -lc
	rm -f *.o
	
	@echo 'Muevo $(LIB).so a /lib/$(LIB).so y la linkeo para que se pueda compartir'
	sudo mv $(LIB).so /lib/
	sudo ln -sf /lib/$(LIB).so /lib/$(LIB).so.1
	ldconfig -n /lib
	
all: 
	shared
