#ifndef DICTIONARY_H
#define DICTIONARY_H

#define MAX_WORD_LEN 100
#define DICT_SIZE 100000 // Example size, adjust based on actual dictionary
#define BUFFER_SIZE 4096 // Buffer for reading from the file

extern char dictionary[DICT_SIZE][MAX_WORD_LEN]; // If you're using a global dictionary

// Function prototypes
int parseWordsAndAddToDictionary(const char *buffer, ssize_t bufLen, int *dictIndex);
int loadDictionary(const char *path);
int customCompare(const char* dictWord, const char* textWord);
int binarySearchForWord(const char* dictionary[], int size, const char* word);
int isAllUpperCase(const char* word);

#endif // DICTIONARY_H
