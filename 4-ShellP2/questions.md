1. Can you think of why we use `fork/execvp` instead of just calling `execvp` directly? What value do you think the `fork` provides?

    > **Answer**:  Using fork/execvp instead of calling execvp directly allows the shell to create a separate child process in which to execute the new command. This way, the parent process (the shell) continues running and can accept further input, while the child process replaces its memory with the new program. Without fork, calling execvp directly would replace the shell process, terminating the interactive session.

2. What happens if the fork() system call fails? How does your implementation handle this scenario?

    > **Answer**:  If fork() fails, it returns -1, indicating that no child process was created. In our implementation, we check for a negative return value and use perror("fork") to print an error message. The shell then continues its loop without attempting to execute the command, ensuring that the failure is handled gracefully.

3. How does execvp() find the command to execute? What system environment variable plays a role in this process?

    > **Answer**:  The execvp() function searches for the command to execute by scanning the directories listed in the PATH environment variable. This environment variable contains a colon-separated list of directories where executable files are located, allowing execvp() to locate and run the command if an absolute path isn’t provided.

4. What is the purpose of calling wait() in the parent process after forking? What would happen if we didn’t call it?

    > **Answer**:  The call to wait() in the parent process ensures that the shell waits for the child process to finish executing before continuing. This is important because it allows the parent to retrieve the child’s exit status and prevents the creation of zombie processes. If we didn’t call wait(), the parent could finish executing other commands or exit before the child process, leading to orphaned or zombie processes.

5. In the referenced demo code we used WEXITSTATUS(). What information does this provide, and why is it important?

    > **Answer**:  WEXITSTATUS() is used to extract the exit status (return code) of the child process after it has terminated. This information is crucial because it tells the parent process whether the command executed successfully or if an error occurred, enabling the shell (and built-in commands like rc) to provide appropriate feedback or error messages.

6. Describe how your implementation of build_cmd_buff() handles quoted arguments. Why is this necessary?

    > **Answer**:  In our implementation of build_cmd_buff() (or the parsing routine), when the parser encounters a quoted argument (i.e., a string that begins with a double quote), it treats everything until the closing double quote as a single argument—even if it contains spaces. This is necessary because many commands require arguments that include spaces (for example, a greeting like "hello, world") and these spaces should not be treated as delimiters.

7. What changes did you make to your parsing logic compared to the previous assignment? Were there any unexpected challenges in refactoring your old code?

    > **Answer**:  Eliminated handling of multiple piped commands and instead focus on a single command line.
Handled quoted arguments so that spaces within quotes are preserved.
Removed duplicate spaces outside of quotes.

8. For this quesiton, you need to do some research on Linux signals. You can use [this google search](https://www.google.com/search?q=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&oq=Linux+signals+overview+site%3Aman7.org+OR+site%3Alinux.die.net+OR+site%3Atldp.org&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBBzc2MGowajeoAgCwAgA&sourceid=chrome&ie=UTF-8) to get started.

- What is the purpose of signals in a Linux system, and how do they differ from other forms of interprocess communication (IPC)?

    > **Answer**:  Signals provide a way for processes to receive asynchronous notifications about events such as interruptions, terminations, or other system-level events. Unlike other IPC methods (e.g., pipes, sockets, shared memory) which are used for sending larger amounts of data or coordinating between processes, signals are designed to quickly notify a process that an event has occurred, without transferring data.

- Find and describe three commonly used signals (e.g., SIGKILL, SIGTERM, SIGINT). What are their typical use cases?

    > **Answer**:  SIGKILL: Immediately terminates a process. It cannot be caught, blocked, or ignored and is used when a process must be forcibly stopped. SIGTERM: Requests graceful termination of a process. The process can catch this signal to perform cleanup before exiting. SIGINT: Issued when the user types Ctrl+C in the terminal, signaling the process to interrupt its current operation. This signal can be caught, allowing the process to handle the interrupt in a controlled manner.

- What happens when a process receives SIGSTOP? Can it be caught or ignored like SIGINT? Why or why not?

    > **Answer**:  When a process receives SIGSTOP, it is immediately suspended by the operating system. Unlike SIGINT, SIGSTOP cannot be caught, blocked, or ignored because it is designed as a definitive way for the system to pause a process regardless of the process’s own handling routines.
