#include "global.h"
#include "trie.h"
#include "array.h"

void initDescendants(int tot);

void freeDescendants();

Array_char createDescendant(Array_char father, Array_char mother, int row);

int getColorIndex(Array_char descendant, bool phenotype, int* totDes);

int getMaxColor();

Array_char getDescendant(int row, int col);

bool hasDescendants();