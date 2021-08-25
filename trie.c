#include "trie.h"

/* Code taken from https://www.techiedelight.com/trie-implementation-insertTrie-searchTrie-delete/
 * with modifications to fit the problem
 */
 
// Function that returns a new Trie node
struct Trie* getNewTrieNode(){
    struct Trie* node = (struct Trie*)malloc(sizeof(struct Trie));
    node->isLeaf = false;
    node->color = -1;
    node->count = 0;
 
    for (int i = 0; i < CHAR_SIZE; i++) {
        node->character[i] = NULL;
    }
 
    return node;
}
 
// Iterative function to insertTrie a string into a Trie
int insertTrie(struct Trie *head, Array_char str, int color){
    // start from the root node
    struct Trie* curr = head;
    for (int i = 0; i < str.used && str.data[i] != '\0'; i++){
        // create a new node if the path doesn't exist
        if (curr->character[str.data[i] - '0'] == NULL) {
            curr->character[str.data[i] - '0'] = getNewTrieNode();
        }
 
        // go to the next node
        curr = curr->character[str.data[i] - '0'];
    }
 
    // mark the current node as a leaf
    curr->isLeaf = true;
    curr->count++;
    if(curr->color == -1){
        curr->color = color;
        color++;
        return color;
    }
    return curr->color;
}
 

struct Trie* searchTrie(struct Trie* head, Array_char str){
    if (head == NULL) {
        return NULL;
    }
 
    struct Trie* curr = head;
    for (int i = 0; i < str.used && str.data[i] != '\0'; i++){
        // go to the next node
        curr = curr->character[str.data[i] - '0'];
        // if the string is invalid (reached end of a path in the Trie)
        if (curr == NULL) {
            return NULL;
        }
    }


    return curr;
}

struct Trie* searchTriePhenotype(struct Trie* head, Array_char str){
    if (head == NULL) {
        return NULL;
    }
    struct Trie* curr = head;
    for (int i = 0; i < str.used && str.data[i] != '\0'; i+=2){
        // go to the next node
        if(str.data[i] < 91 && str.data[i] > 64){
            curr = curr->character[str.data[i] - '0'];
            if(curr->character[str.data[i] - '0']){

                curr = curr->character[str.data[i] - '0'];
            } else if(curr->character[str.data[i+1] - '0']){
                curr = curr->character[str.data[i+1] - '0'];
            } else {
                curr = curr->character[str.data[i] - 16];
            }
        } else if(str.data[i+1] < 91 && str.data[i+1] > 64){
            if(curr->character[str.data[i+1] - '0']){
                curr = curr->character[str.data[i+1] - '0'];
            } else if(curr->character[str.data[i] - '0']){
                curr = curr->character[str.data[i] - '0'];
            } else {
                curr = curr->character[str.data[i+1] - 16];
            }
            if(curr->character[str.data[i+1] - '0']){
                curr = curr->character[str.data[i+1] - '0'];
            } else if(curr->character[str.data[i] - '0']){
                curr = curr->character[str.data[i] - '0'];
            } else {
                curr = curr->character[str.data[i+1] - 16];
            }
        } else{
            curr = curr->character[str.data[i] - '0'];
            curr = curr->character[str.data[i+1] - '0'];
        }
        
        if (curr == NULL) {
            return NULL;
        }
    }

    return curr;
}

bool isLeafTrie(struct Trie* head){
    return head->isLeaf;
}
 
// Returns 1 if a given Trie node has any children
bool hasChildrenTrie(struct Trie* curr){
    for (int i = 0; i < CHAR_SIZE; i++){
        if (curr->character[i]) {
            return true;       // child found
        }
    }
 
    return false;
}
 
// Recursive function to delete a string from a Trie
bool deletionTrie(struct Trie **curr, char* str){
    // return 0 if Trie is empty
    if (*curr == NULL) {
        return false;
    }
 
    // if the end of the string is not reached
    if (*str){
        // recur for the node corresponding to the next character in
        // the string and if it returns 1, delete the current node
        // (if it is non-leaf)
        if (*curr != NULL && (*curr)->character[*str - '0'] != NULL &&
            deletionTrie(&((*curr)->character[*str - '0']), str + 1) &&
            (*curr)->isLeaf == 0){
            if (!hasChildrenTrie(*curr)){
                free(*curr);
                (*curr) = NULL;
                return true;
            } else {
                return false;
            }
        }
    }
 
    // if the end of the string is reached
    if (*str == '\0' && (*curr)->isLeaf){
        // if the current node is a leaf node and doesn't have any children
        if (!hasChildrenTrie(*curr)){
            free(*curr);    // delete the current node
            (*curr) = NULL;
            return true;       // delete the non-leaf parent nodes
        } else {// if the current node is a leaf node and has children
            // mark the current node as a non-leaf node (DON'T DELETE IT)
            (*curr)->isLeaf = 0;
            return false;       // don't delete its parent nodes
        }
    }
 
    return false;
}