#ifndef _C_SERVER_H_
#define _C_SERVER_H_
#ifdef __cplusplus
extern "C"{
#endif

int c_init_server();

void c_do_server_handler();

void c_deinit_server();

void c_reinit_server();

int c_is_server_busy();

void c_wakeup_server();

#ifdef __cplusplus
}
#endif
#endif
