include ../config.mk
include ../server.mk

TARGET = libae_log.a

all : debug.o

	$(AR) -r $(TARGET) *.o
	cp $(TARGET) $(MYLIB)

%.o: %.c

	$(CC) -c -fPIE $(CFLAGS) -o $@ $<


clean :
	/bin/rm -f *.o
	/bin/rm -f $(TARGET)
