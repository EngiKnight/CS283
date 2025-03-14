#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dshlib.h"
#include "rshlib.h"

void print_usage() {
    printf("Usage: %s [-c | -s] [-i IP] [-p PORT] [-x] [-h]\n", "dsh");
}

int main(int argc, char *argv[]) {
    int mode = 0;
    char *ip = NULL;
    int port = 0;
    int threaded = 0;
    int opt;
    while((opt = getopt(argc, argv, "csi:p:xh")) != -1) {
        switch(opt) {
            case 'c': mode = 1; break;
            case 's': mode = 2; break;
            case 'i': ip = optarg; break;
            case 'p': port = atoi(optarg); break;
            case 'x': threaded = 1; break;
            case 'h': print_usage(); exit(0);
            default: print_usage(); exit(1);
        }
    }
    if(mode == 0) {
        return exec_local_cmd_loop();
    } else if(mode == 1) {
        if(ip == NULL) ip = RDSH_DEF_CLI_CONNECT;
        if(port == 0) port = RDSH_DEF_PORT;
        return exec_remote_cmd_loop(ip, port);
    } else if(mode == 2) {
        if(ip == NULL) ip = RDSH_DEF_SVR_INTFACE;
        if(port == 0) port = RDSH_DEF_PORT;
        return start_server(ip, port, threaded);
    }
    return 0;
}
