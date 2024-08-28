#ifndef FT_MAP_H
# define FT_MAP_H

typedef struct FtMapNode {
  char* symbol;
  int* results;
} FtMapNode;

// heap allocated nodes with fixed capacity 
typedef struct FtMap {
  FtMapNode** nodes;
  int capacity;
  int len;
} FtMap;

#endif