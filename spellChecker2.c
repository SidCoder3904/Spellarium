# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <stdint.h>
# include <string.h>
# include <ctype.h>

#define filterSize 10000000
#define k 8

// hash functions
uint32_t djb2(const char* string) {
    uint32_t hash = 5381;
    int c;
    while ((c= *string++)) hash = (hash << 5) + hash + c;    // hash*33 + c
    return hash % filterSize;
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
    return hash % filterSize;
}

bool* filter;

void insert(char* string) {    // bloom filter insertion
    for(int i=0; i<k; i++) filter[(djb2(string)*i + jenkin(string))%filterSize] = 1;
}

int search(char* string) {      // searching in bloom filter
    for(int i=0; i<k; i++) if(!filter[(djb2(string)*i + jenkin(string))%filterSize]) return 0;
    return 1;
}

void loadDictionary(){
    FILE *file = fopen("dictionary.csv", "r");
    char string[100];
    while (fscanf(file, "%s", string) != EOF) insert(string);
    fclose(file);
}

int main() {
    // initializing bloom filter
    filter = calloc(filterSize, sizeof(bool));

    // populating bloom filter with dictionary
    loadDictionary();

    // testing bloom filter
    while(1) {
        printf("\nEnter word to search: ");
        char input[50];
        fscanf(stdin, "%s", input);
        for(int i = 0; input[i]; i++) input[i] = tolower(input[i]);
        if(search(input)) printf("\nword present"); // search
        else printf("\nword not present");
    }
    
    return 0;
}