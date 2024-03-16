#include "file_processing.h"
#include "dictionary.h" // This inclusion depends on the actual content of dictionary.h
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

void checkSpelling(const char *path);

void searchDirAndCheckSpelling(const char* dirPath) {
    DIR* dir = opendir(dirPath);
    struct dirent* start;
    if (!dir) {
        fprintf(stderr, "Error opening directory: %s\n", dirPath);
        return;
    }

    char path[1024];

    while ((start = readdir(dir)) != NULL) {
        if (strcmp(start->d_name, ".") == 0 || strcmp(start->d_name, "..") == 0)
            continue;
        snprintf(path, sizeof(path), "%s/%s", dirPath, start->d_name);
        if (start->d_type == DT_DIR) {
            searchDirAndCheckSpelling(path);
        } else if (start->d_type == DT_REG) {
            if (strstr(start->d_name, ".txt")) {
                checkSpelling(path);
            }
        }
    }

    closedir(dir);
}

void checkSpelling(const char *path) {
    int fo = open(path, O_RDONLY);
    if (fo == -1) {
        perror("File cannot be opened");
        return;
    }
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    while ((bytesRead = read(fo, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytesRead] = '\0';
        // Add logic to parse the buffer and check spelling
    }
    if (bytesRead == -1) {
        perror("Error reading file");
    }

    close(fo);
}
