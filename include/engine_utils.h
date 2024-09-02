#ifndef __ENGINE_UTILS__H__
#define __ENGINE_UTILS__H__

# define MAX_VALUES 16
Symbol** get_inner_symbols(Symbol **list, int *indices);
void generate_truth_permutations(
	int **table,
	int num_elems,
	int *curr_table_y,
	int curr_aux_x,
	int *aux
);

Symbol** generate_mapping_for_truth_table(Symbol **list);
int unique_symbols(Symbol **list);

#endif  //!__ENGINE_UTILS__H__