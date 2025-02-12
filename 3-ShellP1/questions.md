1. In this assignment I suggested you use `fgets()` to get user input in the main while loop. Why is `fgets()` a good choice for this application?

    > **Answer**:  _fgets() would be a good choice since scanf() would not really set a maximum line length to prevent buffer overflows while fgets() allow us to read one line at a time which functions lioke how shell commands would._

2. You needed to use `malloc()` to allocte memory for `cmd_buff` in `dsh_cli.c`. Can you explain why you needed to do that, instead of allocating a fixed-size array?

    > **Answer**:  _we could use a fixed array if we know how much our command buffer exact size, else using malloc would be preferable as it allows us to optimize memory usage and preventing stack overflows by allocating on the heap rather than the stack mem._


3. In `dshlib.c`, the function `build_cmd_list(`)` must trim leading and trailing spaces from each command before storing it. Why is this necessary? If we didn't trim spaces, what kind of issues might arise when executing commands in our shell?

    > **Answer**:  _if we don't trim the space it could interfere with the shell commands like if "echo    hello" we would get "echo" command instead. by trimming space, we could prevent the shell from having unnecessary errors due to whitespace._

4. For this question you need to do some research on STDIN, STDOUT, and STDERR in Linux. We've learned this week that shells are "robust brokers of input and output". Google _"linux shell stdin stdout stderr explained"_ to get started.

- One topic you should have found information on is "redirection". Please provide at least 3 redirection examples that we should implement in our custom shell, and explain what challenges we might have implementing them.

    > **Answer**:  _1. echo "Hello, World!" > output.txt  -  Output Redirection must detects > first in the commands then it will open the file and go to write mode & then finally must use dup2() to redirect STDOUT so that the output is written in the file instead of the terminal.
 
                    2. echo "Check dir"; ls nonexistent_folder 2> error.log  -  Errors redirection exist when the shell detects 2> where it then extract the filename for error logging. Then it must open the file in write mode and use dup2() to redirect STDERR to the file so that it make sure that the error messages is captured instead of being displayed on the terminal.

                    3. echo "Sort txt"; sort < input.txt  -  Input redirection is when the shell detects < where it extract the file and oopen it in read mode and use dup2() to redirect STDIN so that the command reads input from the file._

- You should have also learned about "pipes". Redirection and piping both involve controlling input and output in the shell, but they serve different purposes. Explain the key differences between redirection and piping.

    > **Answer**:  _Redirection - Use case when working with files where the shell must open and close files. 
    				Piping - Use case when combining multiple commands to process data_

- STDERR is often used for error messages, while STDOUT is for regular output. Why is it important to keep these separate in a shell?

    > **Answer**:  _It's important to keep them sepreate so that they don't mix with the results making it hard to read or process the output._

- How should our custom shell handle errors from commands that fail? Consider cases where a command outputs both STDOUT and STDERR. Should we provide a way to merge them, and if so, how?

    > **Answer**:  _We should provide a way to merge STDOUT and STDERR so that both output & error message go to the same place. This would be useful for piping and debugging by preventing errors from being lost or breaking pipelines. we could do this by usingfirst merging them (2>&1) in the input then redirect STDERR to STDOUT before executing a command, so that STDERR would go to the same place as STDOUT._
