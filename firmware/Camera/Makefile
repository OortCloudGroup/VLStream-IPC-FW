##################################
# 
# Authors: ZhangXuelian
# 	
# 
# Changes:
# 	
#	
##################################
subdirs = libaenet libstl libutility libpthread libcrypt libuuid librtsp libhttpserver libupload#libprotocol #liblua liblog
subdirs += main
HOME=$(shell pwd)
export HOME
all:
	@cp ./buildx264/x264install/lib/libx264.a ./lib
	@for dir in $(subdirs); do make -C $$dir||exit $$?; done
	

clean:
	@for dir in $(subdirs); do make -C $$dir clean; done
	rm -f lib/*.a
	rm -f lib/*.so

