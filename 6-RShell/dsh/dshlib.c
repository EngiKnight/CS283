#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include "dshlib.h"

static int last_rc = 0;

char *trim(char *str) {
    while(isspace(*str)) str++;
    char *end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) end--;
    end[1] = '\0';
    return str;
}

void parse_segment(char *segment, command_t *cmd){
    cmd->argc = 0;
    cmd->infile = cmd->outfile = NULL;
    cmd->append = 0;
    char *arg = strtok(segment, " ");
    while(arg && cmd->argc < CMD_ARGV_MAX) {
        if(strcmp(arg, "<") == 0){
            arg = strtok(NULL, " ");
            cmd->infile = strdup(arg);
        } else if(strcmp(arg, ">") == 0) {
            arg = strtok(NULL, " ");
            cmd->outfile = strdup(arg);
            cmd->append = 0;
        } else if(strcmp(arg, ">>") == 0) {
            arg = strtok(NULL, " ");
            cmd->outfile = strdup(arg);
            cmd->append = 1;
        } else {
            cmd->argv[cmd->argc++] = strdup(arg);
        }
        arg = strtok(NULL, " ");
    }
    cmd->argv[cmd->argc] = NULL;
}

int exec_local_cmd_loop() {
    char line[SH_CMD_MAX];
    command_list_t clist;
    int first_iter = 1;

    while (1) {
        printf("%s", SH_PROMPT);
        fflush(stdout);

        if (fgets(line, SH_CMD_MAX, stdin) == NULL) {
            printf("cmd loop returned 0\n");
            break;
        }

        line[strcspn(line, "\n")] = '\0';
        if (strlen(trim(line)) == 0) continue;
        if (strcmp(line, EXIT_CMD) == 0) break;

        char *line_copy = strdup(line);
        int count = 0;
        char *token = strtok(line_copy, "|");
        while (token && count < CMD_MAX) {
            parse_segment(trim(token), &clist.commands[count]);
            count++;
            token = strtok(NULL, "|");
        }
        clist.num = count;

        int pipefd[CMD_MAX-1][2];
        for(int i = 0; i < clist.num - 1; i++)
            pipe(pipefd[i]);

        pid_t pids[CMD_MAX];
        for(int i = 0; i < clist.num; i++) {
            pids[i] = fork();
            if(pids[i] == 0) {
                if(i > 0)
                    dup2(pipefd[i-1][0], STDIN_FILENO);
                if(i < clist.num-1)
                    dup2(pipefd[i][1], STDOUT_FILENO);
                for(int j = 0; j < clist.num - 1; j++){
                    close(pipefd[j][0]);
                    close(pipefd[j][1]);
                }
                execvp(clist.commands[i].argv[0], clist.commands[i].argv);
                perror("execvp");
                exit(1);
            }
        }

        for(int i = 0; i < clist.num - 1; i++){
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }

        int status;
        for(int i = 0; i < clist.num; i++)
            waitpid(pids[i], &status, 0);

        for(int i = 0; i < clist.num; i++) {
            for(int j = 0; j < clist.commands[i].argc; j++)
                free(clist.commands[i].argv[j]);
            if(clist.commands[i].infile) free(clist.commands[i].infile);
            if(clist.commands[i].outfile) free(clist.commands[i].outfile);
        }

        free(line_copy);
        first_iter = 0;
    }
    return 0;
}

void print_dragon(){
    printf("[DRAGON ASCII ART]\n");
}
