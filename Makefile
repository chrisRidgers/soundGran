INCLUDE= -I include/
LIB = -L lib/


default:
	make -C portsf install
	make -C portsf/ veryclean
	gcc -c -o v6c13.o $(INCLUDE) v6c13.c
	gcc $(LIB) -o v6c13.out v6c13.o  lib/libportsf.a
