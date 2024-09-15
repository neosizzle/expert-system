#include "rules.h"
#include "ft_macros.h"
#include "ft_map.h"
#include "engine_utils.h"

#include <string.h>
#include <stdlib.h>

int list_len_neg_1(int *list)
{
	int res = -1;
	while (list[++res] != -1){}
	return res;
}

void print_list_endl(int *list)
{
	int i = -1;
	printf("[");
	while (list[++i] != -1){
		printf("%d, ", list[i]);
	}
	printf("]\n");
}

void add_ignore_list(Rule **list, Rule *rule)
{
	int i = -1;
	while (list[++i]){}
	list[i] = rule;
}

void remove_ignore_list(Rule **list, Rule *rule, char* indent)
{
	char *rule_str = serialize_symbols(rule->symbol_list);
	DBG(indent, "[remove_ignore_list] removing %s\n", rule_str)
	int i = -1;
	while (list[++i])
	{
		if (list[i] == rule)
			break;
	}
	
	list[i] = 0;
	// NOTE: assume no double clear, list always have symbol_key
	while (list[i])
	{
		Rule *next_elem = list[i + i];
		list[i] = next_elem;
		++i;
	}
	
	free(rule_str);
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

// int *truths_perms = resolve_truth_permutations(
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
	
	int *res = (int *)malloc(MAX_VALUES * sizeof(int));
	memset(res, -1, MAX_VALUES * sizeof(int));
	int res_idx = -1;
	for (size_t table_idx = 0; table_idx < table_row_count; table_idx++)
	{
		int *table_row = table[table_idx];

		// generate values based on mapping
		memset(values, -1, MAX_VALUES * sizeof(int));
		int values_idx = -1;
		for (size_t i = 0; symbols[i]; i++)
		{
			Symbol *symbol = symbols[i];
			if (symbol->type != VARIABLE && symbol->type != FACT && symbol->type != INNER)
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
			DIE(1, "[resolve_truth_permutations] invalid number of operators vs values: values_idx %d, operator_idx + 1: %d", values_idx, operator_idx + 1);
		}

		// walk through the symbols and select them to generate the result for that combination
		Symbol **operators_copy = (Symbol **)calloc(MAX_VALUES, sizeof(Symbol *)); // shh.. hardallocate here
		memcpy(operators_copy, operators, MAX_VALUES * sizeof(Symbol *));
		res[++res_idx] = triple_take_premutation_resolve(values, operators_copy);
		free_symbol_list(operators_copy);
		// printf("res is %d\n", triple_take_premutation_resolve(values, operators_copy));
		
	}


	free(values);
	free(operators);
	return res;
}

Symbol** get_inner_symbols(Symbol **list, int *indices)
{
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

// self explanatory
Symbol** generate_mapping_for_truth_table(Symbol **list)
{
	Symbol** res =  (Symbol **)calloc(MAX_VALUES, sizeof(Symbol *)); // shh.. hard allocate here

	char **cache = (char **)calloc(MAX_VALUES, sizeof(char *)); // shh.. hard allocate here

	int res_idx = -1;
	// iterate the list of symbols
	for (size_t i = 0; list[i] ; i++)
	{
		if (list[i]->type != VARIABLE && list[i]->type != FACT && list[i]->type != INNER)
			continue;
		
		char *key = list[i]->str_repr;	
		int found_in_cache = 0;
		int cache_idx = -1;

		while (cache[++cache_idx])
		{
			// if the current cache item matches current list item, mark as found
			if (!strcmp(list[i]->str_repr, cache[cache_idx]))
			{
				++found_in_cache;
				break;
			}
		}

		// not found, add in cache, append to res and continue`
		if (!found_in_cache)
		{
			if (cache_idx >= MAX_VALUES)
			{
				// free_symbol(new_symbol);
				DIE(1, "[unique_symbols] cache idx overflow")
			}
			cache[cache_idx] = strdup(list[i]->str_repr);
			res[++res_idx] = list[i];
		}

	}
	
	// free cache
	for (size_t i = 0; i < MAX_VALUES; i++)
	{
		if (cache[i])
			free(cache[i]);
	}
	
	free(cache);
	return res;
}

// given a list of symbols, calculate the number of elemets needed for truth table
int unique_symbols(Symbol **list)
{
	int res = 0;

	char **cache = (char **)calloc(MAX_VALUES, sizeof(char *));

	// iterate the list of symbols
	for (size_t i = 0; list[i] ; i++)
	{
		if (list[i]->type != VARIABLE && list[i]->type != FACT && list[i]->type != INNER)
			continue;
		
		char *key = list[i]->str_repr;	
		int found_in_cache = 0;
		int cache_idx = -1;

		while (cache[++cache_idx])
		{
			// if the current cache item matches current list item, mark as found
			if (!strcmp(list[i]->str_repr, cache[cache_idx]))
			{
				++found_in_cache;
				break;
			}
		}

		// not found, add in cache, imcrement res and continue`
		if (!found_in_cache)
		{
			++res;
			if (cache_idx >= MAX_VALUES)
			{
				DIE(1, "[unique_symbols] cache idx overflow")
			}
			cache[cache_idx] = strdup(list[i]->str_repr);
		}

	}
	
	// free cache
	for (size_t i = 0; i < MAX_VALUES; i++)
	{
		if (cache[i])
			free(cache[i]);
	}
	
	free(cache);
	return res;
}

// #include "engine_utils.h"
// int res1[3] = {1, 0, -1};
// int res2[3] = {1, 0, -1};
// int res3[2] = {0, -1};
// int res4[2] = {0, -1};

// int *rhs_symbols_res[5] = {res1, res2, res3, res4, 0};
// int *indices_to_keep = filter_tt_for_resolve_for_symbol(
// 	table,
// 	rhs_symbols_res,
// 	symbols,
// 	mapping,
// 	perm_results,
// 	1,
// 	num_elems,
//	IFF
// );

// printf("map==========\n");
// for (size_t i = 0; i < total_rows; i++)
// {
// 	for (size_t j = 0; j < num_elems; j++)
// 		printf("%d ", table[i][j]);
// 	printf(" = %d\n", perm_results[i]);
// }

// for (size_t i = 0; indices_to_keep[i] != -1; i++)
// {
// 	printf("keeping idx %d\n", indices_to_keep[i]);
// }
// filters truth table for resolve_for_symbol, returns indices to keep
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
)
{
	int* res =  (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(res, -1, MAX_VALUES * sizeof(int));
	int curr_idx = -1;
	int res_idx = -1;

	while (perm_results[++curr_idx] != -1)
	{
		int curr_result = perm_results[curr_idx];
		int *curr_symbol_states = table[curr_idx];

		// filter truth table based to contain resolved == rule result
		// filter truth table based on implication rule
		if (resolve_type == IMPLIES)
		{
			if (lhs_res == 1 && curr_result == 0)
				continue;
		}
		else if (resolve_type == IFF)
		{
			if (curr_result != lhs_res)
				continue;
		}
		else {
			DIE(1, "[filter_tt_for_resolve_for_symbol] No resolve type here apparently :/\n")
		}
		
		// filter truth table based to contain saved symbol results
		// col by col
		int *table_row = table[curr_idx];
		int skip_flag = 0;
		// iterate column of table
		for (size_t curr_col = 0; curr_col < num_elems; curr_col++)
		{
			// get mapping entry
			Symbol *to_map = mapping[curr_col];

			// find index of symbol in rhs_symbols
			int actual_symbol_idx = -1;
			int actual_symbol_idx_offset = 0;
			while (rhs_symbols[++actual_symbol_idx])
			{
				Symbol* symbol = rhs_symbols[actual_symbol_idx];

				if (symbol->type != VARIABLE && symbol->type != INNER && symbol->type != FACT)
					++actual_symbol_idx_offset;
				if (symbol->type == INNER && to_map->type == INNER)
				{
					char *list_serialized = serialize_symbols(symbol->inner_symbols);
					char *to_map_serialized = serialize_symbols(to_map->inner_symbols);
					if (!strcmp(list_serialized, to_map_serialized))
					{
						free(list_serialized);
						free(to_map_serialized);
						break;
					}
				}
				if (!strcmp(symbol->str_repr, to_map->str_repr))
					break;
			}
			
			// after get index, obtain result list of that symbol
			// Note: assumes actual_symbol_idx is always correct, else we will get -1 or 0 here
			int *actual_symbol_results = rhs_symbols_res[actual_symbol_idx]; // dont need offset or else read null memory?

			// check if column value is in result list and initialize found flag
			int found_flag = 0;
			int res_to_check = table_row[curr_col];
			for (size_t i = 0; actual_symbol_results[i] != -1; i++)
			{
				if (actual_symbol_results[i] == res_to_check)
					++found_flag;
			}
			
			// if no found_flag, set skip flag and break
			if (!found_flag)
			{
				// only append skip flag if we dont want rule enforce to override decision
				if (rule_enforce)
				{
					continue;
				}
				++skip_flag;
				break;
			}
		}

		// check skip flag. If yes, continue
		if (skip_flag)
			continue;
		
		// if no, add curr_idx to res[++res_idx];
		res[++res_idx] = curr_idx;
	}
	
	return res;
}

int *filter_tt_for_resolve_for_inner(
	int **table,
	int **rhs_symbols_res,
	Symbol **rhs_symbols,
	Symbol **mapping,
	int *perm_results,
	int num_elems
)
{
	int* res =  (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(res, -1, MAX_VALUES * sizeof(int));
	int curr_idx = -1;
	int res_idx = -1;

	while (perm_results[++curr_idx] != -1)
	{
		int curr_result = perm_results[curr_idx];
		int *curr_symbol_states = table[curr_idx];
		
		// filter truth table based to contain saved symbol results
		// col by col
		int *table_row = table[curr_idx];
		int skip_flag = 0;
		// iterate column of table
		for (size_t curr_col = 0; curr_col < num_elems; curr_col++)
		{
			// get mapping entry
			Symbol *to_map = mapping[curr_col];

			// find index of symbol in rhs_symbols
			int actual_symbol_idx = -1;
			int actual_symbol_idx_offset = 0;
			while (rhs_symbols[++actual_symbol_idx])
			{
				Symbol* symbol = rhs_symbols[actual_symbol_idx];

				if (symbol->type != VARIABLE && symbol->type != INNER && symbol->type != FACT)
					++actual_symbol_idx_offset;
				if (!strcmp(symbol->str_repr, to_map->str_repr))
					break;
			}
			
			// after get index, obtain result list of that symbol
			int *actual_symbol_results = rhs_symbols_res[actual_symbol_idx]; // dont need offset or else read null memory?
			// for (size_t i = 0; i < MAX_VALUES && actual_symbol_results[i] != -1; i++)
			// {
			// 	printf("[fttforsymbol] [%d] %d\n", i, actual_symbol_results[i]);
			// }

			// check if column value is in result list and initialize found flag
			int found_flag = 0;
			int res_to_check = table_row[curr_col];
			for (size_t i = 0; actual_symbol_results[i] != -1; i++)
			{
				if (actual_symbol_results[i] == res_to_check)
					++found_flag;
			}
			
			// if no found_flag, set skip flag and break
			if (!found_flag)
			{
				++skip_flag;
				break;
			}
		}

		// check skip flag. If yes, continue
		if (skip_flag)
			continue;
		
		// if no, add curr_idx to res[++res_idx];
		res[++res_idx] = curr_idx;
	}
	
	return res;
}

int *filter_tt_for_resolve_for_rule(
	int **table,
	int **rhs_symbols_res,
	Symbol **rhs_symbols,
	Symbol **mapping,
	int *perm_results,
	int num_elems
)
{
	int* res =  (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(res, -1, MAX_VALUES * sizeof(int));
	int curr_idx = -1;
	int res_idx = -1;

	while (perm_results[++curr_idx] != -1)
	{
		int curr_result = perm_results[curr_idx];
		int *curr_symbol_states = table[curr_idx];
		
		// filter truth table based to contain saved symbol results
		// col by col
		int *table_row = table[curr_idx];
		int skip_flag = 0;
		// iterate column of table
		for (size_t curr_col = 0; curr_col < num_elems; curr_col++)
		{
			// get mapping entry
			Symbol *to_map = mapping[curr_col];

			// printf("[filter_tt_for_resolve_for_rule] to_map %s\n", to_map->str_repr);

			// find index of symbol in rhs_symbols
			int actual_symbol_idx = -1;
			int actual_symbol_idx_offset = 0;
			while (rhs_symbols[++actual_symbol_idx])
			{
				Symbol* symbol = rhs_symbols[actual_symbol_idx];

				// printf("[filter_tt_for_resolve_for_rule] symbol->str_repr %s\n", symbol->str_repr);

				if (symbol->type != VARIABLE && symbol->type != INNER && symbol->type != FACT)
					++actual_symbol_idx_offset;
				if (symbol->type == INNER && to_map->type == INNER)
				{
					char *list_serialized = serialize_symbols(symbol->inner_symbols);
					char *to_map_serialized = serialize_symbols(to_map->inner_symbols);
					if (!strcmp(list_serialized, to_map_serialized))
					{
						free(list_serialized);
						free(to_map_serialized);
						break;
					}
				}
				if (!strcmp(symbol->str_repr, to_map->str_repr))
					break;
			}
			
			// after get index, obtain result list of that symbol
			// Note: assumes actual_symbol_idx is always correct, else we will get -1 or 0 here
			int *actual_symbol_results = rhs_symbols_res[actual_symbol_idx];
			// printf("[filter_tt_for_resolve_for_rule] actual_symbol_results %p, idx %d\n", actual_symbol_results, actual_symbol_idx - actual_symbol_idx_offset);

			// check if column value is in result list and initialize found flag
			int found_flag = 0;
			int res_to_check = table_row[curr_col];
			for (size_t i = 0; actual_symbol_results[i] != -1; i++)
			{
				if (actual_symbol_results[i] == res_to_check)
					++found_flag;
			}
			
			// if no found_flag, set skip flag and break
			if (!found_flag)
			{
				++skip_flag;
				break;
			}
		}

		// check skip flag. If yes, continue
		if (skip_flag)
			continue;
		
		// if no, add curr_idx to res[++res_idx];
		res[++res_idx] = curr_idx;
	}
	
	return res;
}

// Apply filters and free anything that got discarded
void apply_filters(
	int **table,
	int *perm_results,
	int *indices_to_keep,
	int number_of_rows
)
{
	// create new table and perm results
	int **new_table = (int **)calloc(MAX_VALUES, sizeof(int *));
	int *new_perm_results = (int *)malloc(MAX_VALUES * sizeof(int));
	memset(new_perm_results, -1, MAX_VALUES * sizeof(int));
	int perm_result_idx = -1;
	int table_idx = -1;
	
	// iterate through indices to keep and copy original kept data into tabes but free removed items in table
	for (size_t i = 0; indices_to_keep[i] != -1; i++)
	{
		int *table_row = table[indices_to_keep[i]];
		int perm_result = perm_results[indices_to_keep[i]];
		new_perm_results[++perm_result_idx] = perm_result;
		new_table[++table_idx] = table_row;
	}

	// apply filter to table
	for (size_t i = 0; i < number_of_rows; i++)
	{
		int found_in_keep = 0;
		for (size_t j = 0; indices_to_keep[j] != -1; j++)
		{
			if (i == indices_to_keep[j])
				found_in_keep = 1;
		}
		if (!found_in_keep)
			free(table[i]);
		table[i] = 0;
	}
	for (size_t i = 0; new_table[i]; i++)
		table[i] = new_table[i];
	
	// apply filter to perm results
	for (size_t i = 0; i < number_of_rows; i++)
		perm_results[i] = -1;
	for (size_t i = 0; new_perm_results[i] != -1; i++)
		perm_results[i] = new_perm_results[i];
	free(new_table);
	free(new_perm_results);
}

// function that record mapping symbols to cache
// will only overwrite cache entry if cache value is longer than new value
// does negation converison here
void store_results_in_cache(
	Symbol **mapping,
	int **table,
	FtMap* cache
) {
	int aux[MAX_VALUES]; // hard allocate
	memset(aux, -1, sizeof(int) * MAX_VALUES);

	for (size_t symbol_idx = 0; mapping[symbol_idx]; symbol_idx++)
	{
		Symbol *symbol_to_cache = mapping[symbol_idx];
		for (size_t table_idx = 0; table[table_idx]; table_idx++)
		{
			int *results_to_cache = table[table_idx];
			aux[table_idx] = results_to_cache[symbol_idx];
		}

		// our aux should be filled with values for current symbol. Do negation thing
		if (symbol_to_cache->is_negated)
		{
			for (size_t i = 0; aux[i] != -1; i++)
				aux[i] = !aux[i];
		}

		// get cache value to compare length
		int *cache_found = query_map(cache, symbol_to_cache);
		if (cache_found)
		{
			// printf("cache_found[0] %d\n", cache_found[5]);
			int cache_len = list_len_neg_1(cache_found);
			int my_len = list_len_neg_1(aux);
			if (my_len < cache_len)
				insert_map(cache, symbol_to_cache, aux, my_len);
			free(cache_found);
		}
		else
		{
			// printf("new entry to cahce %s \n", symbol_to_cache->str_repr);
			// for (size_t i = 0; i < MAX_VALUES; i++)
			// {
			// 	printf("new aux[%d] %d\n", i, aux[i]);
			// }
			
			insert_map(cache, symbol_to_cache, aux, MAX_VALUES); // NOTE: hardcoded length here
		}
		// reset aux
		memset(aux, -1, MAX_VALUES * sizeof(int));
	}
	
}

void res_deduper(int *permutation_results)
{
	// return either true, false, or true / false
	int first_res = permutation_results[0];
	int snd_res = -1;
	for (size_t i = 0; permutation_results[i] != -1; i++)
	{
		if (permutation_results[i] != first_res)
		{
			snd_res = permutation_results[i];
			break;
		}
	}
	memset(permutation_results, -1, MAX_VALUES * sizeof(int));
	permutation_results[0] = first_res;
	permutation_results[1] = snd_res;
}