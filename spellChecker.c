# include <stdio.h>     // standard C library
# include <stdlib.h>    // for dynamic memory allocation
# include <stdbool.h>   // for bool datatype
# include <stdint.h>    // for uint32 types etc
# include <string.h>    // for string manipulation
# include <ctype.h>     // for formatting
# include <math.h>
# include <ctype.h>
# include "spell.h"     // our header file with bloom filter, trie and lru cache functionality

// testing the functions for correctness
int main() {
    createQueue();
    struct LRUCache* cache=lRUCacheCreate(MAX_SUGGESTIONS);
    TRIE_NODE* root=createNode();
    bool* filter = calloc(FILTER_SIZE, sizeof(bool));
    loadDictionary(filter, root);
    printf(COLOR_YELLOW "Dictionary Loaded\n" COLOR_RESET);
    char str[100];
    while (1){
        printf(COLOR_MAGENTA "Enter a sentence:\n" COLOR_RESET);
        fgets(str, sizeof(str), stdin);
        str[strcspn(str, "\n")] = 0;
        printf(COLOR_MAGENTA "The sentence after checking is: \n" COLOR_RESET);
        int index = 0, incrt_words = 0;

        char word[MAX_LENGTH + 1];
        char display_suggest[MAX_INCRT_WORDS][MAX_SUGGESTIONS + 1][MAX_LENGTH];

        for (int i = 0, l = strlen(str); i < l; i++){
            char c = str[i];
            if (isalpha(c)){
                word[index] = tolower(c);
                index++;
            }
            else if (isdigit(c)){
                printf(COLOR_MAGENTA "\n Error no numbers...!" COLOR_RESET);
            }
            else if (index > 0){
                word[index] = '\0';
                index = 0;
                if (!searchTrie(root, word)){
                    printf(COLOR_RED "%s " COLOR_RESET, word);
                    strcpy(display_suggest[incrt_words][0], word);
                    char suggestions[MAX_SUGGESTIONS][MAX_LENGTH + 1];
                    int num_suggestions = suggest(word, cache, suggestions);
                    if (num_suggestions > 0){
                        for (int i = 0; i < num_suggestions; i++){
                            strcpy(display_suggest[incrt_words][i + 1], suggestions[i]);
                        }
                    }
                    incrt_words++;
                    printf("%s", " ");
                }
                else{
                    printf(COLOR_GREEN "%s " COLOR_RESET, word);
                    printf("%s", " ");
                }
            }
        }
        if (index > 0){
            word[index] = '\0';
            if (!searchTrie(root, word))
            {
                printf(COLOR_RED "%s " COLOR_RESET, word);
                strcpy(display_suggest[incrt_words][0], word);
                char suggestions[MAX_SUGGESTIONS][MAX_LENGTH + 1];
                int num_suggestions = suggest(word, cache, suggestions);
                if (num_suggestions > 0)
                {
                    for (int i = 0; i < num_suggestions; i++)
                    {
                        strcpy(display_suggest[incrt_words][i + 1], suggestions[i]);
                    }
                }
                incrt_words++;
            }
            else printf(COLOR_GREEN "%s " COLOR_RESET, word);
        }
        if (incrt_words > 0){
            printf(COLOR_MAGENTA "\nSuggestions for incorrect words:\n" COLOR_RESET);
            for (int i = 0; i < incrt_words; i++)
            {
                printf(COLOR_RED "%d. ", i + 1);
                for (int j = 0; j <= MAX_SUGGESTIONS; j++){
                    if (j == 0) printf("%s -> " COLOR_RESET, display_suggest[i][0]);
                    else printf(COLOR_CYAN "%s " COLOR_RESET, display_suggest[i][j]);
                }
                printf("\n");
            }
        }
        printf("\n");
    }
    return 0;
}