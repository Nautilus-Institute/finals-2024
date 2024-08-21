SOURCE_FILES :=  \
								NautilusRand.c \
								sha512.c \
								WELL512a.c \
								main.cpp
HEADER_FILES := NautilusRand.h \
								sha512.h \
								WELL512a.h

all: sokoban sokoban_d

sokoban: ${SOURCE_FILES} \
				 ${HEADER_FILES}
	g++ ${SOURCE_FILES} -O2 -Wno-unused-result -Wl,-z,norelro -fno-builtin -o sokoban
	strip --strip-all sokoban

sokoban_d: ${SOURCE_FILES} \
					 ${HEADER_FILES}
	g++ ${SOURCE_FILES} -DDEBUG -g -Wl,-z,norelro -fno-builtin -o sokoban_d

clean:
	rm -f sokoban sokoban_d

