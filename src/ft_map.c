#include "ft_map.h"

FtMap* ft_map_new(int capacity){
  FtMap *res = (FtMap*)calloc(1, sizeof(FtMap));
  FtMapNode **nodes = (FtMapNode*)calloc(capacity, sizeof(FtMapNode*));
  res->capacity = capacity;
  res->len = 0;
  res->nodes = nodes;
  return res;
}

void free_ft_map_node(FtMapNode *node)
{
  // dont need to free symbol, stack allocated 
  
  if (node->results){
    free(node->results);
  }
  
  free(node);
}

void free_ft_map(FtMap *map){
  for(int i = 0; i <  map->len; ++i){
    if(map->nodes[i])
      free_ft_map_node(map->nodes[i])
  }
  free(map->nodes);
  free(map);
}