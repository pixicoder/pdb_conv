all:
	gcc $(CFLAGS) -c pdb_conv.c
	gcc $(LDFLAGS) -o pdb_conv pdb_conv.o $(LIBS)

clean:
	rm -f *.o *.so *.dylib pdb_conv
