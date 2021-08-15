#include "genotypes.h"
#include <math.h>
#include <ctype.h>

Array_chars generateGenotypes(Array_char letters){
  int totComb = (int)pow(3.0, (double)letters.used);
  int totLet = letters.used * 2 + 1;
  int even, first0 = 0;
  Array_chars strList;
  initArray(strList, Array_char, totComb);
  Array_int count;
  initArray(count, Array_int, letters.used * 2);
  for(int i = 0; i < count.size; i++){
    insertArray(count, int, 0);
  }
  for(int i = 0; i < totComb; i++){
    Array_char genotype;
    initArray(genotype, char, totLet);
    for(int j = 0; j < totLet-1; j++){
      even = j % 2 == 0;
      if(count.data[j]){
        if(even){
          insertArray(genotype, char, tolower(letters.data[j/2]));
        } else {
          insertArray(genotype, char, tolower(letters.data[(j-1)/2]));
        }
      } else {
        if(even){
          insertArray(genotype, char, letters.data[j/2]);
        } else {
          insertArray(genotype, char, letters.data[(j-1)/2]);
        }
      }
    }
    insertArray(genotype, char, '\0');
    counter(count, &first0);
    for(int i = 0; i < count.used; i++){
      printf("%d", count.data[i]);
    }
    printf("\n");
    insertArray(strList, Array_char, genotype);
  }

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