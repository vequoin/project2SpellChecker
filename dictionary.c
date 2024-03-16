#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For read, close
#include <fcntl.h> // For open
#include <ctype.h>

#define MAX_WORD_LEN 100
#define DICT_SIZE 100000 // Example size, adjust based on actual dictionary
#define BUFFER_SIZE 4096 // Buffer for reading from the file

char dictionary[DICT_SIZE][MAX_WORD_LEN];

// Helper function to parse words from buffer and add them to the dictionary
int parseWordsAndAddToDictionary(const char *buffer, ssize_t bufLen, int *dictIndex) {
    int wordLen = 0;
    char word[MAX_WORD_LEN] = {0};

    for (ssize_t i = 0; i < bufLen; i++) {
        char c = buffer[i];
        
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '-')) {
            if (wordLen < MAX_WORD_LEN - 1) {
                word[wordLen++] = c;
            }
        } else if (wordLen > 0) { // End of word detected
            word[wordLen] = '\0'; // Null-terminate the current word
            if (*dictIndex < DICT_SIZE) {
                strcpy(dictionary[*dictIndex], word); 
                (*dictIndex)++;
            }
            wordLen = 0; // Reset word length for the next word
        }
    }

    return wordLen; // Return length of the last word processed
}

// Function to load dictionary using POSIX functions
int loadDictionary(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening dictionary file");
        return 0;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int dictIndex = 0;
    int partialWordLen = 0;
    char partialWord[MAX_WORD_LEN] = {0}; // For words split across buffer reads

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        // If we have a partial word from the previous buffer, handle it
        if (partialWordLen > 0) {
            int i = 0;
            // Continue the partial word until space or newline
            while (i < bytesRead && buffer[i] != '\n' && buffer[i] != ' ' && partialWordLen < MAX_WORD_LEN - 1) {
                partialWord[partialWordLen++] = buffer[i++];
            }
            partialWord[partialWordLen] = '\0';
            if (dictIndex < DICT_SIZE) {
                strcpy(dictionary[dictIndex++], partialWord);
            }
            // Reset partial word
            partialWordLen = 0;
            partialWord[0] = '\0';
            // Adjust buffer and length to exclude the processed part
            buffer += i;
            bytesRead -= i;
        }
        // Process current buffer
        partialWordLen = parseWordsAndAddToDictionary(buffer, bytesRead, &dictIndex);
        // If the last word was not fully read, save it as partialWord
        if (partialWordLen > 0) {
            strncpy(partialWord, &buffer[bytesRead - partialWordLen], partialWordLen);
            partialWord[partialWordLen] = '\0';
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

int isWordInDictionary(const char *word){

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

