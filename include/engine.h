#ifndef __ENGINE__H__
#define __ENGINE__H__

void locate_conditional_rule(Rulegraph *rg, char *input_symbol, Rule **res);
int solve_bool_pair(int lhs, Symbol* operator, int rhs);

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
#endif  //!__ENGINE__H__