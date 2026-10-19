#include "mysh.h"

/*  Internal read buffer
 *
 *  We never call read() again once we have seen a newline.
 *  Any leftover bytes after the newline stay in the buffer and are
 *  consumed on the next call to read_line().
 */

#define IBUF_CAP 4096

static char ibuf[IBUF_CAP];
static int ibuf_start = 0; /* index of first unconsumed byte */
static int ibuf_end = 0;   /* one past the last valid byte   */

/*
 * read_line
 *
 * Fills line_buf with the next complete line from fd, stripping the
 * trailing newline.  line_buf is always null-terminated on success.
 *
 * Returns:
 *   1  — got a line (may be empty if the line was just "\n")
 *   0  — EOF, no more data
 *  -1  — read() error
 *
 * Algorithm:
 *   1. Scan the already-buffered bytes for '\n'.
 *   2. If found, copy up to that point into line_buf, advance ibuf_start
 *      past the '\n', and return — never calling read() again.
 *   3. If not found, compact the buffer, call read() once to refill,
 *      and repeat.
 */
int read_line(int fd, char *line_buf, int max_len) {
  while (1) {
    // scan for a new line in buffered data
    for (int i = ibuf_start; i < ibuf_end; i++) {
      if (ibuf[i] == '\n') {
        int copy = i - ibuf_start;
        if (copy >= max_len)
          copy = max_len - 1;
        memcpy(line_buf, ibuf + ibuf_start, copy);
        line_buf[copy] = '\0';
        ibuf_start = i + 1; /* skip past the '\n' */
        return 1;
      }
    }

    int avail = ibuf_end - ibuf_start;

    // Guard against a line longer than the caller's buffer. Truncate and drain
    // the rest up to the next newline.
    if (avail >= max_len - 1) {
      memcpy(line_buf, ibuf + ibuf_start, max_len - 1);
      line_buf[max_len - 1] = '\0';
      ibuf_start = ibuf_end; /* discard buffered portion */

      // drain fd until newline or EOF
      while (1) {
        int n = read(fd, ibuf, IBUF_CAP);
        if (n <= 0)
          return (n == 0) ? 1 : -1;
        ibuf_start = 0;
        ibuf_end = n;
        for (int i = 0; i < n; i++) {
          if (ibuf[i] == '\n') {
            ibuf_start = i + 1;
            return 1;
          }
        }
        ibuf_start = ibuf_end; /* still no newline, keep draining */
      }
    }

    if (avail > 0 && ibuf_start > 0)
      memmove(ibuf, ibuf + ibuf_start, avail);
    ibuf_start = 0;
    ibuf_end = avail;

    /* Fill from fd */
    int n = read(fd, ibuf + ibuf_end, IBUF_CAP - ibuf_end);
    if (n < 0)
      return -1;
    if (n == 0) {
      /* EOF — flush any remaining partial line */
      if (ibuf_end > 0) {
        int copy = ibuf_end;
        if (copy >= max_len)
          copy = max_len - 1;
        memcpy(line_buf, ibuf, copy);
        line_buf[copy] = '\0';
        ibuf_end = 0;
        return 1;
      }
      return 0; /* truly nothing left */
    }
    ibuf_end += n;
  }
}

/*
 * print_prompt
 *
 * Prints the interactive prompt: "<cwd>$ "
 * If cwd starts with $HOME, that prefix is replaced with ~.
 * Does nothing in batch mode.
 *
 * Uses write() directly so we don't buffer past a newline.
 */
void print_prompt(int interactive) {
  if (!interactive)
    return;

  char cwd[MAX_LINE];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    write(STDOUT_FILENO, "$ ", 2);
    return;
  }

  const char *home = getenv("HOME");
  char prompt[MAX_LINE + 4];
  int plen = 0;

  if (home != NULL) {
    size_t hlen = strlen(home);
    if (strncmp(cwd, home, hlen) == 0 &&
        (cwd[hlen] == '/' || cwd[hlen] == '\0')) {
      /* replace home prefix with ~ */
      prompt[plen++] = '~';
      const char *rest = cwd + hlen;
      while (*rest && plen < (int)sizeof(prompt) - 4)
        prompt[plen++] = *rest++;
    } else {
      goto full_path;
    }
  } else {
  full_path:;
    const char *p = cwd;
    while (*p && plen < (int)sizeof(prompt) - 4)
      prompt[plen++] = *p++;
  }

  prompt[plen++] = '$';
  prompt[plen++] = ' ';
  write(STDOUT_FILENO, prompt, plen);
}

//Mostly for testing purposes, to reset the static variables
/*
 * Just in case
 */
void reset_read_line() {
  ibuf_start = 0;
  ibuf_end = 0;
}
