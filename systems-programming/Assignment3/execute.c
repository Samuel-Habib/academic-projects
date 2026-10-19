#include "mysh.h"

//execute_command
/*
 * If the command is only cd, we check argv for location
 * Otherwise, we call fork() and arrange the ends of the pipe
 * We then call execv(), and run the program
 * We check exit status, and exit signal otherwise for termination
 */
void execute_command(Command *cmd, int isInteractive, int in_fd, int out_fd) {
    //Handle cd command
    if (in_fd == STDIN_FILENO && out_fd == STDOUT_FILENO) {
        if (cmd->builtin == BUILTIN_CD) {
            if (cmd->argc > 2)
                fprintf(stderr, "Too many arguments.\n");
            else {
                char *path;
                if (cmd->argc == 1) 
                    path = getenv("HOME");
                else 
                    path = cmd->argv[1];
                if (chdir(path) != 0)
                    perror("cd");
            }
            return;
        }
    }

    //Others
    pid_t pid = fork();
    if (pid == 0) {
        if (cmd->input_file) {
            int fd = open(cmd->input_file, O_RDONLY);
            if (fd < 0) {
                perror("open input");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        else if (in_fd != STDIN_FILENO)
            dup2(in_fd, STDIN_FILENO);
        else if (!isInteractive) {
            int dev_null = open("/dev/null", O_RDONLY);
            dup2(dev_null, STDIN_FILENO);
            close(dev_null);
        }
        
        if (cmd->output_file) {
            int fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0640);
            if (fd < 0) {
                perror("open output");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);            
        }
        else if (out_fd != STDOUT_FILENO)
            dup2(out_fd, STDOUT_FILENO);

        if (cmd->builtin == BUILTIN_PWD) {
            if (cmd->argc > 1) {
                fprintf(stderr, "pwd: too many arguments\n");
                exit(EXIT_FAILURE);
            }            
            char cwd[MAX_LINE];
            if (getcwd(cwd, sizeof(cwd)))
                printf("%s\n", cwd);
            exit(EXIT_SUCCESS);
        }
        else if (cmd->builtin == BUILTIN_WHICH) {
            if (cmd->argc != 2) {
                fprintf(stderr, "which: incorrect number of arguments\n");
                exit(EXIT_FAILURE);
            }            
            char path[MAX_LINE];
            if (resolve_path(cmd->argv[1], path, MAX_LINE) == 1) {
                printf("%s\n", path);
                exit(EXIT_SUCCESS);
            }
            exit(EXIT_FAILURE);
        }

        if (cmd->resolved_path[0] != '\0') {
            execv(cmd->resolved_path, cmd->argv);
            perror("execv");
        }
        exit(EXIT_FAILURE);
    }
    else {
        int status;
        waitpid(pid, &status, 0);
        if (isInteractive && out_fd == STDOUT_FILENO) {
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
                printf("Exited with status %d\n", WEXITSTATUS(status));
            else if (WIFSIGNALED(status))
                printf("Terminated by signal %d: %s\n", WTERMSIG(status), strsignal(WTERMSIG(status)));
        }
    }
}

//execute_job
/*
 * This loops through each command in a job and keeps track of the previous command run using an array of fd
 */
void execute_job(Job *job, int isInteractive) {
    int prev_pipe = STDIN_FILENO;

    for (int i = 0; i< job->num_cmds; i++) {
        //Setup
        int fd[2];
        int current_out = STDOUT_FILENO;

        //Makes sure we aren't at the last command
        if (i < job->num_cmds - 1) {
            if (pipe(fd) == -1) {
                perror("pipe");
                return;
            }
            current_out = fd[1];
        } 

        //Execution
        execute_command(&job->cmds[i], isInteractive, prev_pipe, current_out);

        //Closing, return
        if (prev_pipe != STDIN_FILENO) 
            close(prev_pipe);
        if (current_out != STDOUT_FILENO)
            close(current_out);
        prev_pipe = fd[0];
    }
}