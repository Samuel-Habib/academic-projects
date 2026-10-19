#include <stdio.h>

//main()
/*
 * When run in mysh, this crashes.
 * We want to see if it handles the termination signal 
 */
int main() {
    int *p = NULL;
    *p = 10; 
    return 0;
}