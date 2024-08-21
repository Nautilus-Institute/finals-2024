
all: libkvdb.a

libkvdb.a: kv.o kv_list.o sha256.o
	ar rcs libkvdb.a kv.o kv_list.o sha256.o

kv_list.o: kv_list.c kv_list.h
	gcc -O2 -fno-optimize-sibling-calls -fno-builtin kv_list.c -c -o kv_list.o

kv.o: kv.c kv.h
	gcc -O2 -fno-builtin kv.c -c -o kv.o

sha256.o: sha256.c sha256.h
	gcc -O2 -fno-builtin sha256.c -c -o sha256.o

clean:
	rm -f libkvdb.a kv.o kv_list.o

