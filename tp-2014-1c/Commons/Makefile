CC := gcc
FLAGS := -Wall -fPIC
LIB := libcommons

# sharedlibrary de la catedra
shared:

	@echo 'Creo $(LIB).so, luego borro los .o generados'
	$(CC) $(FLAGS) -c commons/*.c commons/collections/*.c
	$(CC) -shared -Wl,-soname,$(LIB).so.1 -o $(LIB).so *.o -lc
	rm -f *.o
	
	@echo 'Muevo $(LIB).so a /lib/$(LIB).so y la linkeo para que se pueda compartir'
	sudo mv $(LIB).so /lib/
	sudo ln -sf /lib/$(LIB).so /lib/$(LIB).so.1
	ldconfig -n /lib
	
all: 
	shared
