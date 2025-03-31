include ../config.mk
include ../server.mk

#SHELL = /bin/sh
#
#PREFIX = /usr/local
#
#CFLAGS = -ansi -std=c89 -pedantic -Wall -Wextra \
#	 -O2 -D_POSIX_C_SOURCE=200809L
#CC = cc
## Uncomment to build on Illumos
##CFLAGS = -ansi -std=c89 -pedantic -O2 -Wall -Wextra -lsocket -lsendfile
##CC = gcc

#SRC = src/main.c src/parser.c src/server.c src/sandbox.c src/strlcpy.c \
#      -D_POSIX_C_SOURCE=200809L -DNO_PROXY #-DNO_SANDBOX
#
#upload: src/*
#	${CC} ${SRC} ${CFLAGS} -o $@
#
#install:
#	cp upload ${PREFIX}/bin
#	chmod 755 ${PREFIX}/bin/upload
#
#uninstall:
#	rm ${PREFIX}/bin/upload
#
#clean:
#	rm -f upload
	
	
	
TARGET = libupload.a

#SRC = src/c_upload.c src/parser.c src/server.c src/sandbox.c src/strlcpy.c

all : src/c_upload.o src/parser.o src/server.o src/sandbox.o src/strlcpy.o

	$(AR) -r $(TARGET) src/*.o
	cp $(TARGET) $(MYLIB)

%.o: %.c
	$(CC) -c -fPIE $(CFLAGS) -o $@ $<

clean :
	/bin/rm -f src/*.o
	/bin/rm -f $(TARGET)

