#ifndef __DSHLIB_H__
#define __DSHLIB_H__
#include <sys/types.h>
#include <unistd.h>
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
#define SH_CMD_MAX 1024
#define SH_PROMPT "dsh3> "
#define EXIT_CMD "exit"
#define CMD_ARGV_MAX 64
typedef struct {
    int argc;
    char *argv[CMD_ARGV_MAX];
    char *infile;
    char *outfile;
    int append;
} command_t;
typedef struct {
    int num;
    command_t commands[CMD_MAX];
} command_list_t;
int exec_local_cmd_loop();
void print_dragon();
#endif
