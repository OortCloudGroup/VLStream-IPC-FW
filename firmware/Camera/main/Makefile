##################################
# 
# Authors: ZhangXuelian
# 	
# 
# Changes:
# 	
#	
##################################
#-------------------------------------------------
include ../config.mk
include ../server.mk

EXTRALIB += -lae_stl -lae_pthread -lae_net -lae_log  -lae_utility  #-lae_protocol
EXTRALIB += -lpthread -lrt -lrtsp -lx264 -lm#-ldl -lm -lc 

ifeq ($(USE_SSL),y)
EXTRALIB += -lssl -lcrypto 
endif

#------------------------------------------------- 
OBJS = server.o server_cron.o t_dict.o option.o networking.o t_record.o fbmap.o t_rtsp_server.o rtsp_server.o h264encoder.o queueFrame.o rtsp_demo.o rtsp_msg.o rtp_enc.o stream_queue.o utils.o
#-------------------------------------------------
TARGET = server
#-------------------------------------------------
all : $(TARGET)
$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(EXTRALIB)	
ifeq ($(RELEASE),y)	
	$(STRIP) $(TARGET)
endif	

.PHONY: clean

clean: 
	rm -f $(TARGET)
    
