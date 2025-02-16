#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dshlib.h"

/*
 * Implement your main function by building a loop that prompts the
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.  Since we want fgets to also handle
 * end of file so we can run this headless for testing we need to check
 * the return code of fgets.  I have provided an example below of how
 * to do this assuming you are storing user input inside of the cmd_buff
 * variable.
 *
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 *
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 *
 *   Also, use the constants in the dshlib.h in this code.
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *
 *   Expected output:
 *
 *      CMD_OK_HEADER      if the command parses properly. You will
 *                         follow this by the command details
 *
 *      CMD_WARN_NO_CMD    if the user entered a blank command
 *      CMD_ERR_PIPE_LIMIT if the user entered too many commands using
 *                         the pipe feature, e.g., cmd1 | cmd2 | ... |
 *
 *  See the provided test cases for output expectations.
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    // If the user just pressed enter or line is empty, warn no commands
    if (cmd_line[0] == '\0') {
        return WARN_NO_CMDS;
    }

    clist->num = 0;

    // Split on '|'
    char *saveptr;
    char *token = strtok_r(cmd_line, PIPE_STRING, &saveptr);
    while (token != NULL) {
        // If we already have 8 commands, the next one is too many
        if (clist->num == CMD_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        // Trim leading spaces
        while (*token == ' ') {
            token++;
        }
        // If token is now empty, it means there was nothing here
        if (*token == '\0') {
            return WARN_NO_CMDS;
        }

        // Separate exe (first word) from args (the rest)
        char *space = strchr(token, ' ');
        if (space) {
            // Mark the end of the exe
            *space = '\0';
            strncpy(clist->commands[clist->num].exe, token, EXE_MAX);
            clist->commands[clist->num].exe[EXE_MAX - 1] = '\0';

            // Skip any additional spaces before args
            char *args_start = space + 1;
            while (*args_start == ' ') {
                args_start++;
            }
            strncpy(clist->commands[clist->num].args, args_start, ARG_MAX);
            clist->commands[clist->num].args[ARG_MAX - 1] = '\0';
        } else {
            // Entire token is the exe, no args
            strncpy(clist->commands[clist->num].exe, token, EXE_MAX);
            clist->commands[clist->num].exe[EXE_MAX - 1] = '\0';
            clist->commands[clist->num].args[0] = '\0';
        }

        clist->num++;
        token = strtok_r(NULL, PIPE_STRING, &saveptr);
    }

    // If we somehow got no commands, return a warning
    if (clist->num == 0) {
        return WARN_NO_CMDS;
    }

    return OK;
}

int main()
{
    // Allocate memory for input buffer
    char *cmd_buff = malloc(ARG_MAX);
    if (!cmd_buff) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    command_list_t clist;

    while (1) {
        // Print the prompt
        printf("%s", SH_PROMPT);

        // Read user input
        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL) {
            // EOF or error; just break to exit
            printf("\n");
            break;
        }

        // Strip trailing newline
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        // Check for "exit" command before parsing
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            // User wants to exit
            break;
        }

        // Parse the command line into a list of commands
        int rc = build_cmd_list(cmd_buff, &clist);

        if (rc == WARN_NO_CMDS) {
            // Empty command => print warning, continue shell
            printf("%s", CMD_WARN_NO_CMD);  // "warning: no commands provided\n"
            continue;
        } else if (rc == ERR_TOO_MANY_COMMANDS) {
            // Too many piped commands => print error, continue shell
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX); // "error: piping limited to %d commands\n"
            continue;
        } else if (rc != OK) {
            // Some other error
            continue;
        }

        // If we reach here, we have a valid command list
        // Print "PARSED COMMAND LINE - TOTAL COMMANDS X" exactly once
        // to match test expectations (the tests remove all whitespace)
        printf("PARSED COMMAND LINE - TOTAL COMMANDS %d", clist.num);
        printf("\n");
        // Print each command in the form <index>exe[args]
        // The BATS tests remove whitespace, so spacing is not critical,
        // but be consistent so it matches the expected string.
        for (int i = 0; i < clist.num; i++) {
            if (clist.commands[i].args[0] != '\0') {
                // We have arguments
                printf("<%d>%s [%s]", i + 1,
                       clist.commands[i].exe,
                       clist.commands[i].args);
            } else {
                // No arguments
                printf("<%d>%s", i + 1, clist.commands[i].exe);
            }
        }
        // Done printing parsed commands; loop back for next prompt
    }

    free(cmd_buff);
    return 0;
}