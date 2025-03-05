#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
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

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (!cmd_buff->_cmd_buffer) {
        return ERR_MEMORY;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff->_cmd_buffer) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
    }
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;
    }
    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    if (!cmd_line || !cmd_buff) return ERR_MEMORY;
    
    if (cmd_buff->_cmd_buffer) {
        strcpy(cmd_buff->_cmd_buffer, cmd_line);
    } else {
        cmd_buff->_cmd_buffer = strdup(cmd_line);
        if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;
    }

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

int parse_cmd_line(char *line, command_list_t *clist) {
    char *token;
    char *rest = line;
    int count = 0;
    char *cmd_line_copy = strdup(line);
    
    if (!cmd_line_copy) return ERR_MEMORY;
    
    clist->num = 0;
    
    bool in_quotes = false;
    char *cmd_start = cmd_line_copy;
    char *current = cmd_line_copy;
    
    while (*current) {
        if (*current == '"') {
            in_quotes = !in_quotes;
        } else if (*current == PIPE_CHAR && !in_quotes) {
            *current = '\0';
            
            if (count >= CMD_MAX) {
                printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
                free(cmd_line_copy);
                return ERR_TOO_MANY_COMMANDS;
            }
            
            int rc = alloc_cmd_buff(&clist->commands[count]);
            if (rc != OK) {
                for (int i = 0; i < count; i++) {
                    free_cmd_buff(&clist->commands[i]);
                }
                free(cmd_line_copy);
                return ERR_MEMORY;
            }
            
            rc = build_cmd_buff(cmd_start, &clist->commands[count]);
            if (rc != OK && rc != WARN_NO_CMDS) {
                for (int i = 0; i <= count; i++) {
                    free_cmd_buff(&clist->commands[i]);
                }
                free(cmd_line_copy);
                return rc;
            }
            
            if (rc != WARN_NO_CMDS) {
                count++;
            } else {
                free_cmd_buff(&clist->commands[count]);
            }
            
            cmd_start = current + 1;
            while (*cmd_start && isspace((unsigned char)*cmd_start)) cmd_start++;
        }
        current++;
    }
    
    if (count < CMD_MAX) {
        int rc = alloc_cmd_buff(&clist->commands[count]);
        if (rc != OK) {
            for (int i = 0; i < count; i++) {
                free_cmd_buff(&clist->commands[i]);
            }
            free(cmd_line_copy);
            return ERR_MEMORY;
        }
        
        rc = build_cmd_buff(cmd_start, &clist->commands[count]);
        if (rc != OK && rc != WARN_NO_CMDS) {
            for (int i = 0; i <= count; i++) {
                free_cmd_buff(&clist->commands[i]);
            }
            free(cmd_line_copy);
            return rc;
        }
        
        if (rc != WARN_NO_CMDS) {
            count++;
        } else {
            free_cmd_buff(&clist->commands[count]);
        }
    }
    
    free(cmd_line_copy);
    
    clist->num = count;
    
    if (count == 0) {
        printf(CMD_WARN_NO_CMD);
        return WARN_NO_CMDS;
    }
    
    return OK;
}

int freeCmd(command_list_t *clist) {
    for (int i = 0; i < clist->num; i++) {
        free_cmd_buff(&clist->commands[i]);
    }
    clist->num = 0;
    return OK;
}

int exec_cmd(cmd_buff_t *cmd) {
    pid_t pid;
    int status;
    int return_code = 0;
    
    if (cmd->argc > 0) {
        if (strcmp(cmd->argv[0], "cd") == 0) {
            if (cmd->argc > 1) {
                if (chdir(cmd->argv[1]) != 0) {
                    perror("cd");
                    return errno;
                }
            } else {
                if (chdir("/tmp") != 0) {
                    perror("cd");
                    return errno;
                }
            }
            return 0;
        } else if (strcmp(cmd->argv[0], "rc") == 0) {
            return 0;
        }
    }
    
    pid = fork();
    if (pid < 0) {
        perror("fork");
        return errno;
    } else if (pid == 0) {
        execvp(cmd->argv[0], cmd->argv);
        
        switch (errno) {
            case ENOENT:
                fprintf(stderr, "Command not found in PATH\n");
                exit(2);
            case EACCES:
                fprintf(stderr, "Permission denied\n");
                exit(13);
            case ENOEXEC:
                fprintf(stderr, "Not an executable file\n");
                exit(126);
            default:
                perror("Command execution failed");
                exit(errno);
        }
    } else {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            return_code = WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            return_code = 128 + WTERMSIG(status);
        }
    }
    
    return return_code;
}

int execute_pipeline(command_list_t *clist) {
    int num_commands = clist->num;
    int pipes[CMD_MAX-1][2]; 
    pid_t pids[CMD_MAX];     
    int status;
    int last_return_code = 0;
    
    if (num_commands == 1) {
        return exec_cmd(&clist->commands[0]);
    }
    
    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            return ERR_EXEC_CMD;
        }
    }
    
    for (int i = 0; i < num_commands; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            return ERR_EXEC_CMD;
        }
        
        if (pids[i] == 0) {
            if (i > 0) {
                if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                    perror("dup2 stdin");
                    exit(errno);
                }
            }
            
            if (i < num_commands - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                    perror("dup2 stdout");
                    exit(errno);
                }
            }
            
            for (int j = 0; j < num_commands - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            
            switch (errno) {
                case ENOENT:
                    fprintf(stderr, "Command not found in PATH\n");
                    exit(2);
                case EACCES:
                    fprintf(stderr, "Permission denied\n");
                    exit(13);
                case ENOEXEC:
                    fprintf(stderr, "Not an executable file\n");
                    exit(126);
                default:
                    perror("Command execution failed");
                    exit(errno);
            }
        }
    }
    
    for (int i = 0; i < num_commands - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    for (int i = 0; i < num_commands; i++) {
        waitpid(pids[i], &status, 0);
        if (i == num_commands - 1) {
            if (WIFEXITED(status)) {
                last_return_code = WEXITSTATUS(status);
            } else if (WIFSIGNALED(status)) {
                last_return_code = 128 + WTERMSIG(status);
            }
        }
    }
    
    return last_return_code;
}

int exec_local_cmd_loop() {
    char *p = getenv("PATH");
    if (!p || strlen(p) == 0) setenv("PATH", "/usr/bin:/bin", 1);

    char *cmd_buff = malloc(SH_CMD_MAX);
    if (!cmd_buff) {
        fprintf(stderr, "Failed to allocate memory\n");
        return ERR_MEMORY;
    }

    command_list_t cmd_list;
    int last_return_code = 0;
    int rc;
    
    while (1) {
        printf("%s", SH_PROMPT);
        if (!fgets(cmd_buff, SH_CMD_MAX, stdin)) {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
        
        if (strlen(cmd_buff) == 0) {
            printf(CMD_WARN_NO_CMD "\n");
            continue;
        }
        
        if (strcmp(cmd_buff, EXIT_CMD) == 0) {
            printf("exiting...\n");
            free(cmd_buff);
            return OK;
        }
        
        rc = parse_cmd_line(cmd_buff, &cmd_list);
        if (rc < 0) {
            if (rc == WARN_NO_CMDS) {
                continue;
            } else {
                last_return_code = -rc;
                continue;
            }
        }
        
        if (cmd_list.num == 1) {
            if (strcmp(cmd_list.commands[0].argv[0], EXIT_CMD) == 0) {
                printf("exiting...\n");
                freeCmd(&cmd_list);
                free(cmd_buff);
                return OK;
            } else if (strcmp(cmd_list.commands[0].argv[0], "cd") == 0) {
                if (cmd_list.commands[0].argc > 1) {
                    if (chdir(cmd_list.commands[0].argv[1]) != 0) {
                        perror("cd");
                        last_return_code = errno;
                    } else {
                        last_return_code = 0;
                    }
                } else {
                    if (chdir("/tmp") != 0) {
                        perror("cd");
                        last_return_code = errno;
                    } else {
                        last_return_code = 0;
                    }
                }
                freeCmd(&cmd_list);
                continue;
            } else if (strcmp(cmd_list.commands[0].argv[0], "rc") == 0) {
                printf("%d\n", last_return_code);
                freeCmd(&cmd_list);
                continue;
            }
        }
        
        last_return_code = execute_pipeline(&cmd_list);
        
        freeCmd(&cmd_list);
    }
    
    free(cmd_buff);
    return OK;
}