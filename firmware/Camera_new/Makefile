##################################
# 
# Authors: ZhangXuelian
# 	
# 
# Changes:
# 	
#	
##################################
# subdirs = libaenet libstl libutility libpthread liblog libcrypt libuuid #libprotocol #liblua
# subdirs += main
# HOME=$(shell pwd)
# export HOME
# all:
# 	@for dir in $(subdirs); do make -C $$dir||exit $$?; done

# clean:
# 	@for dir in $(subdirs); do make -C $$dir clean; done
# 	rm -f lib/*.a
# 	rm -f lib/*.so


subdirs = libaenet libstl libutility libpthread liblog libcrypt libuuid
subdirs += main
# 这里假设上面这些目录都是在当前目录下的子目录，和源文件结构对应好，若实际不同需调整

all:
	@for /f "delims=" %%i in ('dir /b /ad $(subdirs)') do (
		cd %%i
		mingw32-make
		cd..
	)

clean:
	@for /f "delims=" %%i in ('dir /b /ad $(subdirs)') do (
		cd %%i
		mingw32-make clean
		cd..
	)
	del /q lib\*.a
	del /q lib\*.so

