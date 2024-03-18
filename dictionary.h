#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stddef.h> // For size_t

// Constants
#define MAX_WORD_LEN 100
#define INITIAL_DICT_SIZE 1000
#define BUFFER_SIZE 4096

// Global variables (consider moving to dictionary.c and providing getter functions instead)
extern char **dictionary;
extern size_t dictSize;
extern size_t dictIndex;

// Function declarations
void resizeDictionaryIfNeeded(void);
int parseWordsAndAddToDictionary(const char *buffer, ssize_t bufLen);
int loadDictionary(const char *path);
int isWordInDictionary(const char *word);
int customCompare(const char* dictWord, const char* textWord);
int binarySearchForWord(const char* dictionary[], int size, const char* word);
int isAllUpperCase(const char* word);
void freeDictionary(void);

#endif // DICTIONARY_H
