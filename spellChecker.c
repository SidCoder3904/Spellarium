# include <stdio.h>     // standard C library
# include <stdlib.h>    // for dynamic memory allocation
# include <stdbool.h>   // for bool datatype
# include <stdint.h>    // for uint32 types etc
# include <string.h>    // for string manipulation
# include <limits.h>    // for INT_MAX, max min values
# include <ctype.h>     // for formatting

// Bloom Filter variables
#define FILTER_SIZE 10000000
#define K 8     // no. of hash functions

// Trie variables
#define MAX_LENGTH 50
#define MAX_SUGGESTIONS 100
#define MAX_INCRT_WORDS 25
#define N 26    // no. of distinct characters in language


// hash functions
uint32_t djb2(const char* string) {
    uint32_t hash = 5381;
    int c;
    while ((c= *string++)) hash = (hash << 5) + hash + c;    // hash*33 + c
    return hash % FILTER_SIZE;
}

uint32_t jenkin(const char* str) {
    uint32_t hash = 0;
    while(*str) {
        hash += *str++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash % FILTER_SIZE;
}

void insertFilter(bool* filter, char* word) {    // bloom filter insertion
    for(int i=0; i<K; i++) filter[(djb2(word)*i + jenkin(word))%FILTER_SIZE] = 1;
}

int searchFilter(bool* filter, char* word) {      // searching in bloom filter
    for(int i=0; i<K; i++) if(!filter[(djb2(word)*i + jenkin(word))%FILTER_SIZE]) return 0;
    return 1;
}

// Trie functions
int smallest(int a, int b, int c) {      // helper function returns min(a, b, c)
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

typedef struct node {   // trie node
    struct node *child[26];
    int isEOW;
} TRIE_NODE;

TRIE_NODE* createNode() {
    TRIE_NODE* pNode = malloc(sizeof(TRIE_NODE));
    pNode->isEOW = 0;
    for(int i=0; i<26; i++) pNode->child[i] = NULL;
    return pNode;
}

void insertTrie(TRIE_NODE* root, char* word){
    TRIE_NODE* pCrawl = root;
    int len = strlen(word), idx;
    for(int i=0; i<len; i++) {
        idx = word[i] - 'a';
        if(!pCrawl->child[idx]) pCrawl->child[idx] = createNode();
        pCrawl = pCrawl->child[idx];
    }
    pCrawl->isEOW = 1;
}

int searchTrie(TRIE_NODE* root, char* word){
    TRIE_NODE* pCrawl = root;
    int len = strlen(word), idx;
    for(int i=0; i<len; i++) {
        idx = word[i] - 'a';
        if(!pCrawl->child[idx]) return 0;
        pCrawl = pCrawl->child[idx];
    }
    return (pCrawl->isEOW);
}

void display(TRIE_NODE* root, char* word, int level){
    if(root->isEOW) word[level] = '\0';
    for(int i=0; i<26; i++){
        if(root->child[i]!=NULL) {
            word[level] = i + 'a';
            display(root->child[i], word, level+1);
        }
    }
}

// populating bloom filter and trie with Dictionary words
void loadDictionary(bool* filter, TRIE_NODE* root){
    FILE *file = fopen("dictionary.txt", "r");
    char word[50];
    while(fscanf(file, "%s", word) != EOF) {
        insertFilter(filter, word);
        insertTrie(root, word);
    }
    fclose(file);
}

int levenshteinDistance(char* s1, char* s2){    // algorithm used to find similar correct words
    int n = strlen(s1), m = strlen(s2);
    if(!(n & m)) return 0;
    int cost = 0;
    int dp[MAX_LENGTH + 1][MAX_LENGTH + 1];
    for(int i=0; i<=n; i++) dp[i][0] = i;
    for(int j=0; j<=m; j++) dp[0][j] = j;
    for(int i=1; i<=n; i++) {
        for(int j=1; j<=m; j++) {
            cost = (s1[i-1] == s2[j-1]) ? 0 : 1;
            dp[i][j] = smallest(dp[i-1][j] + 1, dp[i][j-1] + 1, dp[i-1][j-1] + cost);
        }
    }
    return dp[n][m];
}

double jaroWinklerDistance(char* s1, char* s2) {    // jaro winkler similarity
    if(strcmp(s1, s2) == 0) return 1;
    int n = strlen(s1), m = strlen(s2);
    int max_dist = (n > m ? n : m)/2 - 1;
    int match = 0;
    int* h1 = calloc(n, sizeof(int));
    int* h2 = calloc(m, sizeof(int));
    for(int i=0; i<n; i++) {
        for (int j=0>i-max_dist ? 0 : i-max_dist; j<(m<i+max_dist+1 ? m : max_dist+1); j++) {
            if (s1[i] == s2[j] && h2[j] == 0) {
				h1[i] = 1;
				h2[j] = 1;
				match++;
				break;
			}
        }
    }
    if(match == 0) return 0;
	double t = 0;
	int point = 0;
    for(int i=0; i<n; i++) {
        if(h1[i]) {
            while(h2[point] == 0) point++;
            if (s1[i] != s2[point++]) t++;
        }
    }
    return (((double)match) / ((double)n) + ((double)match) / ((double)m) + ((double)match - t/2) / ((double)match)) / 3.0;
}

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
        printf("\nLevenshyein Distance: %d\nJaro-Winkler Similarity:%lf", levenshteinDistance(input, input2), jaroWinklerDistance(input, input2));
    }

    return 0;
}