#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    if (argc < 3) {
        printf("not enough arguments\n");
        return 1;
    }

    int useSystemCalls = 0;
    int searchStart = 2;

    if (strcmp(argv[2], "--systemcalls") == 0) {
        useSystemCalls = 1;
        searchStart = 3;
        if (argc < 4) {
            printf("not enough arguments\n");
            return 1;
        }
    }

    char *filename = argv[1];
    char *buffer = NULL;
    size_t fileSize = 0;

    if (useSystemCalls) {
        int fd = open(filename, O_RDONLY);
        if (fd < 0) {
            perror("Error opening file");
            return 1;
        }
        size_t capacity = 1024;
        buffer = malloc(capacity);
        if (!buffer) {
            perror("malloc");
            close(fd);
            return 1;
        }
        char c;
        ssize_t n;
        while ((n = read(fd, &c, 1)) == 1) {
            if (fileSize == capacity) {
                capacity *= 2;
                char *temp = realloc(buffer, capacity);
                if (!temp) {
                    perror("realloc");
                    free(buffer);
                    close(fd);
                    return 1;
                }
                buffer = temp;
            }
            buffer[fileSize++] = c;
        }
        close(fd);
    } else {
        FILE *fp = fopen(filename, "r");
        
        if(fp == NULL){
            fprintf(stderr, "%s: No such file or directory", argv[0]);
            return 1;
        }

        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(fileSize + 1);
        if (!buffer) {
            perror("malloc");
            fclose(fp);
            return 1;
        }
        fread(buffer, 1, fileSize, fp);
        fclose(fp);
    }

    buffer[fileSize] = '\0';

    int countSize = argc - searchStart;
    int wordCount[countSize];
    for(int i = 0; i < countSize; i++){
        wordCount[i] = 0;
    }

    int found = 0;
    for(int i = searchStart; i < argc; i++){
        int len = strlen(argv[i]);
        for(size_t k = 0; k < fileSize; k++){
            int matchCount = 0;
            while( matchCount < len &&
                   (k + matchCount) < fileSize &&
                   argv[i][matchCount] == buffer[k + matchCount] )
            {
                matchCount++;
            }
            if(matchCount == len){
                wordCount[i - searchStart]++;
                found = 1;
            }
        }
    }

    for(int i = 0; i < countSize; i++){


        printf("%d\n", wordCount[i]);
    }
 

    free(buffer);
    return 0;
}

