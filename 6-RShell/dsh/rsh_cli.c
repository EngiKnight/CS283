#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dshlib.h"
#include "rshlib.h"

int start_client(char *server_ip, int port) {
    int cli_socket;
    struct sockaddr_in addr;
    cli_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(cli_socket < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server_ip);
    if(connect(cli_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(cli_socket);
        return ERR_RDSH_CLIENT;
    }
    return cli_socket;
}

int exec_remote_cmd_loop(char *address, int port) {
    char *cmd_buff = malloc(RDSH_COMM_BUFF_SZ);
    char *rsp_buff = malloc(RDSH_COMM_BUFF_SZ);
    if(!cmd_buff || !rsp_buff)
        return ERR_MEMORY;
    int cli_socket = start_client(address, port);
    if(cli_socket < 0) {
        return client_cleanup(cli_socket, cmd_buff, rsp_buff, ERR_RDSH_CLIENT);
    }
    while(1) {
        printf("rdsh> ");
        if(fgets(cmd_buff, RDSH_COMM_BUFF_SZ, stdin) == NULL) {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        int len = strlen(cmd_buff) + 1;
        if(send(cli_socket, cmd_buff, len, 0) != len) {
            perror("send");
            break;
        }
        if(strcmp(cmd_buff, "exit") == 0 || strcmp(cmd_buff, "stop-server") == 0)
            break;
        while(1) {
            int bytes = recv(cli_socket, rsp_buff, RDSH_COMM_BUFF_SZ, 0);
            if(bytes <= 0)
                break;
            int is_eof = (rsp_buff[bytes-1] == RDSH_EOF_CHAR) ? 1 : 0;
            if(is_eof) {
                rsp_buff[bytes-1] = '\0';
                printf("%s", rsp_buff);
                break;
            }
            printf("%.*s", bytes, rsp_buff);
        }
    }
    return client_cleanup(cli_socket, cmd_buff, rsp_buff, OK);
}

int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc) {
    if(cli_socket > 0)
        close(cli_socket);
    free(cmd_buff);
    free(rsp_buff);
    return rc;
}
