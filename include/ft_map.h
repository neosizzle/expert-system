#ifndef FT_MAP_H
# define FT_MAP_H
#include "rules.h"
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

FtMap *ft_map_new(int capacity);
int *query_map(FtMap *map, Symbol *symbol);
void insert_map(FtMap *map, Symbol *symbol, int *results, int result_len);
void free_ft_map(FtMap *map);
#endif