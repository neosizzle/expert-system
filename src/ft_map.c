#include "ft_map.h"
#include "ft_macros.h"
#include <stdlib.h>

#define VALUE_LIMIT 16

FtMap *ft_map_new(int capacity)
{
  FtMap *res = (FtMap *)calloc(1, sizeof(FtMap));
  FtMapNode **nodes = (FtMapNode **)calloc(capacity, sizeof(FtMapNode *));
  res->capacity = capacity;
  res->len = 0;
  res->nodes = nodes;
  return res;
}

void free_ft_map_node(FtMapNode *node)
{
  if (node->results)
    free(node->results);
  if (node->symbol)
    free(node->symbol);

  free(node);
}

void free_ft_map(FtMap *map)
{
  for (int i = 0; i < map->len; ++i)
  {
    if (map->nodes[i])
      free_ft_map_node(map->nodes[i]);
  }
  free(map->nodes);
  free(map);
}

int query_map_idx(FtMap *map, Symbol *symbol)
{
  int *res = 0;
  char *key = symbol->str_repr;
  if (symbol->is_negated)
    key += 1;
  
  for (size_t i = 0; i < map->len; i++)
  {
    FtMapNode* pair = map->nodes[i];
    if (strcmp(key, pair->symbol))
      continue;
    
    return i;
  }
  return -1;
}

// insert map
void insert_map(FtMap *map, Symbol *symbol, int *results, int result_len)
{
  if (map->len >= map->capacity)
  {
    DIE(1, "[insert_map] No space")
  }
  int found_results = query_map_idx(map, symbol);
  int *new_values = (int *)malloc(VALUE_LIMIT * sizeof(int));
  memset(new_values, -1, VALUE_LIMIT);
  memcpy(new_values, results, result_len * sizeof(int));
  if (found_results != -1)
  {
    free(map->nodes[found_results]->results);
    map->nodes[found_results]->results = new_values;
  }
  else {
    FtMapNode *new_node = (FtMapNode *)calloc(1, sizeof(FtMapNode));
    int offset = 0;
    if (symbol->is_negated)
      ++offset;
    char *new_symbol = strdup(symbol->str_repr + offset);
    new_node->symbol = new_symbol;
    new_node->results = new_values;
    map->nodes[map->len] = new_node;
    map->len += 1;
  }

 
}


// query map that also matches negate symbols
int *query_map(FtMap *map, Symbol *symbol)
{
  int *res = 0;
  char *key = symbol->str_repr;
  if (symbol->is_negated)
    key += 1;
  
  for (size_t i = 0; i < map->len; i++)
  {
    FtMapNode* pair = map->nodes[i];
    if (strcmp(key, pair->symbol))
      continue;
    
    res = (int *)malloc(VALUE_LIMIT * sizeof(int));
    memcpy(res, pair->results, VALUE_LIMIT * sizeof(int));
  }
  return res;
}