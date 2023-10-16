# include <stdio.h>
# include <stdlib.h>
# include <stdint.h>
# include <string.h>
# include <stdbool.h>
# include <ctype.h>

const int filterSize = 1e8+10;
bool* bloomFilter;

//MurmurHash
uint32_t murmur3_32(const char *key, uint32_t len, uint32_t seed) {
    static const uint32_t c1 = 0xcc9e2d51;
    static const uint32_t c2 = 0x1b873593;
    static const uint32_t r1 = 15;
    static const uint32_t r2 = 13;
    static const uint32_t m = 5;
    static const uint32_t n = 0xe6546b64;

    uint32_t hash = seed;

    const int nblocks = len / 4;
    const uint32_t *blocks = (const uint32_t *) key;
    int i;
    for (i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;
        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }

    const uint8_t *tail = (const uint8_t *) (key + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];

        k1 *= c1;
        k1 = (k1 << r1) | (k1 >> (32 - r1));
        k1 *= c2;
        hash ^= k1;
    }

    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

int lookup(bool* bitarray, int arrSize, char* s){ 
    for (int i=0; i<10; i++){
        if (bitarray[murmur3_32(s, strlen(s), i)%filterSize]==0) return 0;
    }
    return 1;
} 
  
// insert operation 
void insert(bool* bitarray, int arrSize, char* s){ 
    // check if the element in already present or not 
    uint32_t hsh[10];
    for (int i=0; i<10; i++){
        bitarray[murmur3_32(s, strlen(s), i)%filterSize]=1;
    }
} 

void loadDictionary(bool* bloomFilter){
    FILE *file = fopen("dictionary.csv", "r");
    char word[100];
    while (fscanf(file, "%s", word) != EOF) insert(bloomFilter, strlen(word), word);
    fclose(file);
}

int main(){
    bloomFilter=(bool*)calloc(filterSize, sizeof(int));
    loadDictionary(bloomFilter);
    char str[100];
    while(1){
        printf("Enter the word\n");
        scanf("%s", &str);
        for(int i = 0; str[i]; i++){
            str[i] = tolower(str[i]);
        }
        if (lookup(bloomFilter, strlen(str), str)) printf("Word present!\n");
        else printf("Word absent!\n");
    }
    return 0;
}