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
	int num_elems,
	ResolveType resolve_type,
	int rule_enforce
);

int *filter_tt_for_resolve_for_rule(
	int **table,
	int **rhs_symbols_res,
	Symbol **rhs_symbols,
	Symbol **mapping,
	int *perm_results,
	int num_elems
);

int *filter_tt_for_resolve_for_inner(
	int **table,
	int **rhs_symbols_res,
	Symbol **rhs_symbols,
	Symbol **mapping,
	int *perm_results,
	int num_elems

);

void apply_filters(
	int **table,
	int *perm_results,
	int *indices_to_keep,
	int number_of_rows
);

void store_results_in_cache(
	Symbol **mapping,
	int **table,
	FtMap* cache
);

int list_len_neg_1(int *list);
void print_list_endl(int *list);
void add_ignore_list(Rule **list, Rule *rule);
void remove_ignore_list(Rule **list, Rule *rule, char* indent);
void res_deduper(int *permutation_results);

#endif  //!__ENGINE_UTILS__H__