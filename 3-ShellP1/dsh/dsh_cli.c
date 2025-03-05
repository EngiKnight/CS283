#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dshlib.h"

int main() {
    char cmd_buff[SH_CMD_MAX];
    int rc;
    command_list_t clist;
    
    while (1) {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        char *p = cmd_buff;
        while (*p && isspace((unsigned char)*p))
            p++;
        if (*p == '\0') {
            printf(CMD_WARN_NO_CMD);
            continue;
        }
        
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            exit(0);
        }
        
        if (strcmp(cmd_buff, "dragon") == 0) {
            print_dragon();
            continue;
        }
        
        rc = build_cmd_list(cmd_buff, &clist);
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD);
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
        } else if (rc == OK) {
            printf(CMD_OK_HEADER, clist.num);
            for (int i = 0; i < clist.num; i++) {
                if (strlen(clist.commands[i].args) == 0)
                    printf("<%d> %s\n", i+1, clist.commands[i].exe);
                else
                    printf("<%d> %s [%s]\n", i+1, clist.commands[i].exe, clist.commands[i].args);
            }
        }
    }
    return 0;
}
