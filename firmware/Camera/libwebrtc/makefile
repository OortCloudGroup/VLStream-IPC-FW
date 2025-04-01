include ../config.mk
include ../server.mk

CXXFLAGS = -Wall -g

# 定义基础目录
HOME_BASE = ./webrtc

# 包含路径
INCLUDE_DIRS = \
    -I$(HOME_BASE)/libmetartc6/src \
    -I$(HOME_BASE)/thirdparty/include \
    -I$(HOME_BASE)/include \
    -I$(HOME_BASE)/src  # 添加这个以确保找到头文件

# 库搜索路径和库名 (仅限Unix)
LIBRARY_DIRS = \
    -L$(HOME_BASE)/thirdparty/lib \
    -L$(HOME_BASE)/bin/lib_release

CFLAGS = -Iinclude -I$(INCLUDE_DIRS)

LIBRARIES = \
    -lmetartc6 -lmetartccore6 -lyuv -lopenh264 -lspeexdsp -lopus -lusrsctp \
    -lpthread -lasound -ldl -lssl2 -lcrypto2 -lsrtp2 -lm

# C++ 源文件列表
CPP_SRCS = \
    $(HOME_BASE)/src/recordpushrtc.cpp \
    $(HOME_BASE)/src/recordpushrtc_capi.cpp \
    $(HOME_BASE)/src/yangrecordthread.cpp \
    $(HOME_BASE)/src/yangpush/YangPushCapture.cpp \
    $(HOME_BASE)/src/yangpush/YangPushEncoder.cpp \
    $(HOME_BASE)/src/yangpush/YangPushFactory.cpp \
    $(HOME_BASE)/src/yangpush/YangPushHandleImpl.cpp \
    $(HOME_BASE)/src/yangpush/YangPushMessageHandle.cpp \
    $(HOME_BASE)/src/yangpush/YangPushPublish.cpp \
    $(HOME_BASE)/src/yangpush/YangRtcPublish.cpp \
    $(HOME_BASE)/src/yangpush/YangSendVideoImpl.cpp
OBJS = $(CPP_SRCS:.cpp=.o)

CXXFLAGS += $(INCLUDE_DIRS)

STATICEXE = librecordpush.a

# 静态库规则
$(STATICEXE): $(OBJS)
	ar rcs $@ $^
	cp $(STATICEXE) $(MYLIB)
# 生成目标文件的规则
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
# 主程序规则
main: main.c librecordpush.a
	$(CC) $(CFLAGS) -c main.c -o main.o
	$(CC) main.o -L. $(LIBRARY_DIRS) -lrecordpush -o main $(LIBRARIES) -lstdc++

# 清理
clean:
	rm -f $(OBJS) librecordpush.a main.o main
	rm -f $(MYLIB)/$(STATICEXE)
	
