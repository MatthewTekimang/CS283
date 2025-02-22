1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  _we use fork() before execvp() so the shell remains running while the new command executes in a separate child process. Without fork(), execvp() would replace the shell’s memory and end the shell._

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  _if fork() fails, the code will print an error and skip creating a child. What my implimentation does is  checks the return value, and if it’s negative, it will handle it by printing “fork error”._

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  _execvp() searches through the directoreis listed in the path environment, trying everything until it finds the requested command._

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  _purpose of calling wait() is to prevent the child from becoming a zombie and to let the shell gather the childs exit status. Without it, the child would finish but remain in the process table._

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  _by calling WEXITSTATUS() we extracts the childs exit code from the status returned by wait(), letting us know if the command succeeded (0) or failed (non-zero)_

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  _it is necessary so spaces within quotes don’t split the argument. This allows commands like echo "Hello World" to treat the text as one argument._

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  _Changes made to the parshing is to handle only one command at a time and carefully removed quotes with memmove(). The unexpected challenge was avoiding edge cases like trailing spaces._

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  _Signals provide asynchronous notifications for events like interrupts or termination, differing from IPC by carrying minimal information and requiring no open channel._

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  _SIGINT- interrupts with(Ctrl+C), SIGTERM- requests termination, SIGKILL- forcibly kills program._

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  _it will suspends a process immediately and won't be caught or ignored, allowing the kernel to pause any process unconditionally._