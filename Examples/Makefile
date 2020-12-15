sample: ori-malloc.o
	gcc -o ori-malloc.out ori-malloc.o -L.. -Wl,-rpath,.. -lNanoNF

ori-malloc.o: ori-malloc.c
	gcc -c ori-malloc.c -g

lib: lib1.c
	gcc -shared -fPIC -o lib1.so lib1.c -g

clean:
	rm *.o *.so