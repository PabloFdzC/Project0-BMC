#include "global.h"
#include "trie.h"
#include "array.h"

void initDescendants();

void freeDescendants();

Array_char createDescendant(Array_char father, Array_char mother);

int getColorIndex(Array_char descendant, bool phenotype);

int getMaxColor();

Array_char getDescendant(int i);

bool hasDescendants();