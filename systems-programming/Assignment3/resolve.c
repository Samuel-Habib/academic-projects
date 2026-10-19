#include "mysh.h"

/* Directories searched in order for bare program names. */
static const char *SEARCH_DIRS[] = {"/usr/local/bin", "/usr/bin", "/bin", NULL};

/*
 * is_builtin
 *
 * Returns the BUILTIN_* id if name matches a built-in command,
 * or BUILTIN_NONE if it does not.
 *
 * Must be checked BEFORE resolve_path() so that programs named "cd"
 * etc. in the search directories are correctly ignored.
 */
int is_builtin(const char *name) {
  if (strcmp(name, "cd") == 0)
    return BUILTIN_CD;
  if (strcmp(name, "pwd") == 0)
    return BUILTIN_PWD;
  if (strcmp(name, "which") == 0)
    return BUILTIN_WHICH;
  if (strcmp(name, "exit") == 0)
    return BUILTIN_EXIT;
  return BUILTIN_NONE;
}

/*
 * resolve_path
 *
 * Resolves a command name to a full executable path using the rules from p3.
 *
 * Logic :
 *   1. If name is a built-in, set out_path to "" and return 0.
 *      (Caller should call is_builtin() first and skip this for
 *       built-ins, but we defend against it here too.)
 *   2. If name contains '/', use it directly as a path.
 *      Test with access(X_OK); fail if not executable.
 *   3. Otherwise search /usr/local/bin, /usr/bin, /bin with access().
 *
 * Writes the resolved path into out_path , at most out_len-1 bytes + NUL.
 *
 * Returns:
 *   1  — path found and written into out_path
 *   0  — name is a built-in; out_path set to ""
 *  -1  — not found / not executable
 */
int resolve_path(const char *name, char *out_path, int out_len) {
  if (is_builtin(name) != BUILTIN_NONE) {
    out_path[0] = '\0';
    return 0;
  }

  if (strchr(name, '/') != NULL) {
    if (access(name, X_OK) == 0) {
      strncpy(out_path, name, out_len - 1);
      out_path[out_len - 1] = '\0';
      return 1;
    }
    return -1;
  }

  // search the three directories in order
  for (int i = 0; SEARCH_DIRS[i] != NULL; i++) {
    char candidate[MAX_LINE];
    int n =
        snprintf(candidate, sizeof(candidate), "%s/%s", SEARCH_DIRS[i], name);
    if (n < 0 || n >= (int)sizeof(candidate))
      continue; /* overflow */

    if (access(candidate, X_OK) == 0) {
      strncpy(out_path, candidate, out_len - 1);
      out_path[out_len - 1] = '\0';
      return 1;
    }
  }

  return -1; // not found anywhere
}
