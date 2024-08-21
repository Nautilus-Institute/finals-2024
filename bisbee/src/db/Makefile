
all: libkvdb.a

libkvdb.a: kv.o kv_list.o kv_hashmap.o
	ar rcs libkvdb.a kv.o kv_list.o kv_hashmap.o

kv_list.o: kv_list.c kv_list.h
	gcc -O2 -fno-builtin kv_list.c -c -o kv_list.o

kv_hashmap.o: kv_hashmap.c kv_hashmap.h
	gcc -O2 -fno-builtin kv_hashmap.c -c -o kv_hashmap.o

kv.o: kv.c kv.h
	gcc -O2 -fno-builtin kv.c -c -o kv.o

clean:
	rm -f libkvdb.a kv.o

