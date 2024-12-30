include ../config.mk
include ../server.mk

#CROSS_COMPILE=
#CC=$(CROSS_COMPILE)gcc
#CXX=$(CROSS_COMPILE)g++
#AR=$(CROSS_COMPILE)ar
#RANLIB= $(CROSS_COoMPILE)ranlib
#STRIP=$(CROSS_COMPILE)strip
#
#CFLAGS = -Wall 
#LDFLAGS = -ldl -lm -lc -lpthread 
#OBJS =   index.o cJSON.o http.o server.o socket.o main.o
#
#all: http_server
#
#%.o: %.c
#	$(CC) $(CFLAGS) -c -o $@ $<
#
#http_server: $(OBJS)
#	$(CC) -o $@ $(OBJS) $(LDFLAGS) 
#
#clean:
#	rm -f rbcfg *.o


TARGET = libserver.a

#main.o
all : index.o cJSON.o http.o server.o socket.o 

	$(AR) -r $(TARGET) *.o
	cp $(TARGET) $(MYLIB)

%.o: %.c
	$(CC) -c -fPIE $(CFLAGS) -o $@ $<

clean :
	/bin/rm -f *.o
	/bin/rm -f $(TARGET)
    
    