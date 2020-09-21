#lib: NFopen.c NFreloc.c NFmap.c headers/NFlink.h
#	gcc -o libNanoNF.so -shared -fPIC -g NFreloc.c NFmap.c NFopen.c

lib: NFmap.o NFreloc.o NFsym.o NFopen.o
	gcc -o libNanoNF.so -shared -fPIC -g NFmap.o NFreloc.o NFopen.o NFsym.o

NFopen.o: NFopen.c
	gcc -fPIC -c NFopen.c

NFreloc.o: NFreloc.c
	gcc -fPIC -c NFreloc.c

NFmap.o: NFmap.c
	gcc -fPIC -c NFmap.c

NFsym.o: NFsym.c
	gcc -fPIC -c NFsym.c

clean:
	rm *.o *.so