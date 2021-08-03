#include "global.h"

typedef Array(char) Array_char; // Necesario para definir el tipo

Array_char dynamicString;

int main() {
  initArray(dynamicString, char, 10); // Inicializa el arreglo, solo se hace una vez, 10 es el tamaño del arreglo
  insertArray(dynamicString, char, 'a'); // Inserta una letra en el arreglo
  freeArray(dynamicString); // Elimina el arreglo por completo
  return 0;
}