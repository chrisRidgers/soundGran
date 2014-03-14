COMPILE= -c -I include/
LIB = -L lib/
LIBS= -lportsf
GCC=gcc


default: lib/libportsf.a v6c13.o v6c13.out

v6c13.out: v6c13.o
	$(GCC) $(LIB) $(LIBS) -o v6c13.out v6c13.o 

v6c13.o: v6c13.c
	$(GCC) $(COMPILE) -o v6c13.o v6c13.c 

lib/libportsf.a:
	make -C portsf install
	make -C portsf/ veryclean
clean:
	rm *.o

veryclean: clean
	rm lib/libportsf.a
