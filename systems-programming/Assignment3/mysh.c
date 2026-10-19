#include "mysh.h"

// Main
int main(int argc, char **argv) {
  // Flags for our parsing loop
  int inputFD = STDIN_FILENO; // The File Descriptor we're using, either 0 for
                              // terminal or anything else
  int isInteractive = 0;      // If we are in Interactive (1) or Batch (0)

  // Checking flags
  /*
      If argc == 2, always run batch mode
          If open() returns -1, the file failed to open
      If there is no arg, check stdin with isatty()
      If too many args, return fail
  */
  if (argc == 2) {
    inputFD = open(argv[1], O_RDONLY);
    if (inputFD < 0) {
      perror("Error opening batch file");
      return EXIT_FAILURE;
    }
    isInteractive = 0;
  } else if (argc == 1) {
    if (isatty(STDIN_FILENO))
      isInteractive = 1;
    else
      isInteractive = 0;
  } else
    return EXIT_FAILURE;

  // Entry message, as per p3.pdf
  if (isInteractive)
    printf("Welcome to my shell!\n");

  // Setup
  char line[MAX_LINE];
  Token tokens[MAX_TOKENS];

  // Parsing loop
  while (1) {
    // Interactive mode: print prompt before each command
    print_prompt(isInteractive);

    // Reading
    /*
     * read_line() uses an internal buffer and read() underneath.
     * It stops reading as soon as it sees '\n' — it will NOT call
     * read() again after receiving a complete command, which keeps
     * interactive mode responsive.
     * Returns 1 = got a line, 0 = EOF, -1 = error.
     */
    int rc = read_line(inputFD, line, sizeof(line));
    if (rc == 0)
      break; // EOF — stop the loop
    if (rc < 0) {
      perror("mysh: read error");
      break;
    }

    // Parsing
    int ntok = tokenize(line, tokens, MAX_TOKENS);
    if (ntok < 0) {
      fprintf(stderr, "mysh: too many tokens\n");
      continue;
    }
    if (ntok == 0)
      continue; // blank line or comment

    Job *job = parse_job(tokens, ntok);
    if (job == NULL)
      continue; // syntax error, already printed
    if (job->num_cmds == 0) {
      free_job(job);
      continue;
    }

    // Resolve program paths and check for exit
    int should_exit = 0;
    for (int i = 0; i < job->num_cmds; i++) {
      Command *cmd = &job->cmds[i];
      if (cmd->argc == 0)
        continue;

      cmd->builtin = is_builtin(cmd->argv[0]);
      if (cmd->builtin == BUILTIN_EXIT) {
        should_exit = 1;
        break;
      }
      if (cmd->builtin == BUILTIN_NONE) {
        int r = resolve_path(cmd->argv[0], cmd->resolved_path, MAX_LINE);
        if (r < 0)
          fprintf(stderr, "mysh: %s: command not found\n", cmd->argv[0]);
      }
    }

    // Execution
    execute_job(job, isInteractive);

    free_job(job);
    if (should_exit)
      break;
  }

  // Exit message, as per p3.pdf
  if (isInteractive)
    printf("Exiting my shell.\n");

  if (inputFD != STDIN_FILENO)
    close(inputFD);

  return EXIT_SUCCESS;
}
