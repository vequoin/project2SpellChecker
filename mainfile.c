#include "dictionary.h"
#include "file-processing.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // Check for the correct number of arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <dictionary_path> <file_or_directory_path>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load the dictionary from the specified path
    if (!loadDictionary(argv[1])) {
        fprintf(stderr, "Failed to load dictionary from %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    // Process the specified file or directory for spell checking
    searchDirAndCheckSpelling(argv[2]);

    // Free the allocated dictionary resources
    freeDictionary();

    return EXIT_SUCCESS;
}
