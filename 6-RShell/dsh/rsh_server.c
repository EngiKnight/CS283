#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "dshlib.h"
#include "rshlib.h"
#include <errno.h>
#include <pthread.h>

static int server_threaded = 0;

void *handle_client(void *arg) {
    int cli_socket = *(int*)arg;
    free(arg);
    exec_client_requests(cli_socket);
    close(cli_socket);
    return NULL;
}

int start_server(char *ifaces, int port, int is_threaded) {
    server_threaded = is_threaded;
    int svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0)
        return svr_socket;
    int rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);
    return rc;
}

int boot_server(char *ifaces, int port) {
    int svr_socket;
    struct sockaddr_in addr;
    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }
    int enable = 1;
    setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ifaces);
    addr.sin_port = htons(port);
    if (bind(svr_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return ERR_RDSH_COMMUNICATION;
    }
    if (listen(svr_socket, 20) < 0) {
        perror("listen");
        return ERR_RDSH_COMMUNICATION;
    }
    return svr_socket;
}

int process_cli_requests(int svr_socket) {
    int rc = OK;
    while(1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int *cli_socket = malloc(sizeof(int));
        if (!cli_socket)
            return ERR_RDSH_SERVER;
        *cli_socket = accept(svr_socket, (struct sockaddr*)&client_addr, &addrlen);
        if (*cli_socket < 0) {
            perror("accept");
            free(cli_socket);
            return ERR_RDSH_COMMUNICATION;
        }
        if(server_threaded) {
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, cli_socket);
            pthread_detach(tid);
        } else {
            rc = exec_client_requests(*cli_socket);
            close(*cli_socket);
            free(cli_socket);
            if(rc == OK_EXIT)
                break;
        }
    }
    return rc;
}

int send_message_string(int cli_socket, char *buff) {
    int total = strlen(buff);
    int sent = 0;
    while(sent < total) {
        int n = send(cli_socket, buff+sent, total-sent, 0);
        if(n <= 0)
            return ERR_RDSH_COMMUNICATION;
        sent += n;
    }
    return send_message_eof(cli_socket);
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int num = clist->num;
    int status;
    if (num == 1) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); exit(EXIT_FAILURE); }
        if (pid == 0) {
            dup2(cli_sock, STDIN_FILENO);
            dup2(cli_sock, STDOUT_FILENO);
            dup2(cli_sock, STDERR_FILENO);
            execvp(clist->commands[0].argv[0], clist->commands[0].argv);
            if(errno == ENOENT)
                printf("Command not found in PATH\n");
            else if(errno == EACCES)
                printf("Permission denied\n");
            else
                printf("Error executing command\n");
            exit(errno);
        } else {
            waitpid(pid, &status, 0);
            return WEXITSTATUS(status);
        }
    } else {
        int pipes[num-1][2];
        pid_t pids[num];
        for (int i = 0; i < num - 1; i++) {
            if(pipe(pipes[i]) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }
        for (int i = 0; i < num; i++) {
            pid_t pid = fork();
            if(pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if(pid == 0) {
                if(i == 0) {
                    dup2(cli_sock, STDIN_FILENO);
                    dup2(pipes[i][1], STDOUT_FILENO);
                } else if(i == num - 1) {
                    dup2(pipes[i-1][0], STDIN_FILENO);
                    dup2(cli_sock, STDOUT_FILENO);
                    dup2(cli_sock, STDERR_FILENO);
                } else {
                    dup2(pipes[i-1][0], STDIN_FILENO);
                    dup2(pipes[i][1], STDOUT_FILENO);
                }
                for (int j = 0; j < num - 1; j++) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                execvp(clist->commands[i].argv[0], clist->commands[i].argv);
                if(errno == ENOENT)
                    printf("Command not found in PATH\n");
                else if(errno == EACCES)
                    printf("Permission denied\n");
                else
                    printf("Error executing command\n");
                exit(errno);
            } else {
                pids[i] = pid;
            }
        }
        for (int i = 0; i < num - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }
        for (int i = 0; i < num; i++) {
            waitpid(pids[i], &status, 0);
        }
        return WEXITSTATUS(status);
    }
}

int exec_client_requests(int cli_socket) {
    char *io_buff = malloc(RDSH_COMM_BUFF_SZ);
    if(!io_buff)
        return ERR_RDSH_SERVER;
    int rc = OK;
    while(1) {
        memset(io_buff, 0, RDSH_COMM_BUFF_SZ);
        int bytes = recv(cli_socket, io_buff, RDSH_COMM_BUFF_SZ, 0);
        if(bytes <= 0) { rc = ERR_RDSH_COMMUNICATION; break; }
        if(io_buff[bytes-1] != '\0') {}
        if(strcmp(io_buff, "exit") == 0) { rc = OK; break; }
        if(strcmp(io_buff, "stop-server") == 0) { rc = OK_EXIT; break; }
        command_list_t clist;
        clist.num = 1;
        clist.commands[0].argc = 0;
        char *token = strtok(io_buff, " ");
        while(token && clist.commands[0].argc < CMD_ARGV_MAX) {
            clist.commands[0].argv[clist.commands[0].argc++] = token;
            token = strtok(NULL, " ");
        }
        clist.commands[0].argv[clist.commands[0].argc] = NULL;
        int cmd_rc = rsh_execute_pipeline(cli_socket, &clist);
        char resp[256];
        snprintf(resp, sizeof(resp), "Return code: %d\n", cmd_rc);
        send_message_string(cli_socket, resp);
    }
    free(io_buff);
    return rc;
}

int stop_server(int svr_socket){
    return close(svr_socket);
}

int send_message_eof(int cli_socket){
    int send_len = (int)sizeof(RDSH_EOF_CHAR);
    int sent_len = send(cli_socket, &RDSH_EOF_CHAR, send_len, 0);
    if(sent_len != send_len)
        return ERR_RDSH_COMMUNICATION;
    return OK;
}
