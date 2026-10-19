#include "mysh.h"
#include <assert.h>

void test_read_line() {
    reset_read_line();
    int fd = open("shellTest1.sh", O_RDONLY);
    char buf[MAX_LINE];
    char *str = "echo hello";

    assert(read_line(fd, buf, sizeof(buf)) == 1);
    assert(strcmp(buf, str) == 0);

    close(fd);
    printf("Read Line tests passed\n"); 
}

void test_tokenize() {
    reset_read_line();
    Token tokens[MAX_TOKENS];
    int n = tokenize("ls -l | wc", tokens, MAX_TOKENS);

    assert(n == 4);
    assert(tokens[0].type == TOK_WORD);
    assert(tokens[2].type == TOK_PIPE);

    printf("Tokenize tests passed\n");
}

void test_parse_job() {
    reset_read_line();
    Token tokens[MAX_TOKENS];
    int n = tokenize("ls -l | grep .c > out.txt", tokens, MAX_TOKENS);
    Job *job = parse_job(tokens, n);

    assert(job != NULL);
    assert(job->num_cmds == 2);
    assert(job->cmds[0].argc == 2);
    assert(strcmp(job->cmds[0].argv[0], "ls") == 0);
    assert(job->cmds[1].argc == 2);
    assert(job->cmds[1].output_file != NULL);
    assert(strcmp(job->cmds[1].output_file, "out.txt") == 0);

    free_job(job);
    printf("Parse Job tests passed\n"); 
}

void test_builtin() {
    reset_read_line();
    assert(is_builtin("cd") == BUILTIN_CD);
    assert(is_builtin("pwd") == BUILTIN_PWD);
    assert(is_builtin("which") == BUILTIN_WHICH);
    assert(is_builtin("exit") == BUILTIN_EXIT);
    assert(is_builtin("ls") == BUILTIN_NONE); 
    printf("Builtin tests passed\n");     
}

void test_resolve_path() {
    reset_read_line();
    char path[MAX_LINE];
    int found = resolve_path("ls", path, MAX_LINE);

    assert(found == 1);
    assert(strstr(path, "/ls") != NULL); 
    assert(resolve_path("fakecommand123", path, MAX_LINE) == -1);

    printf("Resolve tests passed\n");
}

void test_execute_command() {
    reset_read_line();
    char old_cwd[MAX_LINE];
    char new_cwd[MAX_LINE];
    getcwd(old_cwd, MAX_LINE);
    Command cmd;
    cmd.argc = 2;
    cmd.argv = malloc(3 * sizeof(char*));
    cmd.argv[0] = strdup("cd");
    cmd.argv[1] = strdup("..");
    cmd.argv[2] = NULL;
    cmd.builtin = BUILTIN_CD;
    execute_command(&cmd, 0, STDIN_FILENO, STDOUT_FILENO);
    getcwd(new_cwd, MAX_LINE);

    assert(strcmp(old_cwd, new_cwd) != 0);

    chdir(old_cwd); 
    free(cmd.argv[0]); free(cmd.argv[1]); free(cmd.argv);
    printf("Execute Command tests passed\n"); 
}

void test_execute_job() {
    reset_read_line();
    Job job;
    job.num_cmds = 1;
    job.cmds = calloc(1, sizeof(Command));
    job.cmds[0].argc = 2;
    job.cmds[0].argv = malloc(3 * sizeof(char*));
    job.cmds[0].argv[0] = strdup("echo");
    job.cmds[0].argv[1] = strdup("hello world");
    job.cmds[0].argv[2] = NULL;
    job.cmds[0].output_file = strdup("test_out.txt");
    job.cmds[0].builtin = BUILTIN_NONE;
    resolve_path("echo", job.cmds[0].resolved_path, MAX_LINE);
    execute_job(&job, 0);
    FILE *fp = fopen("test_out.txt", "r");

    assert(fp != NULL);
    char results[100];
    fgets(results, sizeof(results), fp);
    assert(strstr(results, "hello world") != NULL);
    
    fclose(fp);
    unlink("test_out.txt");
    free(job.cmds[0].argv[0]); free(job.cmds[0].argv[1]); 
    free(job.cmds[0].argv); free(job.cmds[0].output_file);
    free(job.cmds);
    printf("Execute Job tests passed\n"); 
}

int main() {
    test_read_line();
    test_tokenize();
    test_parse_job();
    test_builtin();
    test_resolve_path();
    test_execute_command();
    test_execute_job();
    return EXIT_SUCCESS;
}