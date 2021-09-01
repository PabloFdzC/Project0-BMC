#include "quicksort.h"

// Quicksort taken from:
// http://codigoprogramacion.com/cursos/tutoriales-c/quicksort-en-c-algoritmo-de-ordenamiento.html#.YSWXu1u71uQ

void qs(Array_int list, Array_int list2, int leftL, int rightL){
  int left;
  int right;
  int temp;
  int temp2;
  int pivot;

  left=leftL;
  right = rightL;
  pivot = list.data[(left+right)/2];
  do{
    while(list.data[left] > pivot && left < rightL)left++;
    while(pivot > list.data[right] && right > leftL)right--;

    if(left <= right){ // tal vez este
      temp = list.data[left];
      temp2 = list2.data[left];
      list.data[left] = list.data[right];
      list2.data[left] = list2.data[right];
      list.data[right] = temp;
      list2.data[right] = temp2;
      left++;
      right--;
    }

  }while(left <= right);
  if(leftL < right){
    qs(list, list2, leftL, right);
  }
  if(rightL > left){
    qs(list, list2, left, rightL);
  }
}

void quicksort(Array_int list, Array_int list2){
  qs(list, list2, 0, list.used-1);
}