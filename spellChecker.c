# include <stdio.h>     // standard C library
# include <stdlib.h>    // for dynamic memory allocation
# include <stdbool.h>   // for bool datatype
# include <stdint.h>    // for uint32 types etc
# include <string.h>    // for string manipulation
# include <ctype.h>     // for formatting
# include <math.h>      // for some math functions
# include <pthread.h>   // for multithreading
# include <windows.h>   // for computational analysis and comparision analysis
# include "spell.h"     // our header file with bloom filter, trie and lru cache etc functionality

// global variables
// dictionary file name
char *dict_file = "dictionary.txt";

// analysis/development functions
int suggest(char *word, struct LRUCache* obj, char suggestions[MAX_SUGGESTIONS][MAX_LENGTH + 1]){

    char dict_word[MAX_LENGTH + 1];

    double tempJaroWinklerValue = 0;
    int tempLevenshteinValue = INT_MAX;
    FILE *file = fopen("dictionary.txt", "r");

    while (fscanf(file, "%s", dict_word) != EOF){
        int distance = levenshteinDistance(word, dict_word);
        double jaroWinklerValue = jaroWinklerDistance(word, dict_word);
        if (distance < tempLevenshteinValue){
            tempLevenshteinValue=distance;
            lRUCachePut(obj, dict_word);
        }
        else if (distance == tempLevenshteinValue && jaroWinklerValue >= tempJaroWinklerValue){
            tempJaroWinklerValue=jaroWinklerValue;
            lRUCachePut(obj, dict_word);
        }
    }

    int num_suggestions = 0;
    struct LRUCacheQueueNode* temp=obj->head->forw;
    while (temp!=obj->tail){
        strcpy(suggestions[num_suggestions], temp->val);
        num_suggestions++;
        temp=temp->forw;
    }

    return num_suggestions;

    fclose(file);
}

int main() {
    // loading dictionary on bloom filter and trie
    TRIE_NODE* root=createNode();
    bool* filter = calloc(FILTER_SIZE, sizeof(bool));
    loadDictionary(dict_file, filter, root);
    int ch;
    char str[100];
    char word[MAX_LENGTH + 1];
    char suggestions[MAX_SUGGESTIONS][MAX_LENGTH + 1];
    double accuracyFilter=0;
    LARGE_INTEGER start, end, freq;
    double time_used_trie;
    double time_used_filter;
    double faster;
    int accurate_comp=0;
    int num_comp=0;
    bool checkTrie;
    bool checkFilter;

    struct LRUCache* cache1=createLRUCache(MAX_SUGGESTIONS);
    struct LRUCache* cache2=createLRUCache(CACHE_SIZE);

    while (1){
        printf(COLOR_YELLOW "Select the mode you want to enter:-\n1. Spell checking and autocorrect\n2. Analysis mode\n3. Optimisation mode\n4. Quit\n" COLOR_RESET);
        scanf("%d", &ch);
        getchar();
        if (ch==1){
            while (1){
                printf(COLOR_MAGENTA "Enter a sentence or type exit to leave:\n" COLOR_RESET);
                fgets(str, sizeof(str), stdin);
                str[strcspn(str, "\n")] = 0;
                if (!strcmp(str, "exit")) break;
                printf(COLOR_MAGENTA "The sentence after checking is: \n" COLOR_RESET);
                int index = 0, incrt_words = 0;

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
                            int num_suggestions = suggest(word, cache1, suggestions);
                            if (num_suggestions > 0){
                                for (int i = 0; i < num_suggestions; i++){
                                    strcpy(display_suggest[incrt_words][i + 1], suggestions[i]);
                                }
                            }
                            incrt_words++;
                            printf("%s", " ");
                        }
                        else {
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
                        int num_suggestions = suggest(word, cache1, suggestions);
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
        }
        else if (ch==2){
            accurate_comp=0;
            num_comp=0;
            time_used_trie=0;
            time_used_filter=0;
            FILE *file;
            file = fopen("input.txt", "r");
            while (fscanf(file, "%s", word) == 1){
                removeNonAlphabetical(word);

                QueryPerformanceCounter(&start); 
                checkTrie=searchTrie(root, word); 
                QueryPerformanceCounter(&end);
                QueryPerformanceFrequency(&freq);
                time_used_trie+= (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
                
                QueryPerformanceCounter(&start); 
                checkFilter=searchFilter(filter, word);
                QueryPerformanceCounter(&end);
                QueryPerformanceFrequency(&freq);
                time_used_filter+= (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

                if ((checkTrie && checkFilter) || (!checkTrie && !checkFilter)) accurate_comp++;
                num_comp++;
            }
            fclose(file);
            accuracyFilter=accurate_comp*100.0/num_comp;
            printf(COLOR_CYAN "\nThe accuracy of Tries is %f percent\n", num_comp*100.0/num_comp);
            printf("The accuracy of Bloom Filter %f percent\n\n" COLOR_RESET, accuracyFilter);
            if (time_used_trie>time_used_filter){
                faster=(time_used_trie-time_used_filter)*100/time_used_trie;
                printf(COLOR_GREEN "Bloom Filter is %f percent faster\n" COLOR_RESET, faster);
            }
            else{
                faster=(time_used_filter-time_used_trie)*100/time_used_filter;
                printf(COLOR_RED "Bloom Filter is %f percent slower\n" COLOR_RESET, faster);
            }
            printf("\n");
        }
        else if (ch==3){
            FILE *file;
            file = fopen("input.txt", "r");
            printf(COLOR_MAGENTA "The incorrect words in the file and their suggestions are:-\n" COLOR_RESET);
            while (fscanf(file, "%s", word) == 1){
                removeNonAlphabetical(word);
                struct LRUCacheMapNode* temp=searchCache(cache2, str);
                if (temp==NULL){
                    if (!searchTrie(root, word)){
                        printf(COLOR_RED "%s -> " COLOR_RESET, word);
                        suggest(word, cache1, suggestions);
                        for (int i = 0; i < MAX_SUGGESTIONS; i++){
                            printf(COLOR_CYAN "%s " COLOR_RESET, suggestions[i]);
                        }
                        printf("\n");
                    }
                    else lRUCachePut(cache2, word);
                }
                else LRUCacheGet(cache2, temp);
            }
            fclose(file);
        }
        else if (ch==4) break;
        else printf("Invalid value!\n");
        printf("\n");
    }
    
    return 0;
}