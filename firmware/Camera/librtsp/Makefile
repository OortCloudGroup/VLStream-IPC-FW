include ../config.mk
include ../server.mk
#	
#	PRO_NAME = librtsp.a
#	PRO_ROOT = $(shell pwd)
#	CC = $(CROSS_COMPILE)gcc
#	AR = $(CROSS_COMPILE)ar
#	INC := -I$(PRO_ROOT)
#	CFLAGS += $(INC)
#	LIB = -lpthread -lrt
#	SRC_DIR := $(PRO_ROOT)
#	LIB_SRC = \
#		$(SRC_DIR)/rtsp_parse.c 		\
#		$(SRC_DIR)/rtsp_handler.c 		\
#		$(SRC_DIR)/rtp.c				\
#		$(SRC_DIR)/h264.c				\
#		$(SRC_DIR)/tcp.c				\
#		$(SRC_DIR)/udp.c				\
#	obj = $(LIB_SRC:%.c=%.o)
#	
#	
#	all:$(obj)
#		#$(AR)  -rcs  $(PRO_NAME) $(obj)
#		cp $(PRO_NAME) $(MYLIB)
#		
#	%.o: %.c
#		$(CC) -c -fPIE $(CFLAGS) -o $@ $< -I ../include
#	clean:
#		#@rm $(obj)  $(PRO_NAME) -rf
#		/bin/rm -f *.o
#		/bin/rm -f $(PRO_NAME)

TARGET = librtsp.a

all : rtsp_parse.o rtsp_handler.o rtp.o h264.o tcp.o udp.o 

	$(AR) -r $(TARGET) *.o
	cp $(TARGET) $(MYLIB)

%.o: %.c
	$(CC) -c -fPIE $(CFLAGS) -o $@ $<

clean :
	/bin/rm -f *.o
	/bin/rm -f $(TARGET)