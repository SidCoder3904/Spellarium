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

// analysis/development functions
int suggest(char *word, struct LRUCache* obj, char suggestions[MAX_SUGGESTIONS][MAX_LENGTH + 1]){
    char dict_word[MAX_LENGTH];
    double temp_JaroWinklerValue = 0;
    int temp_LevenshteinValue = INT_MAX;
    FILE *dict_ptr = fopen(DICT_FILE, "r");
    if(dict_ptr == NULL) {
        perror(COLOR_RED "Error loading dictionary for suggestions\n" COLOR_RESET);
        exit(1);
    }

    while (fscanf(dict_ptr, "%s", dict_word) != EOF){
        int distance = levenshteinDistance(word, dict_word);
        double jaroWinklerValue = jaroWinklerDistance(word, dict_word);
        if (distance < temp_LevenshteinValue){
            temp_LevenshteinValue=distance;
            lRUCachePut(obj, dict_word);
        }
        else if (distance == temp_LevenshteinValue && jaroWinklerValue >= temp_JaroWinklerValue){
            temp_JaroWinklerValue=jaroWinklerValue;
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

    fclose(dict_ptr);
}

int main() {
    // loading dictionary on bloom filter and trie
    TRIE_NODE* root=createNode();
    bool* filter = calloc(FILTER_SIZE, sizeof(bool));
    loadDictionary(dict_file, filter, root);
    printf(COLOR_BLUE "Dictionary loaded successfully.\n" COLOR_RESET);
    int ch;
    char str[100];
    char word[MAX_LENGTH + 1];
    double accuracyFilter=0;
    LARGE_INTEGER start, end, freq;
    double time_used_trie;
    double time_used_filter;
    double faster;
    int accurate_comp=0;
    int num_comp=0;
    bool checkTrie;
    bool checkFilter;

    createQueue();
    struct LRUCache* cache=lRUCacheCreate(MAX_SUGGESTIONS);

    while (1){
        printf(COLOR_YELLOW "Select the mode you want to enter:-\n1. Spell checking and autocorrect\n2. Comparison mode\n3. Optimisation mode\n4. Quit\n" COLOR_RESET);
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

                for(int i=0, l=strlen(str); i<l; i++) {
                    char c = str[i];
                    if(isalpha(c)) word[index++] = tolower(c);
                    else if(isdigit(c)) printf(COLOR_MAGENTA "\n Error no numbers...!" COLOR_RESET);
                    else if(index > 0) {
                        word[index] = '\0';
                        index = 0;
                        if(!searchTrie(root, word)){
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
                        }
                        else printf(COLOR_GREEN "%s " COLOR_RESET, word);
                    }
                }
                if(index>0) {
                    word[index] = '\0';
                    if(!searchTrie(root, word)) {
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
                if(incrt_words>0) {
                    printf(COLOR_MAGENTA "\nSuggestions for incorrect words:\n" COLOR_RESET);
                    for (int i=0; i<incrt_words; i++) {
                        printf(COLOR_RED "%d) ", i+1);
                        for (int j=0; j<=MAX_SUGGESTIONS; j++) {
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
            while (1){
                printf(COLOR_MAGENTA "Enter a word or type exit to leave:\n" COLOR_RESET);
                fgets(word, sizeof(word), stdin);
                word[strcspn(word, "\n")] = '\0';
                if (!strcmp(word, "exit")) break;

                LARGE_INTEGER start, end, freq;
                bool checkTrie, checkFilter;
                double time_used_trie, time_used_filter, faster, accuracyFilter = 0;
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

                if (checkTrie && checkFilter){
                    printf(COLOR_MAGENTA "The word %s is present in both Tries and Bloom Filter\n" COLOR_RESET, word);
                    accurate_comp++;
                }
                else if (!checkTrie && checkFilter){
                    printf(COLOR_MAGENTA "The word %s is absent in Tries but present in Bloom Filter\n" COLOR_RESET, word);
                }
                else if (checkTrie && !checkFilter){
                    printf(COLOR_MAGENTA "The word %s is present in Tries but absent in Bloom Filter\n" COLOR_RESET, word);
                }
                else{
                    printf(COLOR_MAGENTA "The word %s is present in neither Tries nor Bloom Filter\n" COLOR_RESET, word);
                    accurate_comp++;
                }
                num_comp++;
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
        }
        else if (ch==3){

        }
        else if (ch==4) break;
        else printf("Invalid value!\n");
        printf("\n");
    }
    
    return 0;
}