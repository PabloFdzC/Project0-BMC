#define Array(TYPE) struct { TYPE* data; int size; int used; }
#define initArray(A, TYPE, I) A.data = malloc(I * sizeof(TYPE)); A.used = 0; A.size = I
#define insertArray(A, TYPE, E) if (A.used == A.size) { A.size *= 2; A.data = realloc(A.data, A.size * sizeof(TYPE)); } A.data[A.used++] = E
#define freeArray(A) free(A.data); A.data = NULL; A.used = A.size = 0
#define freeArrayP(A) for(int i = 0; i < A.used; i++){ freeArray(A.data[i]);} freeArray(A)
#define deleteAllArray(A) A.used = 0

#ifndef ARRAY_TYPES
#define ARRAY_TYPES

typedef Array(char) Array_char;
typedef Array(Array_char) Array_chars;
typedef Array(Array_chars) Array_chars2;
typedef Array(int) Array_int;

#endif
/**
 * This code was inspired by the question in: 
 * https://stackoverflow.com/questions/42293192/making-a-dynamic-array-that-accepts-any-type-in-c
 */