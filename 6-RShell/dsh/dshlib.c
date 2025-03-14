#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include "dshlib.h"

static int last_rc = 0;
static char *ltrim(char *s){while(*s && isspace((unsigned char)*s))s++; return s;}
static char *rtrim(char *s){char *back = s+strlen(s); while(back>s && isspace((unsigned char)*(--back))) *back='\0'; return s;}
static char *trim(char *s){return rtrim(ltrim(s));}
static void parse_segment(char *seg, command_t *cmd){
    cmd->argc = 0;
    cmd->infile = NULL;
    cmd->outfile = NULL;
    cmd->append = 0;
    char *p = seg;
    while(*p){
        while(*p && isspace((unsigned char)*p)) p++;
        if(!*p) break;
        char token[256];
        int i = 0;
        if(*p=='"'){
            p++;
            while(*p && *p!='"' && i<255) token[i++]=*p++;
            if(*p=='"') p++;
        } else {
            while(*p && !isspace((unsigned char)*p) && i<255) token[i++]=*p++;
        }
        token[i]='\0';
        if(strcmp(token,"<")==0){
            while(*p && isspace((unsigned char)*p)) p++;
            int j = 0;
            char file[256];
            if(*p=='"'){
                p++;
                while(*p && *p!='"' && j<255) file[j++]=*p++;
                if(*p=='"') p++;
            } else {
                while(*p && !isspace((unsigned char)*p) && j<255) file[j++]=*p++;
            }
            file[j]='\0';
            cmd->infile = strdup(file);
        } else if(strcmp(token,">")==0 || strcmp(token,">>")==0){
            if(strcmp(token,">>")==0) cmd->append = 1;
            while(*p && isspace((unsigned char)*p)) p++;
            int j = 0;
            char file[256];
            if(*p=='"'){
                p++;
                while(*p && *p!='"' && j<255) file[j++]=*p++;
                if(*p=='"') p++;
            } else {
                while(*p && !isspace((unsigned char)*p) && j<255) file[j++]=*p++;
            }
            file[j]='\0';
            cmd->outfile = strdup(file);
        } else {
            cmd->argv[cmd->argc] = strdup(token);
            cmd->argc++;
        }
    }
    cmd->argv[cmd->argc] = NULL;
}
int exec_local_cmd_loop(){
    char line[SH_CMD_MAX];
    command_list_t clist;
    while(1){
        printf(SH_PROMPT);
        if(fgets(line, SH_CMD_MAX, stdin)==NULL){printf("\n"); break;}
        line[strcspn(line,"\n")] = '\0';
        trim(line);
        if(strlen(line)==0) continue;
        if(strcmp(line,EXIT_CMD)==0){printf("exiting...\n"); last_rc = 0; break;}
        char *line_copy = strdup(line);
        int count = 0;
        char *token = strtok(line_copy, "|");
        while(token && count < CMD_MAX){
            clist.commands[count].infile = NULL;
            clist.commands[count].outfile = NULL;
            clist.commands[count].append = 0;
            char *seg = trim(token);
            parse_segment(seg, &clist.commands[count]);
            count++;
            token = strtok(NULL, "|");
        }
        free(line_copy);
        clist.num = count;
        if(clist.num == 1){
            if(clist.commands[0].argc > 0 && strcmp(clist.commands[0].argv[0],"cd")==0){
                if(clist.commands[0].argc==2){
                    if(chdir(clist.commands[0].argv[1])<0){perror("cd"); last_rc = errno;} else {last_rc = 0;}
                }
                goto cleanup;
            }
            if(clist.commands[0].argc > 0 && strcmp(clist.commands[0].argv[0],"rc")==0){
                printf("%d\n", last_rc);
                goto cleanup;
            }
        }
        int i;
        int pipefd[CMD_MAX-1][2];
        for(i = 0; i < clist.num - 1; i++){
            if(pipe(pipefd[i]) < 0){perror("pipe"); exit(1);}
        }
        pid_t pids[CMD_MAX];
        for(i = 0; i < clist.num; i++){
            pid_t pid = fork();
            if(pid < 0){perror("fork"); exit(1);}
            if(pid == 0){
                if(i > 0){dup2(pipefd[i-1][0], STDIN_FILENO);}
                if(i < clist.num - 1){dup2(pipefd[i][1], STDOUT_FILENO);}
                for(int j = 0; j < clist.num - 1; j++){
                    close(pipefd[j][0]);
                    close(pipefd[j][1]);
                }
                if(clist.commands[i].infile){
                    int fd = open(clist.commands[i].infile, O_RDONLY);
                    if(fd < 0){perror("open infile"); exit(errno);}
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                if(clist.commands[i].outfile){
                    int fd;
                    if(clist.commands[i].append)
                        fd = open(clist.commands[i].outfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
                    else
                        fd = open(clist.commands[i].outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(fd < 0){perror("open outfile"); exit(errno);}
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                execvp(clist.commands[i].argv[0], clist.commands[i].argv);
                if(errno==ENOENT){printf("Command not found in PATH\n");}
                else if(errno==EACCES){printf("Permission denied\n");}
                else {printf("Error executing command\n");}
                exit(errno);
            } else {
                pids[i] = pid;
            }
        }
        for(i = 0; i < clist.num - 1; i++){
            close(pipefd[i][0]);
            close(pipefd[i][1]);
        }
        int status;
        for(i = 0; i < clist.num; i++){
            waitpid(pids[i], &status, 0);
            if(i == clist.num - 1){
                if(WIFEXITED(status)){last_rc = WEXITSTATUS(status);} else {last_rc = -1;}
            }
        }
    cleanup:
        for(int i = 0; i < clist.num; i++){
            for(int j = 0; j < clist.commands[i].argc; j++){
                free(clist.commands[i].argv[j]);
            }
            if(clist.commands[i].infile) free(clist.commands[i].infile);
            if(clist.commands[i].outfile) free(clist.commands[i].outfile);
        }
    }
    printf("cmd loop returned %d\n", 0);
    return 0;
}
void print_dragon(){}
