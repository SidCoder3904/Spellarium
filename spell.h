// dictionary file name
#define DICT_FILE "dictionary.txt"
#define PARA_FILE "paragraph.txt"

// Bloom Filter variables
#define FILTER_SIZE 10000000
#define K 8     // no. of hash functions

// Trie variables
#define MAX_LENGTH 50
#define MAX_SUGGESTIONS 10
#define MAX_INCRT_WORDS 1000
#define N 26    // no. of distinct characters in language

#define INT_MAX 2147483647

// MultiThreading variables
#define N_THREADS 4

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

void* filterThread(void* filter) {
    char word[50];      // maximum word lenght
    FILE* dict_ptr = fopen(DICT_FILE, "r");
    if(dict_ptr == NULL) {
        perror(COLOR_RED "Error loading Dictionary\n" COLOR_RESET);
        exit(1);
    }
    while(fscanf(dict_ptr, "%s", word) != EOF) insertFilter((bool*) filter, word);
    printf(COLOR_GREEN "Dictionary loaded on filter successfully\n" COLOR_RESET);
    return NULL;
}

void* trieThread(void* root) {
    char word[50];      // maximum word lenght
    FILE* dict_ptr = fopen(DICT_FILE, "r");
    if(dict_ptr == NULL) {
        perror(COLOR_RED "Error loading Dictionary or wrong path/file for dictionary\n" COLOR_RESET);
        exit(1);
    }
    while(fscanf(dict_ptr, "%s", word) != EOF) insertTrie((TRIE_NODE*) root, word);
    printf(COLOR_GREEN "Dictionary loaded on trie successfully\n" COLOR_RESET);
    return NULL;
}

// populating bloom filter and trie with Dictionary words
void loadDictionary(bool* filter, TRIE_NODE* root) {
    pthread_t filter_thread, trie_thread;
    if(pthread_create(&filter_thread, NULL, filterThread, filter) & pthread_create(&trie_thread, NULL, trieThread, root)) printf(COLOR_RED "Error occured in threading\n" COLOR_RESET);
    if(pthread_join(filter_thread, NULL) & pthread_join(trie_thread, NULL)) printf(COLOR_RED "Error occured in threading\n" COLOR_RESET);
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

struct queueNode{
    char val[100];
    struct queueNode* prev;
    struct queueNode* forw;
} *head, *tail;

struct hshNode{
    struct queueNode* nd;
    struct hshNode* prev;
    struct hshNode* forw;
};

struct queueNode* createQueueNode(char* string) {
    struct queueNode* ans = (struct queueNode*)malloc(sizeof(struct queueNode));
    strcpy(ans->val, string);
    ans->forw = NULL;
    ans->prev = NULL;
    return ans;
}

struct hshNode* createHashNode(char* string){
    struct hshNode* ans=(struct hshNode*)malloc(sizeof(struct hshNode));
    ans->nd=createQueueNode(string);
    ans->prev=NULL;
    ans->forw=NULL;
    return ans;
}

struct LRUCache{
    struct hshNode** arr;
    int size;
    int maxSize;
};


struct LRUCache* lRUCacheCreate(int capacity) {
    struct LRUCache* ans=(struct LRUCache*)malloc(sizeof(struct LRUCache));
    ans->arr=(struct hshNode**)malloc(sizeof(struct hshNode*)*capacity);
    for (int i=0; i<capacity; i++) ans->arr[i]=NULL;
    ans->size=0;
    ans->maxSize=capacity;
    return ans;
}

void insert(struct queueNode* nd){
    nd->prev = head;
    nd->forw = head->forw;
    head->forw->prev = nd;
    head->forw = nd;
}

struct queueNode* deleteQueue(){
    struct queueNode* previousNode = tail->prev;
    previousNode->prev->forw = tail;
    tail->prev = previousNode->prev;
    return previousNode;
}

void deleteHsh(struct LRUCache* obj, struct queueNode* QNode){
    int ind=djb2(QNode->val)%(obj->maxSize);
    struct hshNode* temp=obj->arr[ind];
    while (temp->nd!=QNode) temp=temp->forw;
    if (obj->arr[ind]==temp) obj->arr[ind]=temp->forw;
    else{
        temp->prev->forw=temp->forw;
        if (temp->forw!=NULL) temp->forw->prev=temp->prev;
    }
    free(temp);
}

void lRUCachePut(struct LRUCache* obj, char* string) {
    int ind=djb2(string)%(obj->maxSize);
    
    if (obj->arr[ind]!=NULL){
        struct hshNode* temp=obj->arr[ind];
        obj->arr[ind]=createHashNode(string);
        obj->arr[ind]->forw=temp;
        temp->prev=obj->arr[ind];
    }
    else{
        obj->arr[ind]=createHashNode(string);
    }
    insert(obj->arr[ind]->nd);
    if (obj->size<obj->maxSize) obj->size++;
    else{
        struct queueNode* QNode=deleteQueue();
        deleteHsh(obj, QNode);
        free(QNode);
    }
}

void printQueue(){
    struct queueNode* temp=head->forw;
    while (temp!=tail){
        printf("%s ", temp->val);
        temp=temp->forw;
    }
    printf("\n");
}

void createQueue(){
    head=createQueueNode("");
    tail=createQueueNode("");
    head->forw=tail;
    tail->prev=head;
}

struct hshNode* searchCache(struct LRUCache* obj, char* string){
    int ind=djb2(string)%(obj->maxSize);
    struct hshNode* temp=obj->arr[ind];
    while (temp!=NULL && strcmp(temp->nd->val, string)!=0) temp=temp->forw;
    return temp;
}

void lRUCacheGet(struct hshNode* temp){
    temp->nd->prev->forw=temp->nd->forw;
    temp->nd->forw->prev=temp->nd->prev;
    insert(temp->nd);
}

typedef struct suggest_thread {
    int thread_num;
    struct LRUCache* cache;
    char* suggestions;
} THREAD_INP;

// Function to break a part of the file into words
void processPart(char* part, struct LRUCache* cache, char suggestions[MAX_SUGGESTIONS][MAX_LENGTH + 1]) {
    char word[MAX_LENGTH];
    const char* delimiters = " \t\n";
    const char* token = strtok(part, delimiters);

    while (token != NULL) {
        strncpy(word, token, MAX_LENGTH);
        store_suggestions(word, cache, suggestions);
        token = strtok(NULL, delimiters);
    }
    // printf("**im done**");
}

void* processFile(void* thread_inp) {
    // int thread_num = *(int*)arg;
    THREAD_INP* input = (THREAD_INP*) thread_inp;
    FILE* file = fopen(PARA_FILE, "r");
    if(file == NULL) {
        perror(COLOR_RED "Error opening paragraph file" COLOR_RESET);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    long part = file_size / N_THREADS;
    long start = part * input->thread_num;
    long end = input->thread_num < N_THREADS - 1 ? start + part : file_size;

    if(input->thread_num > 0) {
        fseek(file, start, SEEK_SET);
        while(start < end) {
            int c = fgetc(file);
            if(c == ' ' || c == '\t' || c == '\n') break;
            start++;
        }
    }

    // Read and process the part
    fseek(file, start, SEEK_SET);
    char* chunk = (char*) malloc((end-start+1) * sizeof(char));
    fread(chunk, 1, end - start, file);
    chunk[end-start] = '\0';
    fclose(file);

    processPart(chunk, (struct LRUCache*) input->cache, (char*) input->suggestions);

    free(chunk);
    return NULL;
}

void part_file(struct LRUCache* cache, char suggestions[MAX_SUGGESTIONS][MAX_LENGTH + 1]) {     // size is number of elements probably put -1 for size not known
    pthread_t threads[N_THREADS];
    THREAD_INP thread_inp[N_THREADS];
    for(int i=0; i<N_THREADS; i++) {
        thread_inp[i].thread_num = i;
        thread_inp[i].cache = cache;
        thread_inp[i].suggestions = suggestions;
        pthread_create(&threads[i], NULL, processFile, &thread_inp[i]);
    }
    for(int i=0; i<N_THREADS; i++) pthread_join(threads[i], NULL);
}