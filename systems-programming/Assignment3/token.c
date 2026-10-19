#include "mysh.h"

/*
 * Splits a null-terminated line into tokens and fills the tokens[]
 * array.  Returns the token count, or -1 on overflow.
 *
 * Rules (from p3):
 *   - A token is a run of non-whitespace characters.
 *   - '<', '>', '|' are always single-character tokens on their own.
 *     ( p3 guarantees they are whitespace-separated, but we handle
 *      them correctly even if they are not.)
 *   - '#' starts a comment; everything from '#' to EOL is discarded.
 */

int tokenize(const char *line, Token *tokens, int max_tokens) {
  int ntok = 0;
  int i = 0;
  int len = (int)strlen(line);

  while (i < len) {
    char c = line[i];

    /* skip whitespace */
    if (c == ' ' || c == '\t' || c == '\r') {
      i++;
      continue;
    }

    /* comment, stop scanning */
    if (c == '#')
      break;

    if (ntok >= max_tokens)
      return -1;

    /* single-character special tokens */
    if (c == '<') {
      tokens[ntok].type = TOK_REDIR_IN;
      tokens[ntok].value[0] = '<';
      tokens[ntok].value[1] = '\0';
      ntok++;
      i++;
      continue;
    }
    if (c == '>') {
      tokens[ntok].type = TOK_REDIR_OUT;
      tokens[ntok].value[0] = '>';
      tokens[ntok].value[1] = '\0';
      ntok++;
      i++;
      continue;
    }
    if (c == '|') {
      tokens[ntok].type = TOK_PIPE;
      tokens[ntok].value[0] = '|';
      tokens[ntok].value[1] = '\0';
      ntok++;
      i++;
      continue;
    }

    /* word token, consume until whitespace, special char, or comment */
    int j = 0;
    while (i < len) {
      char ch = line[i];
      if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '<' || ch == '>' ||
          ch == '|' || ch == '#')
        break;
      if (j < (int)sizeof(tokens[ntok].value) - 1)
        tokens[ntok].value[j++] = ch;
      i++;
    }
    tokens[ntok].value[j] = '\0';
    tokens[ntok].type = TOK_WORD;
    ntok++;
  }

  return ntok;
}

/* Allocate a fresh Command with an empty argv array. */
static void init_command(Command *cmd) {
  cmd->argv = calloc(MAX_TOKENS + 1, sizeof(char *));
  cmd->argc = 0;
  cmd->input_file = NULL;
  cmd->output_file = NULL;
  cmd->builtin = BUILTIN_NONE;
  cmd->resolved_path[0] = '\0';
}

/* Append a word to a command's argv (null-terminator maintained). */
static void append_arg(Command *cmd, const char *word) {
  cmd->argv[cmd->argc++] = strdup(word);
  cmd->argv[cmd->argc] = NULL;
}

/* Free the internals of a single Command (not the Command itself). */
static void free_command_internals(Command *cmd) {
  for (int i = 0; i < cmd->argc; i++) {
    free(cmd->argv[i]);
  }
  free(cmd->argv);
  free(cmd->input_file);
  free(cmd->output_file);
}

/*
 *  parse_job
 *
 * Converts a flat token array (from tokenize()) into a Job.
 *
 * Builds one Command per pipeline stage; "|" tokens advance to the
 * next stage.  "<" and ">" consume the following word token as the
 * redirect filename (not added to argv).
 *
 * Returns:
 *   - An allocated Job on success.  num_cmds == 0 means an empty/
 *     comment line (do nothing).
 *   - NULL on a syntax error (message already printed to stderr).
 *
 * Caller must free with free_job().
 */
Job *parse_job(Token *tokens, int num_tokens) {
  /* Empty / comment-only line */
  if (num_tokens == 0) {
    Job *j = calloc(1, sizeof(Job));
    j->num_cmds = 0;
    j->cmds = NULL;
    return j;
  }

  Job *job = calloc(1, sizeof(Job));
  job->cmds = calloc(MAX_CMDS, sizeof(Command));
  int cmd_idx = 0;

  init_command(&job->cmds[0]);

  for (int i = 0; i < num_tokens; i++) {
    TokenType t = tokens[i].type;
    Command *cur = &job->cmds[cmd_idx];

    /* ---- Pipeline separator ---- */
    if (t == TOK_PIPE) {
      if (cur->argc == 0) {
        fprintf(stderr, "mysh: syntax error near '|'\n");
        goto syntax_err;
      }
      cmd_idx++;
      if (cmd_idx >= MAX_CMDS) {
        fprintf(stderr, "mysh: too many pipeline stages\n");
        goto syntax_err;
      }
      init_command(&job->cmds[cmd_idx]);
      continue;
    }

    // Input redirection
    if (t == TOK_REDIR_IN) {
      if (i + 1 >= num_tokens || tokens[i + 1].type != TOK_WORD) {
        fprintf(stderr, "mysh: syntax error: expected filename after '<'\n");
        goto syntax_err;
      }
      if (cur->input_file != NULL) {
        fprintf(stderr, "mysh: syntax error: duplicate '<'\n");
        goto syntax_err;
      }
      cur->input_file = strdup(tokens[i + 1].value);
      i++; /* consume the filename token */
      continue;
    }

    // output direction
    if (t == TOK_REDIR_OUT) {
      if (i + 1 >= num_tokens || tokens[i + 1].type != TOK_WORD) {
        fprintf(stderr, "mysh: syntax error: expected filename after '>'\n");
        goto syntax_err;
      }
      if (cur->output_file != NULL) {
        fprintf(stderr, "mysh: syntax error: duplicate '>'\n");
        goto syntax_err;
      }
      cur->output_file = strdup(tokens[i + 1].value);
      i++;
      continue;
    }

    // wildcard
    if (strchr(tokens[i].value, '*') != NULL) {
      glob_t g;
      if (glob(tokens[i].value, GLOB_NOCHECK | GLOB_TILDE, NULL, &g) == 0) {
        for (size_t j = 0; j < g.gl_pathc; j++) 
          append_arg(cur, g.gl_pathv[j]);
        globfree(&g);
      }
      continue;
    }

    /* ---- Normal argument word ---- */
    append_arg(cur, tokens[i].value);
  }

  /* Trailing pipe with nothing after it */
  {
    Command *last = &job->cmds[cmd_idx];
    if (last->argc == 0 && cmd_idx > 0) {
      fprintf(stderr, "mysh: syntax error: trailing '|'\n");
      goto syntax_err;
    }
  }

  job->num_cmds = cmd_idx + 1;

  /* Blank line (no words at all, no pipes) */
  if (job->num_cmds == 1 && job->cmds[0].argc == 0) {
    free_job(job);
    Job *empty = calloc(1, sizeof(Job));
    empty->num_cmds = 0;
    empty->cmds = NULL;
    return empty;
  }

  return job;

syntax_err:
  /* Free however many commands we allocated so far */
  for (int k = 0; k <= cmd_idx; k++)
    free_command_internals(&job->cmds[k]);
  free(job->cmds);
  free(job);
  return NULL;
}

/*
 * free_job: releases all memory owned by a Job.
 */
void free_job(Job *job) {
  if (!job)
    return;
  for (int i = 0; i < job->num_cmds; i++)
    free_command_internals(&job->cmds[i]);
  free(job->cmds);
  free(job);
}
