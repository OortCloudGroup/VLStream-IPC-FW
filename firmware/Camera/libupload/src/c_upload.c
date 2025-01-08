/* See LICENSE for license details. */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include "c_upload.h"
#include "sandbox.h"

typedef struct tagUploadContext
{
	struct stat sb;
	int ret;
	int port;

} UPLOAD_CONTEXT;

UPLOAD_CONTEXT g_upload_context = {0};

int c_init_upload()
{
	UPLOAD_CONTEXT* c = &g_upload_context;
	c->port = 9527;
	// 使用当前时间来设置随机数种子
    srand(time(NULL));

	c->ret = stat("download", &c->sb);
	if ((c->ret && mkdir("download", 0700)) ||
	   (!c->ret && !S_ISDIR(c->sb.st_mode))) {
		printf("Failed to create download directory\n");
		return -1;
	}

	if (server_init(c->port)) {
		printf("Failed to initialize the server\n");
		return -1;
	}

	load_file("static/index.html", "/", "text/html");
	load_file("static/upload.html", "/upload", "text/html");
	load_file("static/favicon.ico", "/favicon.ico", "image/x-icon");

	init_pollfds();
	return 0;
}

void c_do_upload_handler()
{
	UPLOAD_CONTEXT* c = &g_upload_context;
	server_thread();
}

void c_deinit_upload()
{
	UPLOAD_CONTEXT* c = &g_upload_context;
	server_stop();
}
// int main(int argc, char* argv[]) {
// 	struct stat sb;
// 	int ret, port = 8080;

// 	srand(time(NULL));

// 	ret = stat("download", &sb);
// 	if ((ret && mkdir("download", 0700)) ||
// 	   (!ret && !S_ISDIR(sb.st_mode))) {
// 		printf("Failed to create download directory\n");
// 		return -1;
// 	}

// 	if (argc > 1) {
// 		port = atoi(argv[1]);
// 		if (!port) port = 8080;
// 	}

// 	load_file("static/index.html", "/", "text/html");
// 	load_file("static/upload.html", "/upload", "text/html");
// 	load_file("static/favicon.ico", "/favicon.ico", "image/x-icon");
// 	sandbox_start();

// 	if (server_init(port)) {
// 		printf("Failed to initialize the server\n");
// 		return -1;
// 	}
// 	server_thread();
// 	server_stop();

// 	return 0;
// }
