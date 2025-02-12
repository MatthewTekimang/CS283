#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
static char* trim(char* str);
static char* trim(char* str) {
    char* end;
    
    while(str != NULL && isspace((unsigned char)*str)) str++;    
    if(*str == 0) return str;
    
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    return str;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *cmd_copy = strdup(cmd_line);
    char *saveptr1, *saveptr2; 
    char *cmd_part;
    int cmd_count = 0;

    if (!cmd_copy) {
        return ERR_CMD_OR_ARGS_TOO_BIG;
    }
    
    memset(clist, 0, sizeof(command_list_t));
    
    cmd_part = strtok_r(cmd_copy, "|", &saveptr1);
    
    while (cmd_part != NULL) {
        if (cmd_count >= CMD_MAX) {
            free(cmd_copy);
            return ERR_TOO_MANY_COMMANDS;
        }
        
        char *trimmed_cmd = trim(cmd_part);
        char *token = strtok_r(trimmed_cmd, " \t", &saveptr2);
        if (token != NULL) {
            if (strlen(token) >= EXE_MAX) {
                free(cmd_copy);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            
            strcpy(clist->commands[cmd_count].exe, token);
            
            char args_buffer[ARG_MAX] = "";
            char *arg = strtok_r(NULL, " \t", &saveptr2);
            while (arg != NULL) {
                if (strlen(args_buffer) > 0) {
                    strcat(args_buffer, " ");
                }
                strcat(args_buffer, arg);
                arg = strtok_r(NULL, " \t", &saveptr2);
            }
            if (strlen(args_buffer) >= ARG_MAX) {
                free(cmd_copy);
                return ERR_CMD_OR_ARGS_TOO_BIG;
            }
            if (strlen(args_buffer) > 0) {
                strcpy(clist->commands[cmd_count].args, args_buffer);
            }
            cmd_count++;
        }
        cmd_part = strtok_r(NULL, "|", &saveptr1);
    }
    clist->num = cmd_count;
    free(cmd_copy);
    return OK;
}