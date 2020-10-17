#lib: NFopen.c NFreloc.c NFmap.c headers/NFlink.h
#	gcc -o libNanoNF.so -shared -fPIC -g NFreloc.c NFmap.c NFopen.c

lib: NFmap.o NFreloc.o NFsym.o NFusage.o NFopen.o NFdeps.o
	gcc -o libNanoNF.so -shared -fPIC -g NFmap.o NFreloc.o NFopen.o NFsym.o NFusage.o NFdeps.o -ldl

NFopen.o: src/NFopen.c
	gcc -fPIC -g -c src/NFopen.c

NFreloc.o: src/NFreloc.c
	gcc -fPIC -g -c src/NFreloc.c

NFmap.o: src/NFmap.c
	gcc -fPIC -g -c src/NFmap.c

NFsym.o: src/NFsym.c
	gcc -fPIC -g -c src/NFsym.c

clean:
	rm *.o *.so

NFusage.o: src/NFusage.c
	gcc -fPIC -g -c src/NFusage.c

NFdeps.o: src/NFdeps.c
	gcc -fPIC -g -c src/NFdeps.c