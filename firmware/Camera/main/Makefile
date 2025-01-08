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

EXTRALIB += -lae_stl -lae_pthread -lae_net -lae_utility  #-lae_protocol
EXTRALIB += -lpthread -lrt -lrtsp -lx264 -lm -lserver -lupload #-lae_log -ldl -lm -lc 

ifeq ($(USE_SSL),y)
EXTRALIB += -lssl -lcrypto 
endif

#------------------------------------------------- 
OBJS = server.o vls_record.o vls_rtsp_server.o vls_server.o c_h264_encoder.o c_queue_frame.o c_rtsp_server.o vls_upload.o
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
	rm -f $(TARGET) $(OBJS)
    
