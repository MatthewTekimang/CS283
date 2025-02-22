#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dshlib.h"

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
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
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    if (!cmd_line || !cmd_buff) return ERR_MEMORY;
    memset(cmd_buff, 0, sizeof(cmd_buff_t));
    cmd_buff->_cmd_buffer = strdup(cmd_line);
    if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;

    bool quotes = false;
    int ccount = 0;
    char *cur = cmd_buff->_cmd_buffer;
    while (*cur && isspace((unsigned char)*cur)) cur++;
    while (*cur && ccount < CMD_ARGV_MAX - 1) {
        cmd_buff->argv[ccount] = cur;
        while (*cur) {
            if (*cur == '"') {
                quotes = !quotes;
                memmove(cur, cur + 1, strlen(cur));
                continue;
            }
            if (!quotes && isspace((unsigned char)*cur)) break;
            cur++;
        }
        if (*cur) {
            *cur++ = '\0';
            while (*cur && isspace((unsigned char)*cur)) cur++;
        }
        ccount++;
    }
    cmd_buff->argv[ccount] = NULL;
    cmd_buff->argc = ccount;
    if (ccount == 0) return WARN_NO_CMDS;
    return OK;
}

int exec_local_cmd_loop()
{
    char *p = getenv("PATH");
    if (!p || strlen(p) == 0) setenv("PATH", "/usr/bin:/bin", 1);

    char *linebuf = malloc(SH_CMD_MAX);
    if (!linebuf) {
        fprintf(stderr, "Failed to allocate memory\n");
        return ERR_MEMORY;
    }

    cmd_buff_t cmd;
    int st;
    while (1) {
        printf("%s", SH_PROMPT);
        if (!fgets(linebuf, SH_CMD_MAX, stdin)) {
            printf("\n");
            break;
        }
        linebuf[strcspn(linebuf, "\n")] = '\0';
        if (strlen(linebuf) == 0) {
            printf(CMD_WARN_NO_CMD "\n");
            continue;
        }
        int rc = build_cmd_buff(linebuf, &cmd);
        if (rc == WARN_NO_CMDS) {
            printf(CMD_WARN_NO_CMD "\n");
            continue;
        }
        if (rc != OK) continue;
        if (strcmp(cmd.argv[0], EXIT_CMD) == 0) {
            free(linebuf);
            exit(0);
        }
        if (strcmp(cmd.argv[0], "cd") == 0) {
            if (cmd.argc > 1) {
                if (chdir(cmd.argv[1]) != 0) perror("cd failed");
            } else {
                if (chdir("/tmp") != 0) perror("cd failed");
            }
            continue;
        }
        pid_t cpid = fork();
        if (cpid < 0) {
            perror("fork error");
            continue;
        }
        if (cpid == 0) {
            if (execvp(cmd.argv[0], cmd.argv) < 0) {
                perror("execvp failed");
                exit(1);
            }
        } else {
            waitpid(cpid, &st, 0);
        }
    }
    free(linebuf);
    return OK;
}
