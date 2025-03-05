1. Your shell forks multiple child processes when executing piped commands. How does your implementation ensure that all child processes complete before the shell continues accepting user input? What would happen if you forgot to call waitpid() on all child processes?

My implementation stores each child’s process ID in an array and then calls waitpid() for each child in a loop. This ensures that the parent process waits until every child in the pipeline has finished executing before moving on to accept new input. Without calling waitpid() on all terminated children, their process entries remain in the system’s process table in an unresolved state, which can lead to resource leakage and eventually exhaust available system resources.

2. The dup2() function is used to redirect input and output file descriptors. Explain why it is necessary to close unused pipe ends after calling dup2(). What could go wrong if you leave pipes open?

After using dup2() to redirect a file descriptor, it is necessary to close the original pipe ends because they still reference the same underlying pipe. If these unused file descriptors are left open, the reading or writing side of the pipe may never see an end-of-file condition, causing processes to hang indefinitely. Additionally, failing to close these descriptors can lead to resource leaks.

3. Your shell recognizes built-in commands (cd, exit, dragon). Unlike external commands, built-in commands do not require execvp(). Why is cd implemented as a built-in rather than an external command? What challenges would arise if cd were implemented as an external process?

The cd command is implemented as a built-in because it needs to change the current working directory of the shell process itself. If cd were executed as an external command via execvp(), it would run in a separate child process, and any change in the working directory would not affect the parent shell. This would make it impossible for the shell to update its own context based on directory changes.

4. Currently, your shell supports a fixed number of piped commands (CMD_MAX). How would you modify your implementation to allow an arbitrary number of piped commands while still handling memory allocation efficiently? What trade-offs would you need to consider?

To support an arbitrary number of piped commands, I would replace the fixed-size array with a dynamically allocated structure that can grow as needed—using functions like malloc() and realloc(). The trade-offs include increased complexity in managing dynamic memory (ensuring proper allocation, deallocation, and error handling) as well as the potential performance overhead from frequent reallocations if the number of commands is very large. Additionally, safeguards would be needed to prevent resource exhaustion in case a user enters an extremely high number of commands.
