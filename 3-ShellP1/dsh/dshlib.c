#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dshlib.h"

static char *trim(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0)
        return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end+1) = '\0';
    return str;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char cmd_copy[SH_CMD_MAX];
    strncpy(cmd_copy, cmd_line, SH_CMD_MAX);
    cmd_copy[SH_CMD_MAX - 1] = '\0';
    char *line = trim(cmd_copy);
    if (line[0] == '\0')
        return WARN_NO_CMDS;
    int cmd_count = 0;
    char *token;
    char *saveptr;
    token = strtok_r(line, PIPE_STRING, &saveptr);
    while (token != NULL) {
        char *command_str = trim(token);
        if (command_str[0] == '\0') {
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        if (cmd_count >= CMD_MAX)
            return ERR_TOO_MANY_COMMANDS;
        char *arg_token;
        char *cmd_saveptr;
        arg_token = strtok_r(command_str, " ", &cmd_saveptr);
        if (arg_token == NULL) {
            token = strtok_r(NULL, PIPE_STRING, &saveptr);
            continue;
        }
        if (strlen(arg_token) >= EXE_MAX)
            return ERR_CMD_OR_ARGS_TOO_BIG;
        strcpy(clist->commands[cmd_count].exe, arg_token);
        clist->commands[cmd_count].args[0] = '\0';
        int first_arg = 1;
        while ((arg_token = strtok_r(NULL, " ", &cmd_saveptr)) != NULL) {
            if (!first_arg)
                strcat(clist->commands[cmd_count].args, " ");
            first_arg = 0;
            if (strlen(arg_token) >= ARG_MAX)
                return ERR_CMD_OR_ARGS_TOO_BIG;
            strcat(clist->commands[cmd_count].args, arg_token);
        }
        cmd_count++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }
    clist->num = cmd_count;
    return OK;
}

void print_dragon() {
    static const unsigned char dragon_xor[] = {
        0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,
        0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,
        0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,
        0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0xE6,0x00,
    };
    size_t len = sizeof(dragon_xor) / sizeof(dragon_xor[0]);
    for (size_t i = 0; i < len; i++) {
        putchar(dragon_xor[i] ^ 0xAA);
    }
}
