#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"
static int last_rc = 0;
static char *ltrim(char *s){
    while(*s && isspace((unsigned char)*s)) s++;
    return s;
}
static char *rtrim(char *s){
    char *back = s + strlen(s);
    while(back > s && isspace((unsigned char)*(--back))) *back = '\0';
    return s;
}
static char *trim(char *s){
    return rtrim(ltrim(s));
}
static void parse_cmd_buff(char *line, cmd_buff_t *buff){
    buff->argc = 0;
    char *p = line;
    while(*p){
        while(*p && isspace((unsigned char)*p)) p++;
        if(!*p) break;
        if(*p=='"'){
            p++;
            buff->argv[buff->argc] = p;
            while(*p && *p!='"') p++;
            if(*p=='"'){*p='\0'; p++;}
            buff->argc++;
        } else {
            buff->argv[buff->argc] = p;
            while(*p && !isspace((unsigned char)*p)) p++;
            if(*p){*p='\0'; p++;}
            buff->argc++;
        }
    }
    buff->argv[buff->argc] = NULL;
}
int exec_local_cmd_loop(){
    char line[SH_CMD_MAX];
    cmd_buff_t buff;
    buff._cmd_buffer = line;
    while(1){
        printf(SH_PROMPT);
        if(fgets(line, SH_CMD_MAX, stdin)==NULL){printf("\n"); break;}
        line[strcspn(line,"\n")] = '\0';
        trim(line);
        if(strlen(line)==0) continue;
        parse_cmd_buff(line, &buff);
        if(buff.argc==0) continue;
        if(strcmp(buff.argv[0],EXIT_CMD)==0) exit(0);
        if(strcmp(buff.argv[0],"cd")==0){
            if(buff.argc==2){
                if(chdir(buff.argv[1])<0){
                    perror("cd");
                    last_rc = errno;
                } else {
                    last_rc = 0;
                }
            }
            continue;
        }
        if(strcmp(buff.argv[0],"rc")==0){
            printf("%d\n",last_rc);
            continue;
        }
        pid_t pid = fork();
        if(pid<0){
            perror("fork");
            continue;
        }
        if(pid==0){
            execvp(buff.argv[0],buff.argv);
            if(errno==ENOENT){
                printf("Command not found in PATH\n");
            } else if(errno==EACCES){
                printf("Permission denied\n");
            } else {
                printf("Error executing command\n");
            }
            exit(errno);
        } else {
            int status;
            waitpid(pid,&status,0);
            if(WIFEXITED(status)){
                last_rc = WEXITSTATUS(status);
            } else {
                last_rc = -1;
            }
        }
    }
    return 0;
}
void print_dragon(){}
