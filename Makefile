INCLUDE= -I include/
LIB = -L lib/


default: lib/libportsf.a v6c13.o v6c13.out

v6c13.out: v6c13.o
	gcc $(LIB) -l portsf -o v6c13.out v6c13.o 

v6c13.o: v6c13.c
	gcc -c $(INCLUDE) v6c13.c -o v6c13.o

lib/libportsf.a:
	make -C portsf install
	make -C portsf/ veryclean
clean:
	rm *.o

veryclean: clean
	rm lib/libportsf.a
