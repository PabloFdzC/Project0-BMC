#include "descendants.h"

Array_chars2 descendants;
int maxColor = 0;
int maxCant = 32;
bool hasDesc = false;

struct Trie* genTHead;

void initDescendants(int tot){
  genTHead = getNewTrieNode();
  if(tot > maxCant){
    maxCant = tot;
  }
  initArray(descendants, Array_char, maxCant);
}

void freeDescendants(){
  hasDesc = false;
  maxColor = 0;
  for(int i = 0; i < descendants.used; i++){
    for(int j = 0; j < descendants.data[i].used; j++){
      deletionTrie(&genTHead, descendants.data[i].data[j].data);
      freeArray(descendants.data[i].data[j]);
    }
    freeArray(descendants.data[i]);
  }
  freeArray(descendants);
}

Array_char createDescendant(Array_char father, Array_char mother, int row){
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
  Array_chars dRow;
  if(descendants.used == row){
    initArray(dRow, Array_char, maxCant);
    insertArray(dRow, Array_char, descendant);
    insertArray(descendants, Array_chars, dRow);
  } else {
    dRow = descendants.data[row];
    insertArray(dRow, Array_char, descendant);
    descendants.data[row] = dRow;
  }
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

Array_char getDescendant(int row, int col){
  Array_char error = {NULL, 0, 0};
  if(row >= descendants.used){
    return error;
  }
  if(col >= descendants.data[row].used){
    return error;
  }
  return descendants.data[row].data[col];
}

bool hasDescendants(){
  return hasDesc;
}