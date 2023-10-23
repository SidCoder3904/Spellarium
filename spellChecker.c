# include <stdio.h>     // standard C library
# include <stdlib.h>    // for dynamic memory allocation
# include <stdbool.h>   // for bool datatype
# include <stdint.h>    // for uint32 types etc
# include <string.h>    // for string manipulation
# include <ctype.h>     // for formatting
# include "spell.h"     // our header file with bloom filter, trie and lru cache functionality

// testing the functions for correctness
int main() {
    // initializing bloom filter
    bool* filter = calloc(FILTER_SIZE, sizeof(bool));
    TRIE_NODE* root = createNode();
    // populating bloom filter and trie with dictionary
    loadDictionary(filter, root);
    printf("Loaded Dictionary on Bloom Filter and Trie.\n");
    // testing bloom filter
    while(1) {
        printf("\nEnter word to search: ");
        char input[50];
        fscanf(stdin, "%s", input);
        for(int i = 0; input[i]; i++) input[i] = tolower(input[i]);
        printf("\nUsing Bloom Filter:");
        if(searchFilter(filter, input)) printf("\nword present"); // search
        else printf("\nword not present");
        printf("\nUsing Trie:");
        if(searchTrie(root, input)) printf("\nword present"); // search
        else printf("\nword not present");
        printf("\nEnter word to compare: ");
        char input2[50];
        fscanf(stdin, "%s", input2);
        for(int i = 0; input2[i]; i++) input2[i] = tolower(input2[i]);
        printf("\nLevenshyein Distance: %d\nJaro-Winkler Similarity: %lf", levenshteinDistance(input, input2), jaroWinklerDistance(input, input2));
    }
    return 0;
}