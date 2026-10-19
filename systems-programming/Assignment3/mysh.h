#include <dirent.h>
#include <fcntl.h>
#include <glob.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 4096
#define MAX_TOKENS 256
#define MAX_CMDS 64

#define BUILTIN_NONE 0
#define BUILTIN_CD 1
#define BUILTIN_PWD 2
#define BUILTIN_WHICH 3
#define BUILTIN_EXIT 4

typedef enum {
  TOK_WORD,
  TOK_REDIR_IN,  /* < */
  TOK_REDIR_OUT, /* > */
  TOK_PIPE,      /* | */
} TokenType;

typedef struct {
  TokenType type;
  char value[MAX_LINE];
} Token;

typedef struct {
  char **argv; // null-terminated argument list
  int argc;
  char *input_file;             // NULL if none
  char *output_file;            // NULL if none
  int builtin;                  // BUILTIN_* id, or BUILTIN_NONE
  char resolved_path[MAX_LINE]; // full path for execv(); empty if builtin
} Command;

typedef struct {
  Command *cmds;
  int num_cmds;
} Job;

// input.c
int read_line(int fd, char *line_buf, int max_len);
void print_prompt(int interactive);
void reset_read_line();

// token.c
int tokenize(const char *line, Token *tokens, int max_tokens);
Job *parse_job(Token *tokens, int num_tokens);
void free_job(Job *job);

// resolve.c
int is_builtin(const char *name);
int resolve_path(const char *name, char *out_path, int out_len);

// execute.c
void execute_command(Command *cmd, int isInteractive, int in_fd, int out_fd);
void execute_job(Job *job, int isInteractive);
