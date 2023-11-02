# include <pthread.h>
// Bloom Filter variables
#define FILTER_SIZE 10000000
#define K 1     // no. of hash functions

// Trie variables
#define MAX_LENGTH 50
#define MAX_SUGGESTIONS 10
#define MAX_INCRT_WORDS 1000
#define CACHE_SIZE 100
#define N 26    // no. of distinct characters in language

#define INT_MAX 2147483647

// Defining colors for formatting
#define COLOR_BLUE "\x1b[34m"
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

// helper functions returns min(a, b, c)
int greater(int x, int y){
    return x>y?x:y;
}

int smaller(int x, int y){
    return x>y?y:x;
}

int smallest(int a, int b, int c) {
    return a < b ? (a < c ? a : c) : (b < c ? b : c);
}

// hash functions
uint32_t djb2(const char* string) {     // DJB_2 Hash
    uint32_t hash = 5381;
    int c;
    while ((c= *string++)) hash = (hash << 5) + hash + c;    // hash*33 + c
    return hash % FILTER_SIZE;
}

uint32_t jenkin(const char* str) {      // Jenkin's Hash
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

// Bloom Filter functions
void insertFilter(bool* filter, char* word) {    // bloom filter insertion
    for(int i=0; i<K; i++) filter[(djb2(word)*i + jenkin(word))%FILTER_SIZE] = 1;
}

bool searchFilter(bool* filter, char* word) {      // searching in bloom filter
    for(int i=0; i<K; i++) if(!filter[(djb2(word)*i + jenkin(word))%FILTER_SIZE]) return false;
    return true;
}

// Trie functions
typedef struct node {   // trie node
    struct node *child[26];
    bool isEOW;
} TRIE_NODE;

TRIE_NODE* createNode() {
    TRIE_NODE* pNode = (TRIE_NODE*)malloc(sizeof(TRIE_NODE));
    pNode->isEOW = false;
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
    pCrawl->isEOW = true;
}

bool searchTrie(TRIE_NODE* root, char* word){
    TRIE_NODE* pCrawl = root;
    int len = strlen(word), idx;
    for(int i=0; i<len; i++) {
        idx = word[i] - 'a';
        if(!pCrawl->child[idx]) return false;
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

typedef struct filter_args {
    char* dict;
    bool* filter;
} FILTER_ARGS;

typedef struct trie_args {
    char* dict;
    TRIE_NODE* root;
} TRIE_ARGS;

void* filterThread(void* arg) {
    char word[50];      // maximum word lenght
    FILTER_ARGS* args = (FILTER_ARGS*) arg;
    char* dict_file = args->dict;
    FILE* dict_ptr = fopen(dict_file, "r");
    bool* filter = args->filter;
    while(fscanf(dict_ptr, "%s", word) != EOF) insertFilter(filter, word);
    printf(COLOR_GREEN "Dictionary loaded on filter successfully\n" COLOR_RESET);
    return NULL;
}

void* trieThread(void* arg) {
    char word[50];      // maximum word lenght
    TRIE_ARGS* args = (TRIE_ARGS*) arg;
    char* dict_file = args->dict;
    FILE* dict_ptr = fopen(dict_file, "r");
    TRIE_NODE* root = args->root;
    while(fscanf(dict_ptr, "%s", word) != EOF) insertTrie(root, word);
    printf(COLOR_GREEN "Dictionary loaded on trie successfully\n" COLOR_RESET);
    return NULL;
}

// populating bloom filter and trie with Dictionary words
void loadDictionary(char* dict_file, bool* filter, TRIE_NODE* root) {
    pthread_t filter_thread, trie_thread;
    FILTER_ARGS filter_args = {dict_file, filter};
    TRIE_ARGS trie_args = {dict_file, root};
    if(pthread_create(&filter_thread, NULL, filterThread, &filter_args) & pthread_create(&trie_thread, NULL, trieThread, &trie_args)) printf(COLOR_RED "Error occured in threading\n" COLOR_RESET);
    if(pthread_join(filter_thread, NULL) & pthread_join(trie_thread, NULL)) printf(COLOR_RED "Error occured in threading\n" COLOR_RESET);
    filter = filter_args.filter;
    root = trie_args.root;
}

int levenshteinDistance(const char *s, const char *t){
    int n = strlen(s);
    int m = strlen(t);

    if (n == 0)
        return m;
    if (m == 0)
        return n;

    int dp[MAX_LENGTH + 1][MAX_LENGTH + 1];
    for (int i = 0; i <= n; i++)
        dp[i][0] = i;
    for (int j = 0; j <= m; j++)
        dp[0][j] = j;

    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            int cost = (s[i - 1] == t[j - 1]) ? 0 : 1;
            dp[i][j] = smallest(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost);
        }
    }
    return dp[n][m];
}

double jaroWinklerDistance(char* s1, char* s2){
    if (s1 == s2)
        return 1.0;

    int len1 = strlen(s1), len2 = strlen(s2);
    int max_dist = floor(greater(len1, len2) / 2) - 1;
    int match = 0;
 
    int* hash_s1 = calloc(len1, sizeof(int));
    int* hash_s2 = calloc(len2, sizeof(int));
 
    for(int i=0; i<len1; i++) {
        for(int j=greater(0, i - max_dist);
             j<smaller(len2, i + max_dist + 1); j++)
            if (s1[i] == s2[j] && hash_s2[j] == 0) {
                hash_s1[i] = 1;
                hash_s2[j] = 1;
                match++;
                break;
            }
    }
    if (match == 0) return 0.0;
    
    double t = 0; 
    int point = 0;

    for(int i=0; i < len1; i++)
        if(hash_s1[i]) {
            while(hash_s2[point] == 0) point++;
            if(s1[i] != s2[point++]) t++;
        }

    t /= 2;
    return (((double)match) / ((double)len1) + ((double)match) / ((double)len2) + ((double)match - t) / ((double)match)) / 3.0;
}

struct LRUCacheQueueNode{
    char val[100];
    struct LRUCacheQueueNode* prev;
    struct LRUCacheQueueNode* forw;
};

struct LRUCacheMapNode{
    struct LRUCacheQueueNode* nd;
    struct LRUCacheMapNode* prev;
    struct LRUCacheMapNode* forw;
};

struct LRUCacheQueueNode* createQueueNode(char* string) {
    struct LRUCacheQueueNode* ans = (struct LRUCacheQueueNode*)malloc(sizeof(struct LRUCacheQueueNode));
    strcpy(ans->val, string);
    ans->forw = NULL;
    ans->prev = NULL;
    return ans;
}

struct LRUCacheMapNode* createHashNode(char* string){
    struct LRUCacheMapNode* ans=(struct LRUCacheMapNode*)malloc(sizeof(struct LRUCacheMapNode));
    ans->nd=createQueueNode(string);
    ans->prev=NULL;
    ans->forw=NULL;
    return ans;
}

struct LRUCacheMap{
    struct LRUCacheMapNode** arr;
};

struct LRUCache{
    struct LRUCacheQueueNode *head, *tail;
    struct LRUCacheMapNode** map;
    int size;
    int maxSize;
};

struct LRUCache* createLRUCache(int sz){
    struct LRUCache* res=(struct LRUCache*)malloc(sizeof(struct LRUCache));
    res->map=(struct LRUCacheMapNode**)malloc(sizeof(struct LRUCacheMapNode*)*sz);
    for (int i=0; i<sz; i++) res->map[i]=NULL;
    res->size=0;
    res->maxSize=sz;
    res->head=createQueueNode("");
    res->tail=createQueueNode("");
    res->head->forw=res->tail;
    res->tail->prev=res->head;
    return res;
};

void insert(struct LRUCache* obj, struct LRUCacheQueueNode* nd){
    nd->prev = obj->head;
    nd->forw = obj->head->forw;
    obj->head->forw->prev = nd;
    obj->head->forw = nd;
}

struct LRUCacheQueueNode* deleteQueue(struct LRUCache* obj){
    struct LRUCacheQueueNode* previousNode = obj->tail->prev;
    previousNode->prev->forw = obj->tail;
    obj->tail->prev = previousNode->prev;
    return previousNode;
}

void deleteHsh(struct LRUCache* obj, struct LRUCacheQueueNode* QNode){
    int ind=djb2(QNode->val)%(obj->maxSize);
    struct LRUCacheMapNode* temp=obj->map[ind];
    while (temp->nd!=QNode) temp=temp->forw;
    if (obj->map[ind]==temp) obj->map[ind]=temp->forw;
    else{
        temp->prev->forw=temp->forw;
        if (temp->forw!=NULL) temp->forw->prev=temp->prev;
    }
    free(temp);
}

void lRUCachePut(struct LRUCache* obj, char* string) {
    int ind=djb2(string)%(obj->maxSize);
    
    if (obj->map[ind]!=NULL){
        struct LRUCacheMapNode* temp=obj->map[ind];
        obj->map[ind]=createHashNode(string);
        obj->map[ind]->forw=temp;
        temp->prev=obj->map[ind];
    }
    else{
        obj->map[ind]=createHashNode(string);
    }
    insert(obj, obj->map[ind]->nd);
    if (obj->size<obj->maxSize) obj->size++;
    else{
        struct LRUCacheQueueNode* QNode=deleteQueue(obj);
        deleteHsh(obj, QNode);
        free(QNode);
    }
}

void printQueue(struct LRUCache* obj){
    printf("\nThe Cache Queue is: ");
    struct LRUCacheQueueNode* temp=obj->head->forw;
    while (temp!=obj->tail){
        printf("%s ", temp->val);
        temp=temp->forw;
    }
    printf("\n");
}

struct LRUCacheMapNode* searchCache(struct LRUCache* obj, char* string){
    int ind=djb2(string)%(obj->maxSize);
    struct LRUCacheMapNode* temp=obj->map[ind];
    while (temp!=NULL && strcmp(temp->nd->val, string)!=0) temp=temp->forw;
    return temp;
}

void LRUCacheGet(struct LRUCache* obj, struct LRUCacheMapNode* temp){
    temp->nd->prev->forw=temp->nd->forw;
    temp->nd->forw->prev=temp->nd->prev;
    insert(obj, temp->nd);
}

void removeNonAlphabetical(char *word) {
    int i, j = 0;

    for (i = 0; word[i] != '\0'; i++) {
        if (isalpha(word[i])) {
            word[j] = tolower(word[i]);
            j++;
        }
    }
    word[j] = '\0';
}