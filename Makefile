lib: NFopen.c NFreloc.c NFmap.c headers/NFlink.h
	gcc -o libNanoNF.so -shared -fPIC -g NFreloc.c NFmap.c NFopen.c
