CC=gcc
CFLAGS=-Wall -Wextra -Wpedantic -c -Wno-parentheses -fno-strict-aliasing -std=c99
LFLAGS=-L/usr/lib -lcurl -lssl -lcrypto
SRC=$(wildcard *.c)
COMPILE=$(patsubst %.c, %.o, $(SRC))
OBJ=$(wildcard bin/mkcert.o)

OUT=mkcert


all: create_dir $(COMPILE) link

copy_objects:
	mv *.o bin/

create_dir:
	mkdir -p bin/

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

link: copy_objects
	$(CC) $(OBJ) -o bin/$(OUT) $(LFLAGS)

clean:
	rm -f bin/*.*
	rm -f bin/$(OUT)
