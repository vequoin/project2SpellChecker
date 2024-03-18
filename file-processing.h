#ifndef FILE_PROCESSING_H
#define FILE_PROCESSING_H

// Function declarations for file processing and spell checking

/**
 * Recursively searches through directories and checks the spelling of words
 * in .txt files.
 * 
 * @param dirPath The path to the directory to be searched.
 */
void searchDirAndCheckSpelling(const char* dirPath);

/**
 * Checks the spelling of words in a given file.
 * 
 * @param path The path to the file whose spelling is to be checked.
 */
void checkSpelling(const char *path);

/**
 * Determines if a character should be considered part of a word based
 * on its value and the value of the previous character in the text.
 * 
 * @param c The current character being considered.
 * @param prevChar The previous character in the text.
 * @return 1 if the character is part of a word, 0 otherwise.
 */
int isPartOfWord(char c, char prevChar);

#endif // FILE_PROCESSING_H
