#include "global.h"
#include "array.h"
#define CHAR_SIZE 75

/* Code taken from https://www.techiedelight.com/trie-implementation-insert-search-delete/
 * with little modifications
 */

// Data structure to store a Trie node
struct Trie{
    bool isLeaf;
    struct Trie* character[CHAR_SIZE]; 
    int count;
    int color;
};
 
struct Trie* getNewTrieNode();
 
int insertTrie(struct Trie *head, Array_char str, int color);
 
struct Trie* searchTrie(struct Trie* head, Array_char str);

struct Trie* searchTriePhenotype(struct Trie* head, Array_char str);

bool isLeafTrie(struct Trie* head);
 
bool hasChildrenTrie(struct Trie* curr);
 
bool deletionTrie(struct Trie **curr, char* str);