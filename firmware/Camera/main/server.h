/*
 All source file must include this head file
 
 Authors: ZhangXuelian

 Changes:
 	
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#define MY_VERSION "1.0.0.1"

#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <locale.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include "../libaenet/ae.h" 
#include "../libaenet/net.h"
#include "../libstl/libstl.h"
#include "../libutility/libutility.h"
#include "../liblog/liblog.h"
#include "../libcrypt/libcrypt.h"
#include "../libuuid/libc_uuid/get_uuid.h"
#include "../libpthread/libpthread.h"
#include "../libprotocol/protocol.h"
// for lua
#include "lua/lua.h"
#include "lua/luaconf.h"
#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "vls_record.h"
#include "vls_rtsp_server.h"
#include "c_queue_frame.h"
#include "vls_server.h"
#include "vls_upload.h"


// Error codes 
#define C_OK                    0
#define C_ERR                   -1

#include "../libprotocol/protocol.h"

#endif


