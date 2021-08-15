#include "global.h"
#include "array.h"

typedef Array(char) Array_char;
typedef Array(Array_char) Array_chars;
typedef Array(int) Array_int;

Array_chars generateGenotypes(Array_char letters);

Array_int counter(Array_int count, int *first0);