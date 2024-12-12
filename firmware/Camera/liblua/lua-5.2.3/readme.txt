���ڰ��ĵ������⡣
�˴���Ҫ�õ�ԭʼ��ƽ̨�ϵİ������á�

�ԱȺ󣬽����֣�ֻ��һ���ļ��в��


luaconf.h

/* ��ϸ��˵�����˴���Luaִ��ʱ��Ѱ�ҿ��·����Ĭ��ϵͳ������ν��
   ���д���˺ܶ�⣬��Ҫ������ָ����·��������Luaִ��ʱ���Զ�ȥ�ҵ����λ��
   http://www.educity.cn/wenda/308308.html
   http://see.sl088.com/wiki/Lua_%E5%8C%85%E8%B7%AF%E5%BE%84%E6%93%8D%E4%BD%9C
   http://stackoverflow.com/questions/19982915/how-to-include-lua-module-in-build
   http://www.4byte.cn/question/364946/how-to-include-lua-module-in-build.html
   
   Add the dist paths to the environment: LUA_CPATH='dist/usr/local/lib/lua/5.1/?.so;;' and LUA_PATH='dist/usr/local/share/lua/5.1/?.lua;;'
   �����˴��У�ͨ�䣬���뵽���ǲ���Luaִ��ʱ���ڻ��������������ʲô��������
   �ڱ����ʱ���ָ���ˣ�Ѱ�ҿ��·����
   �����ڿ�OPENWRT�����Makefile�ļ�ʱ������Ҫ��luaconf.h����ļ�����ָ��·����

    �������ڴ��޸ġ�
    ����Lua ִ�е�ʱ��û�д���������ָ��Ѱ�ҿ��·����lua --?
    Ҳ��Ҫ���Ƕ�������������ͨ���Ǹ���
    Ҫ��Ȼ�����Ǳ�����˳�������������ļ�������ҵ���
    Ҳ����˵���ҵĳ���Ҫ�� ϵͳ�Դ���/usr/bin/lua ����ļ���ִ�л�����һ���ġ��Ϳ����ˡ�
    ��ˣ�Ҫ�����ı�����̣�����β����������
    
    ���������ű���û�����û�����������ˣ��϶������ڱ����
    
    /etc/init.d/ �»��м��� �õ��� lua -l xxxx ���������÷���
       
   
@@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
@* Lua libraries.
@@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
@* C libraries.
** CHANGE them if your machine has a non-conventional directory
** hierarchy or if you want to install your libraries in
** non-conventional directories.
*/
#if defined(_WIN32)	/* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define LUA_LDIR	"!\\lua\\"
#define LUA_CDIR	"!\\"
#define LUA_PATH_DEFAULT  \
		LUA_LDIR"?.lua;"  LUA_LDIR"?\\init.lua;" \
		LUA_CDIR"?.lua;"  LUA_CDIR"?\\init.lua;" ".\\?.lua"
#define LUA_CPATH_DEFAULT \
		LUA_CDIR"?.dll;" LUA_CDIR"loadall.dll;" ".\\?.dll"

#else			/* }{ */

/*
#define LUA_VDIR	LUA_VERSION_MAJOR "." LUA_VERSION_MINOR "/"
#define LUA_ROOT	"/usr/local/"
#define LUA_LDIR	LUA_ROOT "share/lua/" LUA_VDIR
#define LUA_CDIR	LUA_ROOT "lib/lua/" LUA_VDIR
#define LUA_PATH_DEFAULT  \
		LUA_LDIR"?.lua;"  LUA_LDIR"?/init.lua;" \
		LUA_CDIR"?.lua;"  LUA_CDIR"?/init.lua;" "./?.lua"
#define LUA_CPATH_DEFAULT \
		LUA_CDIR"?.so;" LUA_CDIR"loadall.so;" "./?.so" 
*/

// for openwrt environment by zxl 2014.10.24
// �ɴ�������Luaִ��ʱ��Ѱ�ҿ��·����
#define LUA_ROOT	"/usr/"
#define LUA_LDIR	LUA_ROOT "share/lua/"
#define LUA_CDIR	LUA_ROOT "lib/lua/"
#define LUA_PATH_DEFAULT  \
		"./?.lua;"  LUA_LDIR"?.lua;"  LUA_LDIR"?/init.lua;" \
		            LUA_CDIR"?.lua;"  LUA_CDIR"?/init.lua"
#define LUA_CPATH_DEFAULT \
	"./?.so;"  LUA_CDIR"?.so;" LUA_CDIR"loadall.so"
#endif			/* } */

// edited end.


���Ϸ�����Lua�е��ÿ�ʱ��Lua��Ŀ�û���ҵ���֮��ģ��ͻ���ֶϴ���
��ˣ����ǻָ���5.2.5�汾�ģ�������dofile�������������û�еĿ⣬������ʾ��Ϣ������

���û��� �����ķ�ʽ��
������������û��д��
export LUA_CPATH='/usr/lib/lua/?.so'
export LUA_PATH='/usr/lib/lua/?.lua'
����������������ǶԵġ�
�ڳ�������������shell����һ���ġ�

��ˣ��������ַ��������С�

������Ȼ���ֶδ��������� OPENWRT���õ�lua���汾�����˺ܶಹ���������������İ汾���������������� ����С�
����Ͳ������ˣ�ͨ���Աȷ��֣����˺ܶಹ�����޸��˺ܶ���룬�ֲ������в��ˡ������Ļ���LUCIд�����ĳ��򣬾Ͳ����ڱ�׼��LUA�������ˡ�
����ֶδ���

LUA_CPATH='/usr/local/lib/lua/5.1/?.so;;' 
LUA_PATH='/usr/local/share/lua/5.1/?.lua;;'

�ֲ���һ���� ����luci�Ŀ⣬�ͳ��ֶδ���
����ֱ��COPY��ϵͳ���lua.a�ļ������á�������������û��ʹ�á�

2014.10.24

