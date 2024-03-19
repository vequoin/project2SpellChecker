#include "file-processing.h" // Ensure this header exists and is correctly named
#include "dictionary.h" // Assumes it declares isWordInDictionary
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h> 
#include <sys/types.h>

#define BUFFER_SIZE 4096
#define MAX_WORD_LEN 100


void checkSpelling(const char *path) {
    //printf("Starting to check spelling in file: %s\n", path);
    int fo = open(path, O_RDONLY);
    if (fo == -1) {
        perror("File cannot be opened");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int lineNum = 1, colNum = 1; // Initialize line and column numbers
    char prevChar = '\0'; // Correct variable naming to match convention

    while ((bytesRead = read(fo, buffer, BUFFER_SIZE - 1)) > 0) {
        //printf("Processing %zd bytes from file.\n", bytesRead);
        buffer[bytesRead] = '\0'; // Ensuring the buffer is null-terminated
        ssize_t i = 0;

        while (i < bytesRead) {
            if (isPartOfWord(buffer[i], prevChar)) {
                char word[MAX_WORD_LEN] = {0};
                int wordLen = 0, wordStartCol = colNum;

                while (i < bytesRead && isPartOfWord(buffer[i], prevChar)) {
                    if (wordLen < MAX_WORD_LEN - 1) {
                        word[wordLen++] = buffer[i];
                    }
                    prevChar = buffer[i];
                    colNum++;
                    i++;
                }

                word[wordLen] = '\0'; // Null-terminate the word
                //printf("Extracted word: '%s' at line %d, column %d\n", word, lineNum, wordStartCol);

                //trimTrailingPunctuation(word);
                //printf("After trimming, word: '%s'\n", word);

                if(isWordInDictionary(word)) {
                    //printf("'%s' is in the dictionary.\n", word);
                } else {
                    printf("Misspelled word '%s' at line %d, column %d\n", word, lineNum, wordStartCol);
                }

            } else {
                if (buffer[i] == '\n') {
                    lineNum++;
                    colNum = 1; // Reset column number for new line
                    //printf("Newline encountered. Moving to line %d.\n", lineNum);
                } else {
                    colNum++;
                }
                prevChar = buffer[i];
                i++; // Increment `i` here for non-word characters
            }
        }
    }

    if (bytesRead == -1) {
        perror("Error reading file");
    }

    printf("Finished checking file: %s\n", path);
    close(fo);
}




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

int isPartOfWord(char c, char prevChar) {
    if (isalpha(c)) {
        return 1; // Alphabetic characters are always part of a word
    } else if (c == '-') {
        // Hyphens are part of a word if the previous character is alphabetic.
        // You may need to adjust this if you also want to check the following character,
        // which would require changing the function signature to pass the next character or buffer.
        return isalpha(prevChar);
    } else if (c == '\'') {
        // Apostrophes are part of a word if they are not the first character in the word,
        // indicated by the previous character being alphabetic.
        return isalpha(prevChar);
    }
    return 0;
}

void trimTrailingPunctuation(char* word) {
    int length = strlen(word);

    while (length > 0) {
        char lastChar = word[length - 1];
        if (lastChar == '.' || lastChar == ',' || lastChar == '?' || lastChar == '!' ||
            lastChar == ':' || lastChar == ';' || lastChar == '"' || lastChar == '\'' ||
            lastChar == ')' || lastChar == ']' || lastChar == '}') {
            word[length - 1] = '\0'; 
            length--; 
        } else {
           
            break;
        }
    }
}

