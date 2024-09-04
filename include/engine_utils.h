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
int *resolve_truth_permutations(
	Symbol** mapping,
	int **table,
	int table_row_count,
	Symbol **symbols
);
Symbol** generate_mapping_for_truth_table(Symbol **list);
int unique_symbols(Symbol **list);
int *filter_tt_for_resolve_for_symbol(
	int **table,
	int **rhs_symbols_res,
	Symbol **rhs_symbols,
	Symbol **mapping,
	int *perm_results,
	int lhs_res,
	int num_elems
);

void apply_filters(
	int **table,
	int *perm_results,
	int *indices_to_keep,
	int number_of_rows
);
#endif  //!__ENGINE_UTILS__H__