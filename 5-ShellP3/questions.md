1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

_My implimentation uses waitpid() to ensure that all child processes complete, if I forgot to call waitpid(), all child processes would remain in the process table, which will end up causing resource depletion._

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

_It would be necessary to close unused pipe ends after calling dup2() to prevents full stop where processes wait for EOF signals that never come. Open pipes also consume file descriptors, which are limited system resources._

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

_The cd command is implimented as a built-in because it needs to change the shell process's own working directory. An external command would runs in a separate child process with its own environment and working directory._

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

_I would replace fixed arrays with dynamically allocated linked lists. This removes the command limit but requires more complex memory management and error handling._