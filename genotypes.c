#include "genotypes.h"
#include <math.h>
#include <ctype.h>

Array_chars generateGenotypes(Array_char letters){
  int totComb = (int)pow(3.0, (double)letters.used);
  int totLet = letters.used * 2 + 1;
  int even, first0 = 0;
  Array_chars strList;
  initArrayP(strList, Array_char, totComb);
  Array_int count;
  initArray(count, int, letters.used * 2);
  for(int i = 0; i < count.size; i++){
    insertArray(count, int, 0);
  }
  for(int i = 0; i < totComb; i++){
    Array_char genotype;
    initArray(genotype, char, totLet);
    for(int j = totLet-2; j >= 0; j--){
      even = j % 2 == 0;
      if(count.data[j]){
        if(even){
          insertArray(genotype, char, tolower(letters.data[letters.used-1-j/2]));
        } else {
          insertArray(genotype, char, tolower(letters.data[letters.used-1-(j-1)/2]));
        }
      } else {
        if(even){
          insertArray(genotype, char, letters.data[letters.used-1-j/2]);
        } else {
          insertArray(genotype, char, letters.data[letters.used-1-(j-1)/2]);
        }
      }
    }
    insertArray(genotype, char, '\0');
    counter(count, &first0);
    insertArray(strList, Array_char, genotype);
  }
  freeArray(count);
  return strList;
}

Array_int counter(Array_int count, int *first0){
  if(count.data[(*first0)] == 0){
    count.data[(*first0)] = 1;
    if((*first0) > 1 && (*first0) % 2 == 0){
      for(int i = 0; i < (*first0); i++){
        count.data[i] = 0;
      }
      (*first0) = 0;
    } else {
      (*first0)++;
    }
  } else {
    if(count.data[(*first0)+1] == 0){
      count.data[(*first0)+1] = 1;
      for(int i = 0; i < (*first0); i++){
        count.data[i] = 0;
      }
    } else {
      while(count.data[(*first0)] != 0){
        (*first0)++;
        if((*first0) == count.size){
          break;
        }
      }
      for(int i = 0; i < (*first0); i++){
        count.data[i] = 0;
      }
      if((*first0) < count.size){
        count.data[(*first0)] = 1;
        if((*first0) % 2 != 0){
          count.data[(*first0)-1] = 1;
        }
      }
    }
    (*first0) = 0;
  }
  return count;
}