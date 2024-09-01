#include "rules.h"
#include "ft_macros.h"
#include "ft_map.h"
#include <string.h>
#include <stdlib.h>

# define MAX_VALUES 16

// given a symbol, look for rules which implies / iffs that symbol and writes to res
void locate_conditional_rule(Rulegraph *rg, char *input_symbol, Rule **res)
{
	Rule **rules = rg->all_rules_vertices;

	int i = -1;
	int j = -1;
	while (rules[++i])
	{
		Rule *rule = rules[i];
		Rule *rule_cmp = NULL;
		if (rules[i]->implies)
			rule_cmp = rule->implies;
		else
			rule_cmp = rule->iff;

		if (rule_cmp)
		{
			char *symbols_str = serialize_symbols(rule_cmp->symbol_list);
			if (strcspn(symbols_str, input_symbol) < strlen(symbols_str))
				res[++j] = rule;

			free(symbols_str);
		}
	}
}

// solve a boolean equation value Operator value
// expects both symbols to have define values
int solve_bool_pair(int lhs, Symbol* operator, int rhs)
{
	// validations
	// if (lhs->type != VARIABLE && lhs->type != FACT)
	// {
	// 	DIE(1, "[solve_bool_pair] lhs symbol is a non variable or fact type\n");
	// }

	// if ((!lhs->possible_results) || (lhs->possible_results[0] != FT_TRUE && lhs->possible_results[0] != FT_FALSE))
	// {
	// 	DIE(1, "[solve_bool_pair] lhs symbol is unresolved \n");
	// }

	// if (rhs->type != VARIABLE && rhs->type != FACT)
	// {
	// 	DIE(1, "[solve_bool_pair] rhs symbol is a non variable or fact type\n");
	// }

	// if ((!rhs->possible_results) || (rhs->possible_results[0] != FT_TRUE && rhs->possible_results[0] != FT_FALSE))
	// {
	// 	DIE(1, "[solve_bool_pair] rhs symbol is unresolved \n");
	// }

	if (operator->type != OPERATOR || operator->operator == NOP)
	{
		DIE(1, "[solve_bool_pair] operator is invalid\n");
	}

	// int lhs_res = (lhs->possible_results[0] == FT_TRUE)? 1 : 0;
	// int rhs_res = (rhs->possible_results[0] == FT_TRUE)? 1 : 0;

	if (operator->operator == AND)
		return lhs & rhs;
	if (operator->operator == OR)
		return lhs | rhs;
	if (operator->operator == XOR)
		return lhs ^ rhs;
}

// populate an unresolved truth table given number of unique variables
// NOTE: C and !C should be considered different here
void generate_truth_permutations(
	int **table,
	int num_elems,
	int *curr_table_y,
	int curr_aux_x,
	int *aux
)
{
	for (size_t curr_bit = 0; curr_bit < 2; curr_bit++)
	{
		// write current bit to auxillary
		aux[curr_aux_x] = curr_bit;

		// base case, our x is filled up to num_elems
		if (curr_aux_x == num_elems - 1)
		{
			// copy the aux buffer into tables[curr_table_y]
			for (size_t i = 0; i < num_elems; i++)
				table[*curr_table_y][i] = aux[i];
			
			// increment curr_table_y after 1 copy
			*curr_table_y = *curr_table_y + 1;

			continue;
		}

		// we recurse
		generate_truth_permutations(
			table,
			num_elems,
			curr_table_y,
			curr_aux_x + 1,
			aux
		);
	}
}

// helper that runs resolution 3 times, first for AND, then OR, then XOR in order
// this will modify int *values and Symbol** operators
static int triple_take_premutation_resolve(
	int *values,
	Symbol **operators
)
{
	Operator operator_prio[3] = {AND, OR, XOR};
	for (size_t i = 0; i < 3; i++)
	{
		Operator operator_to_run = operator_prio[i];
		int window_idx = 0;
		int operator_idx = -1;
		
		while (operators[++operator_idx])
		{
			Symbol *operator = operators[operator_idx];
			int lhs = values[window_idx];
			int rhs = values[++window_idx];

			if (operator->operator != operator_to_run)
				continue;
			
			if (rhs == -1)
			{
				DIE(1, "[triple_take_premutation_resolve] First run invalid RHS\n");
			}

			int res = solve_bool_pair(lhs, operator, rhs);

			// shift values
			values[window_idx] = res;
			while (values[window_idx] != -1)
			{
				values[window_idx - 1] = values[window_idx];
				++window_idx;
			}
			values[window_idx - 1] = -1;

			// shift operators
			while (operators[operator_idx])
			{
				operators[operator_idx] = operators[operator_idx + 1];
				++operator_idx;
			}

			window_idx = 0;
			operator_idx = -1;

		}
	}

	return values[0];
}

// int num_elems = 3;
// int total_rows = pow(2, num_elems);
// int **table = (int **)calloc(total_rows, sizeof(int *));
// for (size_t i = 0; i < total_rows; i++)
// 	table[i] = (int *)calloc(num_elems, sizeof(int));
// int* aux = (int *)calloc(num_elems, sizeof(int));
// int curr_table_y = 0;

// generate_truth_permutations(
// 	table,
// 	num_elems,
// 	&curr_table_y,
// 	0,
// 	aux
// );

// Symbol *s1 = generate_symbol_from("A", 0, 0);
// Symbol *s2 = generate_symbol_from("+", 0, 0);
// Symbol *s3 = generate_symbol_from("B", 0, 0);
// Symbol *s4 = generate_symbol_from("|", 0, 0);
// Symbol *s5 = generate_symbol_from("C", 0, 0);
// Symbol *s6 = generate_symbol_from("+", 0, 0);
// Symbol *s7 = generate_symbol_from("C", 0, 0);

// Symbol *mapping[4] = {s1, s3, s5, 0};
// Symbol *symbols[8] = {s1, s2, s3, s4, s5, s6, s7, 0};

// resolve_truth_permutations(
// 	mapping,
// 	table,
// 	curr_table_y,
// 	symbols
// );

// printf("map==========\n");
// for (size_t i = 0; i < total_rows; i++)
// {
// 	for (size_t j = 0; j < num_elems; j++)
// 		printf("%d ", table[i][j]);
// 	printf("\n");
// }

// resolve the truth permutations generated by generate_truth_permutations and filtered. It takes
// a mapping because the symbols we are resolving may contain duplicates that should map to the same value
// assume validation is already done, we are passing in valid symbols
// if we have 4 symbols, we should expect 4 elements in each row of the table
// the return value of this function can be used for further filtering
// NOTE: C and !C should be considered different here
int *resolve_truth_permutations(
	Symbol** mapping,
	int **table,
	int table_row_count,
	Symbol **symbols
)
{
	// generate array of values to be resolved
	int *values = (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hardallocate here
	memset(values, -1, MAX_VALUES * sizeof(int));

	// generate array of operators for the values
	Symbol **operators = (Symbol **)calloc(MAX_VALUES, sizeof(Symbol *)); // shh.. hardallocate here
	int operator_idx = -1;
	for (size_t i = 0; symbols[i]; i++)
	{
		Symbol *symbol = symbols[i];

		if (symbol->type != OPERATOR)
			continue;
		
		operators[++operator_idx] = symbol;
	}
	
	int *res = (int *)calloc(table_row_count, sizeof(int));

	for (size_t table_idx = 0; table_idx < table_row_count; table_idx++)
	{
		int *table_row = table[table_idx];

		// generate values based on mapping
		memset(values, -1, MAX_VALUES * sizeof(int));
		int values_idx = -1;
		for (size_t i = 0; symbols[i]; i++)
		{
			Symbol *symbol = symbols[i];
			if (symbol->type != VARIABLE && symbol->type != FACT)
				continue;

			// see if symbol appear in mapping list
			int symbol_mapping_idx = -1;
			for (size_t i = 0; mapping[i]; i++)
			{
				Symbol *symbol_map = mapping[i];

				if (strstr(symbol_map->str_repr, symbol->str_repr))
				{
					symbol_mapping_idx = i;
					break;
				}
			}
			if (symbol_mapping_idx == -1)
			{
				DIE(1, "[resolve_truth_permutations] symbol not found in mapping");
			}
			values[++values_idx] = table[table_idx][symbol_mapping_idx];
		}

		if (values_idx != operator_idx + 1)
		{
			DIE(1, "[resolve_truth_permutations] invalid number of operators vs values");
		}

		// walk through the symbols and select them to generate the result for that combination
		Symbol **operators_copy = (Symbol **)calloc(MAX_VALUES, sizeof(Symbol *)); // shh.. hardallocate here
		memcpy(operators_copy, operators, MAX_VALUES * sizeof(Symbol *));
		printf("res is %d\n", triple_take_premutation_resolve(values, operators_copy));
		
	}


	free(values);
	free(operators);
	return res;
}

int *resolve_for_symbol(Rulegraph *rg, Symbol *symbol, char* facts, FtMap* cache)
{
	Rule **rules_to_resolve = (Rule **)calloc(MAX_VALUES, sizeof(Rule *)); // shh.. hard allocate here
	int *res = (int*)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(res, -1, MAX_VALUES);

	// check facts first
	char *key = symbol->str_repr;
	if (symbol->is_negated)
 	   key += 1;
	if (strstr(facts, symbol_str))
	{
		printf("%s is found in facts\n", symbol_str);
		res[0] = 1;
	}

	// check cache. If found, return here
	int *cache_found = query_map(map, symbol);
	if (cache_found)
		return cache_found;

	// search for all rules where rhs has symbol
	locate_conditional_rule(rg, symbol_str, rules_to_resolve);

	// iterate through all results
	for (size_t i = 0; rules_to_resolve[i]; i++)
	{
		Rule *rule_to_resolve = rules_to_resolve[i];

		// resolve that rule
		int *curr_rule_res = resolve_for_rule(rule_to_resolve);
		
		// iterate through all possible results for that rule
		int j = -1;
		while (curr_rule_res[++j] != -1)
		{
			int curr_lhs_res = curr_rule_res[j];
			int **symbols_res = (int**)calloc(MAX_VALUES, sizeof(int*)); // shh.. hard allocate here
			int *inner_symbols_indices = (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
			memset(inner_symbols_indices, -1, MAX_VALUES)
			int inner_symbols_indices_idx = -1;

			Symbol **rhs_symbols = 0;
			if (rule_to_resolve->resolve_type == IMPLIES)
				rhs_symbols = rule_to_resolve->implies->symbol_list;
			else
				rhs_symbols = rule_to_resolve->iff->symbol_list;
			
			// check if any inner symbols in rhs. If yes, handle them first
			Symbol **inner_symbols = get_inner_symbols(rhs_symbols, inner_symbols_indices);
			for (size_t k = 0; inner_symbols[k]; k++)
			{
				Symbol *curr_symbol_list = inner_symbols[k];
				int *inner_res = resolve_for_inner(rg, curr_symbol_list, cache);
				int idx_for_inner_symbol = inner_symbols_indices[k];

				symbols_res[idx_for_inner_symbol] = inner_res;
			}
			

			// iterate through rhs symbols
				// call resolve_for_symbol() for current symbol and save them in symbols_res

				// if current symbol is negated, apply negation on the results

				// generate & resolve truth permutations


			free(symbols_res);
		}
		


		
		printf("Rule found to resolving %s, %s\n", symbol_str, serialize_symbols(rule_to_resolve->symbol_list));
	}
	
	free(rules_to_resolve);
	return res;
}

int *resolve_for_rule(Rulegraph *rg, Rule* rule, FtMap* cache)
{
	return 0;
}

int *resolve_for_inner(Rulegraph *rg, Symbol* inner_symbols, FtMap *cache)
{
	return 0;
}

Symbol* get_inner_symbols(Symbol **list, int *indices) {
	Symbol **res = (Symbol **)calloc(MAX_VALUES, sizeof(Symbol *));
	int j = -1;
	int k = -1;
	for (size_t i = 0; list[i]; i++)
	{
		if (list[i]->type == INNER)
		{
			res[++j] = list[i];
			indices[++k] = i;
		}
	}
	

	return res;
}