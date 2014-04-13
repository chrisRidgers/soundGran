COMPILE= --std=c99 -c -I include/
LIB = -Llib/
LIBS= -lportsf -lm
GCC=gcc


default: lib/libportsf.a v6c13.o v6c13.out
defaultb: lib/libportsf.a v6c13b.o v6c13b.out

debug:COMPILE+= -g
debug: lib/libportsf.a v6c13.o v6c13.out

debugb:COMPILE+= -g
debugb: lib/libportsf.a v6c13b.o v6c13b.out

v6c13.out: v6c13.o
	$(GCC) v6c13.o $(LIB) $(LIBS) -o v6c13.out  

v6c13.o: v6c13.c
	$(GCC) $(COMPILE) v6c13.c -o v6c13.o 

lib/libportsf.a:
	make -C portsf install
	make -C portsf/ veryclean

v6c13b.out: v6c13b.o
	$(GCC) v6c13b.o $(LIB) $(LIBS) -o v6c13b.out  

v6c13b.o: v6c13b.c
	$(GCC) $(COMPILE) v6c13b.c -o v6c13b.o 
clean:
	rm *.o

veryclean: clean
	rm lib/libportsf.a
