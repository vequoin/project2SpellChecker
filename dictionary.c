#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For read, close
#include <fcntl.h> // For open
#include <ctype.h>

#define MAX_WORD_LEN 100
#define INITIAL_DICT_SIZE 1000 // Initial size, will grow as needed
#define BUFFER_SIZE 4096 // Buffer for reading from the file

char **dictionary = NULL;
size_t dictSize = INITIAL_DICT_SIZE;
size_t dictIndex = 0; // Tracks the next free slot in the dictionary

// Helper function to parse words from buffer and add them to the dictionary
int parseWordsAndAddToDictionary(const char *buffer, ssize_t bufLen) {
    int wordLen = 0;
    char word[MAX_WORD_LEN] = {0};

    for (ssize_t i = 0; i < bufLen; i++) {
        char c = buffer[i];
        
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '-') || (c == '\'')) {
            if (wordLen < MAX_WORD_LEN - 1) {
                word[wordLen++] = c;
            }
        } else if (wordLen > 0) { // End of word detected
            word[wordLen] = '\0'; // Null-terminate the current word
            
            resizeDictionaryIfNeeded(); // Ensure there's room for the new word
            dictionary[dictIndex] = strdup(word); // Allocate memory and copy the word
            dictIndex++;
            
            wordLen = 0; // Reset word length for the next word
        }
    }

    // Handle any partial word left at the end of the buffer
    if (wordLen > 0) {
        word[wordLen] = '\0';
        resizeDictionaryIfNeeded();
        dictionary[dictIndex] = strdup(word);
        dictIndex++;
    }

    return wordLen; // Note: This return value may not be used; consider void return type
}

int loadDictionary(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening dictionary file");
        return 0;
    }
    dictionary = malloc(dictSize * sizeof(char*));
    if (!dictionary) {
        perror("Memory allocation failed");
        close(fd);
        exit(EXIT_FAILURE); // Or handle more gracefully
    }

    size_t dictSize = INITIAL_DICT_SIZE; // Current size of the dictionary
    size_t dictIndex = 0; // Index for the next word to add

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    char partialWord[MAX_WORD_LEN] = {0}; // For words split across buffer reads
    int partialWordLen = 0;
    char word[MAX_WORD_LEN] = {0};

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        ssize_t currentPos = 0; // Track current position within the buffer

        // If there's a partial word from the previous read, handle it first
        if (partialWordLen > 0) {
            while (currentPos < bytesRead && buffer[currentPos] != '\n' && buffer[currentPos] != ' ' && partialWordLen < MAX_WORD_LEN - 1) {
                partialWord[partialWordLen++] = buffer[currentPos++];
            }
            partialWord[partialWordLen] = '\0'; // Null-terminate the partial word
            
            // If we completed the partial word (by reaching a space, newline, or end of buffer)
            if (currentPos < bytesRead && (buffer[currentPos] == ' ' || buffer[currentPos] == '\n')) {
                if (dictIndex < INITIAL_DICT_SIZE) {
                    strcpy(dictionary[dictIndex++], partialWord); // Add the completed partial word to the dictionary
                }
                partialWordLen = 0; // Reset for the next word
                currentPos++; // Move past the space/newline that ended the partial word
            }
        }

        // Now process the rest of the buffer starting from currentPos
        ssize_t start = currentPos; // Remember the start of the next word to process
        for (; currentPos < bytesRead; currentPos++) {
            if (buffer[currentPos] == ' ' || buffer[currentPos] == '\n' || currentPos == bytesRead - 1) {
                // Extract the word from start to currentPos
                int len = currentPos - start + (currentPos == bytesRead - 1 && buffer[currentPos] != ' ' && buffer[currentPos] != '\n'); // Include the last character if it's not whitespace
                if (len > 0 && len < MAX_WORD_LEN) {
                    strncpy(word, buffer + start, len);
                    word[len] = '\0'; // Null-terminate the word
                    if (dictIndex < INITIAL_DICT_SIZE) {
                        strcpy(dictionary[dictIndex++], word); // Add the word to the dictionary
                    }
                }
                start = currentPos + 1; // Set start to the beginning of the next word
            }
        }

        // Handle case where the word at the end of the buffer is incomplete (a new partial word)
        if (start < bytesRead) {
            partialWordLen = bytesRead - start;
            strncpy(partialWord, buffer + start, partialWordLen);
            partialWord[partialWordLen] = '\0'; // Prepare the partial word for the next read
        }
    }

    if (bytesRead == -1) {
        perror("Error reading dictionary file");
        close(fd);
        return 0;
    }

    close(fd);
    return 1;
}

void resizeDictionaryIfNeeded() {
    if (dictIndex >= dictSize) {
        size_t newSize = dictSize * 2;
        char **newDict = realloc(dictionary, newSize * sizeof(char*));
        if (!newDict) {
            perror("Failed to resize dictionary");
            exit(EXIT_FAILURE);
        }
        dictionary = newDict;
        dictSize = newSize;
    }
}


int isWordInDictionary(const char *word) {
    return binarySearchForWord(dictionary, INITIAL_DICT_SIZE, word) != -1;
}

int customCompare(const char* dictWord, const char* textWord) {
    while (*dictWord && *textWord) {
        // Convert both characters to lowercase for comparison.
        char lowerDictChar = tolower((unsigned char)*dictWord);
        char lowerTextChar = tolower((unsigned char)*textWord);

        if (lowerDictChar != lowerTextChar) {
            return 0; // Characters don't match, not the same word.
        }

        // Additional rule: if dictWord character is uppercase and textWord character doesn't match exactly.
        if (isupper((unsigned char)*dictWord) && *dictWord != *textWord) {
            return 0; // Uppercase character in dictWord doesn't match textWord character exactly.
        }

        dictWord++;
        textWord++;
    }

    // Both strings should end at the same time for a complete match.
    return *dictWord == '\0' && *textWord == '\0';
}

int binarySearchForWord(const char* dictionary[], int size, const char* word) {
    int low = 0, high = size - 1;
    
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int res = customCompare(word, dictionary[mid]);

        // If words match case-insensitively, check for case-sensitive match if needed.
        if (res == 0) {
            // For words that must retain exact capitalization (e.g., "McDonalds"),
            // perform an additional exact match check here if the dictionary word
            // has capital letters beyond the initial character.
            if (strcmp(word, dictionary[mid]) == 0 || isAllUpperCase(word)) {
                return mid; // Exact match found
            }
            // If not an exact match and not all uppercase, adjust search range.
            // This part may need refinement based on how you define "exact capitalization rules".
        }

        if (res < 0) high = mid - 1; // Search left half
        else low = mid + 1; // Search right half
    }

    return -1; // Word not found
}

int isAllUpperCase(const char* word) {
    // Returns 1 (true) if all characters in 'word' are uppercase, 0 (false) otherwise.
    while (*word) {
        if (islower(*word)) return 0;
        word++;
    }
    return 1;
}

void freeDictionary(){
    for (size_t i = 0; i < dictIndex; i++) {
    free(dictionary[i]); // Free each word
}
free(dictionary); // Free the array of pointers
}
