#ifndef __DSHLIB_H__
#define __DSHLIB_H__
#include <sys/types.h>
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define SH_CMD_MAX (EXE_MAX+ARG_MAX)
#define SH_PROMPT "dsh2> "
#define EXIT_CMD "exit"
#define CMD_ARGV_MAX 64
typedef struct {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *_cmd_buffer;
} cmd_buff_t;
int exec_local_cmd_loop();
void print_dragon();
#endif
