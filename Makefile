nix:
	gcc $(CFLAGS) -c pdb_conv.c
	gcc $(LDFLAGS) -o pdb_conv pdb_conv.o $(LIBS)

win32:
	i686-w64-mingw32-gcc $(CFLAGS) -c pdb_conv.c
	i686-w64-mingw32-gcc $(LDFLAGS) -o pdb_conv.exe pdb_conv.o $(LIBS)

win64:
	x86_64-w64-mingw32-gcc $(CFLAGS) -c pdb_conv.c
	x86_64-w64-mingw32-gcc $(LDFLAGS) -o pdb_conv.exe pdb_conv.o $(LIBS)

clean:
	rm -f *.o *.so *.dylib pdb_conv
