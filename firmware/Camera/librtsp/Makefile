include ../config.mk
include ../server.mk


TARGET = librtsp.a

all : rtsp_demo.o rtsp_msg.o rtp_enc.o stream_queue.o utils.o 

	$(AR) -r $(TARGET) *.o
	cp $(TARGET) $(MYLIB)

%.o: %.c
	$(CC) -c -fPIE $(CFLAGS) -o $@ $<

clean :
	/bin/rm -f *.o
	/bin/rm -f $(TARGET)
