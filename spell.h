// Bloom Filter variables
#define FILTER_SIZE 10000000
#define K 8     // no. of hash functions

// Trie variables
#define MAX_LENGTH 50
#define MAX_SUGGESTIONS 10
#define MAX_INCRT_WORDS 1000
#define N 26    // no. of distinct characters in language

#define INT_MAX 2147483647
#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_CYAN "\x1b[36m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET "\x1b[0m"

int maxi(int x, int y){
    return x>y?x:y;
}

int mini(int x, int y){
    return x>y?y:x;
}

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
    TRIE_NODE* pNode = (TRIE_NODE*)malloc(sizeof(TRIE_NODE));
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
    // If the strings are equal
    if (s1 == s2)
        return 1.0;
 
    // Length of two strings
    int len1 = strlen(s1), len2 = strlen(s2);
 
    // Maximum distance upto which matching
    // is allowed
    int max_dist = floor(maxi(len1, len2) / 2) - 1;
 
    // Count of matches
    int match = 0;
 
    // Hash for matches
    int* hash_s1 = calloc(len1, sizeof(int));
    int* hash_s2 = calloc(len2, sizeof(int));
 
    // Traverse through the first string
    for (int i = 0; i < len1; i++) {
 
        // Check if there is any matches
        for (int j = maxi(0, i - max_dist);
             j < mini(len2, i + max_dist + 1); j++)
 
            // If there is a match
            if (s1[i] == s2[j] && hash_s2[j] == 0) {
                hash_s1[i] = 1;
                hash_s2[j] = 1;
                match++;
                break;
            }
    }
 
    // If there is no match
    if (match == 0)
        return 0.0;
 
    // Number of transpositions
    double t = 0;
 
    int point = 0;
 
    // Count number of occurrences
    // where two characters match but
    // there is a third matched character
    // in between the indices
    for (int i = 0; i < len1; i++)
        if (hash_s1[i]) {
 
            // Find the next matched character
            // in second string
            while (hash_s2[point] == 0)
                point++;
 
            if (s1[i] != s2[point++])
                t++;
        }
 
    t /= 2;
 
    // Return the Jaro Similarity
    return (((double)match) / ((double)len1)
            + ((double)match) / ((double)len2)
            + ((double)match - t) / ((double)match))
           / 3.0;
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
    struct queueNode* temp=head->forw;
    while (temp!=tail){
        strcpy(suggestions[num_suggestions], temp->val);
        num_suggestions++;
        temp=temp->forw;
    }

    return num_suggestions;

    fclose(file);
}