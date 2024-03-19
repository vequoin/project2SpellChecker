#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For read, close
#include <fcntl.h> // For open
#include <ctype.h>
#include "dictionary.h"
#include <sys/types.h>
#include <strings.h>

#define MAX_WORD_LEN 100
//#define INITIAL_DICT_SIZE 10000 // Initial size, will grow as needed
#define BUFFER_SIZE 4096 // Buffer for reading from the file

char **dictionary = NULL;
size_t dictSize = 0;
size_t dictIndex = 0; 

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
            
            //resizeDictionaryIfNeeded(); // Ensure there's room for the new word
            dictionary[dictIndex] = strdup(word); // Allocate memory and copy the word
            dictIndex++;
            
            wordLen = 0; // Reset word length for the next word
        }
    }

    // Handle any partial word left at the end of the buffer
    if (wordLen > 0) {
        word[wordLen] = '\0';
        //resizeDictionaryIfNeeded();
        dictionary[dictIndex] = strdup(word);
        dictIndex++;
    }

    return wordLen; 
}

size_t countWordsInDictionary(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening dictionary file for counting");
        return 0;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    size_t wordCount = 0;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) {
            // Assuming words are separated by newlines or spaces
            if (buffer[i] == '\n' || buffer[i] == ' ') {
                wordCount++;
            }
        }
    }

    close(fd);

    // Account for the last word if file doesn't end with a newline or space
    wordCount++;

    return wordCount;
}
int loadDictionary(const char *path) {
    // First, count the number of words to allocate memory accordingly
    dictSize = countWordsInDictionary(path);
    printf("Number of words in dictionary: %zu\n", dictSize);

    dictionary = malloc(dictSize * sizeof(char*)); // Allocation based on word count
    if (!dictionary) {
        perror("Failed to allocate memory for dictionary");
        return 0;
    }

    // Open the file again to actually load the words
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening dictionary file for loading");
        free(dictionary); // Clean up allocated memory
        return 0;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    size_t currentWordIndex = 0;
    char word[MAX_WORD_LEN] = {0};
    int wordLen = 0;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytesRead; i++) {
            if (buffer[i] == '\n' || buffer[i] == ' ') {
                if (wordLen > 0) {
                    word[wordLen] = '\0'; // Null-terminate the current word
                    dictionary[currentWordIndex++] = strdup(word);
                    wordLen = 0; // Reset for the next word
                }
            } else {
                if (wordLen < MAX_WORD_LEN - 1) {
                    word[wordLen++] = buffer[i];
                }
            }
        }
    }

    // Handle the last word if file doesn't end with a newline or space
    if (wordLen > 0) {
        word[wordLen] = '\0';
        dictionary[currentWordIndex++] = strdup(word);
    }

    close(fd);
    printf("Loaded %zu words into the dictionary.\n", currentWordIndex);
    return 1;
}



/*void resizeDictionaryIfNeeded() {
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
}*/


/*int isWordInDictionary(const char *word) {
    return binarySearchForWord((const char* const*)dictionary, INITIAL_DICT_SIZE, word) != -1;
}*/

// Helper function to check if a word is all uppercase
int isAllUpperCase(const char* word) {
    for (; *word; ++word) {
        if (islower((unsigned char)*word)) return 0;
    }
    return 1;
}

// Custom comparison function
int customCompare(const char* dictWord, const char* textWord) {
    // Perform a case-insensitive comparison first.
    //printf("Comparing dictionary word '%s' with text word '%s'\n", dictWord, textWord);
    const char* pDict = dictWord;
    const char* pText = textWord;
    while (*pDict && *pText && tolower((unsigned char)*pDict) == tolower((unsigned char)*pText)) {
        ++pDict;
        ++pText;
    }
    // If not at the end of both strings, they're not a match.
    if (*pDict != '\0' || *pText != '\0') return 0;

    // If the text word is all uppercase, it's considered a correct match.
    if (isAllUpperCase(textWord)) return 1;

    // Re-check ensuring uppercase in dictWord matches exactly in textWord.
    while (*dictWord && *textWord) {
        if (isupper((unsigned char)*dictWord) && *dictWord != *textWord) {
            // Uppercase letter doesn't match exactly, not a match.
            return 0;
        }
        ++dictWord;
        ++textWord;
    }

    // Passed all checks, it's a match.
    return 1;
}

int isWordInDictionary(const char *word) {
    return sequentialSearchForWord((const char* const*)dictionary, dictSize, word) != -1;
}

int sequentialSearchForWord(const char* const dictionary[], int size, const char* word) {
    for (int i = 0; i < size; ++i) {
        //printf("Checking word: '%s'\n", word);
        if (customCompare(dictionary[i], word)) {
            return i; // Found a match
        }
    }
    return -1; // No match found
}


int binarySearchForWord(const char* const dictionary[], int size, const char* word) {
    int low = 0, high = size - 1;
    
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int compareResult = strcasecmp(word, dictionary[mid]);

        if (compareResult == 0) { // Case-insensitive match found
            if (customCompare(dictionary[mid], word)) {
                // Match confirmed with custom rules
                return mid;
            } else {
                // Continue searching even after a fundamental match because 
                // customCompare might fail due to specific rules.
                // This is where you might adjust logic based on the specifics of your dictionary and rules.
                int leftMatch = (mid > low) ? binarySearchForWord(dictionary, mid, word) : -1;
                if (leftMatch != -1) return leftMatch;

                int rightMatch = (mid < high) ? binarySearchForWord(dictionary + mid + 1, high - mid, word) : -1;
                if (rightMatch != -1) return mid + 1 + rightMatch;

                return -1; // No match found according to rules.
            }
        }

        if (compareResult < 0) high = mid - 1; // Search left half
        else low = mid + 1; // Search right half
    }

    return -1; // Word not found
}

void freeDictionary(){
    for (size_t i = 0; i < dictIndex; i++) {
    free(dictionary[i]); // Free each word
}
free(dictionary); // Free the array of pointers
}
