#include "descendants.h"

Array_chars descendants;
int maxColor = 0;
bool hasDesc = false;

struct Trie* genTHead;

void initDescendants(){
  genTHead = getNewTrieNode();
  initArray(descendants, Array_char, 1024);
}

void freeDescendants(){
  hasDesc = false;
  maxColor = 0;
  for(int i = 0; i < descendants.used; i++){
      deletionTrie(&genTHead, descendants.data[i].data);
      freeArray(descendants.data[i]);
  }
  freeArray(descendants);
}

Array_char createDescendant(Array_char father, Array_char mother){
  hasDesc = true;
  int r;
  Array_char descendant;
  initArray(descendant, char, (mother.used-1)*2+1);
  for(int i = 0; i < mother.used-1; i++){
    insertArray(descendant, char, father.data[i]);
    insertArray(descendant, char, mother.data[i]);
  }
  r = insertTrie(genTHead, descendant, maxColor);
  insertArray(descendant, char, '\0');
  insertArray(descendants, Array_char, descendant);
  if(r > maxColor){
    maxColor = r;
  }
  return descendant;
}

int getColorIndex(Array_char descendant, bool phenotype){
  struct Trie* curr;
  if(phenotype){
    curr = searchTriePhenotype(genTHead, descendant);
  } else {
    curr = searchTrie(genTHead, descendant);
  }
  return curr->color;
}

int getMaxColor(){
  return maxColor;
}

Array_char getDescendant(int i){
  return descendants.data[i];
}

bool hasDescendants(){
  return hasDesc;
}