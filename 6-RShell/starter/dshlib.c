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
 
 /*
  * Global variable to track the last return code
  * This can be used by built-in commands like 'rc'
  */
 static int last_return_code = 0;
 
 /* 
  * Helper function to trim leading and trailing whitespace
  */
 static char* trim(char* str) {
     if (!str) return NULL;
     
     // Trim leading space
     while(isspace((unsigned char)*str)) str++;
     if(*str == 0) return str;
     
     // Trim trailing space
     char* end = str + strlen(str) - 1;
     while(end > str && isspace((unsigned char)*end)) end--;
     end[1] = '\0';
     
     return str;
 }
 
 /*
  * Allocates memory for a command buffer
  * Returns OK on success, ERR_MEMORY on failure
  */
 int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
     if (!cmd_buff) return ERR_MEMORY;
     
     cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
     if (!cmd_buff->_cmd_buffer) return ERR_MEMORY;
     
     cmd_buff->argc = 0;
     memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
     
     return OK;
 }
 
 /*
  * Frees memory allocated for a command buffer
  * Returns OK on success, ERR_MEMORY on failure
  */
 int free_cmd_buff(cmd_buff_t *cmd_buff) {
     if (!cmd_buff) return ERR_MEMORY;
     
     if (cmd_buff->_cmd_buffer) {
         free(cmd_buff->_cmd_buffer);
         cmd_buff->_cmd_buffer = NULL;
     }
     
     cmd_buff->argc = 0;
     
     return OK;
 }
 
 /*
  * Clears a command buffer for reuse
  * Initializes all fields including redirection fields
  * Returns OK on success, ERR_MEMORY on failure
  */
 int clear_cmd_buff(cmd_buff_t *cmd_buff) {
     if (!cmd_buff || !cmd_buff->_cmd_buffer) return ERR_MEMORY;
     
     memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
     cmd_buff->argc = 0;
     memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
     
     // Initialize redirection fields
     cmd_buff->input_file = NULL;
     cmd_buff->output_file = NULL;
     cmd_buff->append_output = false;
     
     return OK;
 }
 
 /*
  * Builds a command buffer from a command line string
  * Parses the command line into argc/argv format
  * Detects and processes redirection operators (<, >, >>)
  * Returns OK on success, ERR_MEMORY on failure
  */
 int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
     if (!cmd_line || !cmd_buff) return ERR_MEMORY;
     
     clear_cmd_buff(cmd_buff);
     strcpy(cmd_buff->_cmd_buffer, cmd_line);
     
     char *str = cmd_buff->_cmd_buffer;
     int i = 0;
     
     // Initialize redirection fields
     cmd_buff->input_file = NULL;
     cmd_buff->output_file = NULL;
     cmd_buff->append_output = false;
     
     // Parse each token
     char *token = strtok(str, " \t");
     
     while (token != NULL && i < CMD_ARGV_MAX - 1) {
         // Check for input redirection
         if (strcmp(token, "<") == 0) {
             token = strtok(NULL, " \t");
             if (token != NULL) {
                 cmd_buff->input_file = token;
                 token = strtok(NULL, " \t");
             }
             continue;
         }
         
         // Check for output redirection with append
         if (strcmp(token, ">>") == 0) {
             token = strtok(NULL, " \t");
             if (token != NULL) {
                 cmd_buff->output_file = token;
                 cmd_buff->append_output = true;
                 token = strtok(NULL, " \t");
             }
             continue;
         }
         
         // Check for output redirection
         if (strcmp(token, ">") == 0) {
             token = strtok(NULL, " \t");
             if (token != NULL) {
                 cmd_buff->output_file = token;
                 cmd_buff->append_output = false;
                 token = strtok(NULL, " \t");
             }
             continue;
         }
         
         // Regular argument
         cmd_buff->argv[i++] = token;
         token = strtok(NULL, " \t");
     }
     
     cmd_buff->argc = i;
     cmd_buff->argv[i] = NULL;  // Ensure NULL termination for execvp
     
     return OK;
 }
 
 /*
  * Frees resources associated with a command list
  * Returns OK on success, error code on failure
  */
 int free_cmd_list(command_list_t *cmd_lst) {
     if (!cmd_lst) return ERR_MEMORY;
     
     for (int i = 0; i < cmd_lst->num; i++) {
         free_cmd_buff(&cmd_lst->commands[i]);
     }
     
     return OK;
 }
 
 /*
  * Builds a command list from a command line containing pipes
  * Returns OK on success, appropriate error code on failure
  */
 int build_cmd_list(char *cmd_line, command_list_t *clist) {
     if (!cmd_line || !clist) return ERR_MEMORY;
     
     // Initialize command list
     memset(clist, 0, sizeof(command_list_t));
     clist->num = 0;
     
     // Trim input and check if empty
     char *trimmed_cmd = trim(cmd_line);
     if (strlen(trimmed_cmd) == 0) {
         return WARN_NO_CMDS;
     }
     
     // Make a copy of the command line
     char *cmd_copy = strdup(trimmed_cmd);
     if (!cmd_copy) return ERR_MEMORY;
     
     // Split by pipe character
     char *cmd_str;
     char *saveptr;
     int cmd_idx = 0;
     
     for (cmd_str = strtok_r(cmd_copy, PIPE_STRING, &saveptr); 
          cmd_str != NULL && cmd_idx < CMD_MAX; 
          cmd_str = strtok_r(NULL, PIPE_STRING, &saveptr)) {
         
         // Allocate buffer for this command
         if (alloc_cmd_buff(&clist->commands[cmd_idx]) != OK) {
             free(cmd_copy);
             return ERR_MEMORY;
         }
         
         // Build command buffer from this segment
         char *trimmed_segment = trim(cmd_str);
         if (strlen(trimmed_segment) > 0) {
             if (build_cmd_buff(trimmed_segment, &clist->commands[cmd_idx]) != OK) {
                 free(cmd_copy);
                 return ERR_MEMORY;
             }
             cmd_idx++;
         }
     }
     
     free(cmd_copy);
     
     // Check if too many commands
     if (cmd_str != NULL && cmd_idx >= CMD_MAX) {
         printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
         return ERR_TOO_MANY_COMMANDS;
     }
     
     // Update command count
     clist->num = cmd_idx;
     
     // Check if any commands were found
     if (clist->num == 0) {
         return WARN_NO_CMDS;
     }
     
     return OK;
 }
 
 /*
  * Identifies if a command is a built-in command
  * Returns the built-in command type or BI_NOT_BI if not a built-in
  */
 Built_In_Cmds match_command(const char *input) {
     if (!input) return BI_NOT_BI;
     
     if (strcmp(input, EXIT_CMD) == 0) return BI_CMD_EXIT;
     if (strcmp(input, "dragon") == 0) return BI_CMD_DRAGON;
     if (strcmp(input, "cd") == 0) return BI_CMD_CD;
     
     return BI_NOT_BI;
 }
 
 /*
  * Executes a built-in command
  * Returns BI_EXECUTED if executed, BI_NOT_BI if not a built-in
  */
 Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {
     if (!cmd || !cmd->argv[0]) return BI_NOT_BI;
     
     Built_In_Cmds cmd_type = match_command(cmd->argv[0]);
     
     switch(cmd_type) {
         case BI_CMD_EXIT:
             printf("exiting...\n");
             exit(EXIT_SC);
             
         case BI_CMD_CD:
             if (cmd->argc > 1) {
                 if (chdir(cmd->argv[1]) != 0) {
                     perror("cd failed");
                 }
             } else {
                 // cd with no args goes to home directory
                 if (chdir(getenv("HOME")) != 0) {
                     perror("cd failed");
                 }
             }
             return BI_EXECUTED;
             
         case BI_CMD_DRAGON:
             printf("Roar! The dragon breathes fire!\n");
             return BI_EXECUTED;
             
         default:
             return BI_NOT_BI;
     }
 }
 
 /*
  * Executes a single command (non-piped)
  * Returns OK on success, ERR_EXEC_CMD on failure
  */
 int exec_cmd(cmd_buff_t *cmd) {
     if (!cmd || !cmd->argv[0]) return ERR_EXEC_CMD;
     
     pid_t pid = fork();
     
     if (pid < 0) {
         perror("Fork failed");
         return ERR_EXEC_CMD;
     }
     
     if (pid == 0) {  // Child process
         execvp(cmd->argv[0], cmd->argv);
         // If we get here, execvp failed
         perror("Command execution failed");
         exit(ERR_EXEC_CMD);
     } else {  // Parent process
         int status;
         waitpid(pid, &status, 0);
         
         if (WIFEXITED(status)) {
             last_return_code = WEXITSTATUS(status);
             return last_return_code;
         }
         
         return ERR_EXEC_CMD;
     }
 }
 
 /*
  * Executes a pipeline of commands
  * Handles both piping and file redirection (<, >, >>)
  * Returns OK on success, appropriate error code on failure
  */
 int execute_pipeline(command_list_t *clist) {
     if (!clist || clist->num == 0) return WARN_NO_CMDS;
     
     // Handle built-in commands (only for the first command in pipeline)
     if (clist->num == 1) {
         Built_In_Cmds result = exec_built_in_cmd(&clist->commands[0]);
         if (result == BI_EXECUTED) {
             return OK;
         }
     }
     
     int pipes[CMD_MAX-1][2]; // Array of pipe file descriptors
     pid_t child_pids[CMD_MAX]; // Array to store child process IDs
     
     // Create pipes
     for (int i = 0; i < clist->num - 1; i++) {
         if (pipe(pipes[i]) == -1) {
             perror("Pipe creation failed");
             return ERR_EXEC_CMD;
         }
     }
     
     // Create child processes and set up pipes
     for (int i = 0; i < clist->num; i++) {
         child_pids[i] = fork();
         
         if (child_pids[i] < 0) {
             perror("Fork failed");
             return ERR_EXEC_CMD;
         }
         
         if (child_pids[i] == 0) {
             // Child process
             
             // Handle input redirection from file (only for first command)
             if (i == 0 && clist->commands[i].input_file != NULL) {
                 int input_fd = open(clist->commands[i].input_file, O_RDONLY);
                 if (input_fd == -1) {
                     perror("Failed to open input file");
                     exit(ERR_EXEC_CMD);
                 }
                 if (dup2(input_fd, STDIN_FILENO) == -1) {
                     perror("dup2 input file redirection failed");
                     exit(ERR_EXEC_CMD);
                 }
                 close(input_fd);
             } 
             // Handle input from previous pipe (for commands after the first)
             else if (i > 0) {
                 if (dup2(pipes[i-1][0], STDIN_FILENO) == -1) {
                     perror("dup2 pipe input redirection failed");
                     exit(ERR_EXEC_CMD);
                 }
             }
             
             // Handle output redirection to file (only for last command)
             if (i == clist->num - 1 && clist->commands[i].output_file != NULL) {
                 int flags = O_WRONLY | O_CREAT;
                 if (clist->commands[i].append_output) {
                     flags |= O_APPEND;  // Use append mode for >>
                 } else {
                     flags |= O_TRUNC;   // Truncate file for >
                 }
                 
                 int output_fd = open(clist->commands[i].output_file, flags, 0644);
                 if (output_fd == -1) {
                     perror("Failed to open output file");
                     exit(ERR_EXEC_CMD);
                 }
                 if (dup2(output_fd, STDOUT_FILENO) == -1) {
                     perror("dup2 output file redirection failed");
                     exit(ERR_EXEC_CMD);
                 }
                 close(output_fd);
             } 
             // Handle output to next pipe (for commands before the last)
             else if (i < clist->num - 1) {
                 if (dup2(pipes[i][1], STDOUT_FILENO) == -1) {
                     perror("dup2 pipe output redirection failed");
                     exit(ERR_EXEC_CMD);
                 }
             }
             
             // Close all pipe file descriptors
             for (int j = 0; j < clist->num - 1; j++) {
                 close(pipes[j][0]);
                 close(pipes[j][1]);
             }
             
             // Execute command
             execvp(clist->commands[i].argv[0], clist->commands[i].argv);
             
             // If we get here, execvp failed
             perror("Command execution failed");
             exit(ERR_EXEC_CMD);
         }
     }
     
     // Parent process
     
     // Close all pipe file descriptors in the parent
     for (int i = 0; i < clist->num - 1; i++) {
         close(pipes[i][0]);
         close(pipes[i][1]);
     }
     
     // Wait for all child processes to complete
     int status;
     int last_status = 0;
     
     for (int i = 0; i < clist->num; i++) {
         waitpid(child_pids[i], &status, 0);
         if (WIFEXITED(status)) {
             last_status = WEXITSTATUS(status);
         }
     }
     
     last_return_code = last_status;
     return OK;
 }
 
 /*
  * Main command loop for the shell
  * Prompts for and processes user input until exit
  * Returns OK on normal exit
  */
 int exec_local_cmd_loop() {
     char cmd_buff[SH_CMD_MAX];
     command_list_t cmd_list;
     int rc;
     
     while (1) {
         // Display prompt
         printf("%s", SH_PROMPT);
         
         // Get user input
         if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL) {
             printf("\n");
             break;
         }
         
         // Remove trailing newline
         cmd_buff[strcspn(cmd_buff, "\n")] = '\0';
         
         // Check for exit command (quick check before parsing)
         if (strcmp(trim(cmd_buff), EXIT_CMD) == 0) {
             printf("exiting...\n");
             return OK;
         }
         
         // Parse the command line into a command list
         rc = build_cmd_list(cmd_buff, &cmd_list);
         
         if (rc == WARN_NO_CMDS) {
             // Empty input, just continue
             continue;
         } else if (rc == ERR_TOO_MANY_COMMANDS) {
             // Too many commands, already printed error message
             continue;
         } else if (rc != OK) {
             // Other error
             fprintf(stderr, "Error parsing command\n");
             continue;
         }
         
         // Execute the pipeline
         rc = execute_pipeline(&cmd_list);
         
         // Free resources
         free_cmd_list(&cmd_list);
     }
     
     return OK;
 }