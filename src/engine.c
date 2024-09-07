#include "rules.h"
#include "ft_macros.h"
#include "ft_map.h"
#include "engine_utils.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

int *resolve_for_rule(Rulegraph *rg, Rule* rule, char* facts, FtMap* cache);
Symbol** get_inner_symbols(Symbol **list, int *indices);
int *resolve_for_inner(Rulegraph *rg, Symbol* inner_symbols, FtMap *cache);

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

int *resolve_for_symbol(Rulegraph *rg, Symbol *symbol, char* facts, FtMap* cache)
{
	printf("[resolve_for_symbol] Entry for %s\n", symbol->str_repr);

	Rule **rules_to_resolve = (Rule **)calloc(MAX_VALUES, sizeof(Rule *)); // shh.. hard allocate here
	int *res = (int*)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(res, -1, MAX_VALUES);

	// check facts first
	if (symbol->type == FACT)
	{
		free(rules_to_resolve);
		printf("[resolve_for_symbol] %s is found in facts, is_negated? %d\n", symbol->str_repr, symbol->is_negated);
		res[0] = 1;
		if (symbol->is_negated)
			res[0] = 0;
		return res;
	}
	// char *symbol_key = symbol->str_repr;
	// if (symbol->is_negated)
 	//    symbol_key += 1;
	// if (strstr(facts, symbol_key))
	// {
	// 	printf("%s is found in facts\n", symbol_key);
	// 	if (symbol->is_negated)
	// 		res[0] = 0;
	// 	res[0] = 1;
	// 	// return res;
	// }

	char *symbol_key = symbol->str_repr;
	if (symbol->is_negated)
		symbol_key += 1;

	// check cache. If found, return here
	int *cache_found = query_map(cache, symbol);
	if (cache_found)
	{
		printf("[resolve_for_symbol] %s found in cache: ", symbol->str_repr);
		print_list_endl(cache_found);
		free(rules_to_resolve);
		free(res);
		return cache_found;
	}

	// search for all rules where rhs has symbol
	locate_conditional_rule(rg, symbol_key, rules_to_resolve);

	// iterate through all results
	for (size_t i = 0; rules_to_resolve[i]; i++)
	{
		Rule *rule_to_resolve = rules_to_resolve[i];
		char *rule_str = serialize_symbols(rule_to_resolve->symbol_list);

		printf("[resolve_for_symbol] resolving rule %s\n", rule_str);
		// resolve that rule
		int *curr_rule_res = resolve_for_rule(rg, rule_to_resolve, facts, cache);
		printf("[resolve_for_symbol] rule %s resolved to: ", rule_str);
		print_list_endl(curr_rule_res);


		// iterate through all possible results for that rule
		int j = -1;
		while (curr_rule_res[++j] != -1)
		{

			int curr_lhs_res = curr_rule_res[j];
			int **rhs_symbols_res = (int**)calloc(MAX_VALUES, sizeof(int*)); // shh.. hard allocate here
			int *inner_symbols_indices = (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
			memset(inner_symbols_indices, -1, MAX_VALUES);
			int inner_symbols_indices_idx = -1;
			
			printf("[resolve_for_symbol] if rule %s is %d: ", rule_str, curr_lhs_res);

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

				rhs_symbols_res[idx_for_inner_symbol] = inner_res;
			}
			
			// iterate through rhs symbols
			for (size_t k = 0; rhs_symbols[k]; k++)
			{
				Symbol *curr_rhs_symbol = rhs_symbols[k];
				
				if (curr_rhs_symbol->type == INNER)
					continue;
				
				// call resolve_for_symbol() for current symbol and save them in rhs_symbols_res
				printf("[resolve_for_symbol] resolving RHS symbol %s: ", curr_rhs_symbol->str_repr);
				int *curr_symbol_res = resolve_for_symbol(rg, curr_rhs_symbol, facts, cache);
				printf("[resolve_for_symbol] RHS symbol %s resolved to: ", curr_rhs_symbol->str_repr);
				print_list_endl(curr_rule_res);

				// if current symbol is negated, apply negation on the results
				if (symbol->is_negated)
				{
					for (size_t l = 0; curr_symbol_res[l] > -1; l++)
					{
						if (curr_symbol_res[l] == 1)
							curr_symbol_res[l] = 0;
						else
							curr_symbol_res[l] = 1;
					}
				}
				rhs_symbols_res[k] = curr_symbol_res;
			}
			
			// generate inputs for truth table
			int num_elems = unique_symbols(rhs_symbols);
			int total_rows = pow(2, num_elems);
			int **table = (int **)calloc(total_rows, sizeof(int *));
			for (size_t i = 0; i < total_rows; i++)
				table[i] = (int *)calloc(num_elems, sizeof(int));
			int* aux = (int *)calloc(num_elems, sizeof(int));
			int curr_table_y = 0;
			Symbol **mapping = generate_mapping_for_truth_table(rhs_symbols);

			// generate & resolve truth permutations
			generate_truth_permutations(
				table,
				num_elems,
				&curr_table_y,
				0,
				aux
			);

			int *permutation_results = resolve_truth_permutations(
				mapping,
				table,
				curr_table_y,
				rhs_symbols
			);

			// filter the truth table to only extract relevant rules
			int *table_indices_to_keep = filter_tt_for_resolve_for_symbol(
				table,
				rhs_symbols_res,
				rhs_symbols,
				mapping,
				permutation_results,
				curr_lhs_res,
				num_elems,
				rule_to_resolve->resolve_type
			);

			apply_filters(
				table,
				permutation_results,
				table_indices_to_keep,
				total_rows
			);
			
			printf("[resolve_for_symbol] map\n");
			for (size_t i = 0; i < total_rows; i++)
			{
				for (size_t j = 0; j < num_elems; j++)
					printf("%d ", table[i][j]);
				printf(" = [%d]\n", permutation_results[i]);
			}
			printf("[resolve_for_symbol] map end\n");

			// store results of compputation in cache
			store_results_in_cache(
				mapping,
				table,
				cache
			);

			// TODO free all stuff here
			free(rule_str);
			free(rhs_symbols_res);
		}

	}
	
	// if we get no rules to resolve, means it is false by default
	if (!rules_to_resolve[0])
		res[0] = 0;

	// get cache and copy and return
	cache_found = query_map(cache, symbol);
	if (cache_found)
	{
		printf("[resolve_for_symbol] return %s found in cache: ", symbol->str_repr);
		print_list_endl(cache_found);
		printf("========================");

		int len = list_len_neg_1(cache_found);
		memcpy(res, cache_found, len * sizeof(int));
		free(rules_to_resolve);
		free(cache_found);
		return res;
	}
	DIE(1, "[resolve_for_symbol] We did all that but cache is still empty")
}

int *resolve_for_rule(Rulegraph *rg, Rule* rule, char *facts, FtMap* cache)
{
	int **rhs_symbols_res = (int**)calloc(MAX_VALUES, sizeof(int*)); // shh.. hard allocate here
	int *inner_symbols_indices = (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(inner_symbols_indices, -1, MAX_VALUES);
	int inner_symbols_indices_idx = -1;
	char *rule_str = serialize_symbols(rule->symbol_list);

	printf("[resolve_for_rule] Entry for %s\n", rule_str);

	// check if any inner symbols in rhs. If yes, handle them first
	Symbol **inner_symbols = get_inner_symbols(rule->symbol_list, inner_symbols_indices);
	for (size_t k = 0; inner_symbols[k]; k++)
	{
		Symbol *curr_symbol_list = inner_symbols[k];
		int *inner_res = resolve_for_inner(rg, curr_symbol_list, cache);
		int idx_for_inner_symbol = inner_symbols_indices[k];

		rhs_symbols_res[idx_for_inner_symbol] = inner_res;
	}

	// iterate symbols in rules
	for (size_t i = 0; rule->symbol_list[i]; i++)
	{
		Symbol *curr_symbol = rule->symbol_list[i];

		if (curr_symbol->type == INNER)
			continue;

		printf("[resolve_for_rule] resolving symbol %s: ", curr_symbol->str_repr);
		int *curr_symbol_res = resolve_for_symbol(rg, curr_symbol, facts, cache);
		printf("[resolve_for_rule] symbol %s resolved to: ", curr_symbol->str_repr);
		print_list_endl(curr_symbol_res);

		// if current symbol is negated, apply negation on the results
		if (curr_symbol->is_negated)
		{
			for (size_t l = 0; curr_symbol_res[l] > -1; l++)
			{
				if (curr_symbol_res[l] == 1)
					curr_symbol_res[l] = 0;
				else
					curr_symbol_res[l] = 1;
			}
		}
		rhs_symbols_res[i] = curr_symbol_res;
	}
	
	// generate inputs for truth table
	int num_elems = unique_symbols(rule->symbol_list);
	int total_rows = pow(2, num_elems);
	int **table = (int **)calloc(total_rows, sizeof(int *));
	for (size_t i = 0; i < total_rows; i++)
		table[i] = (int *)calloc(num_elems, sizeof(int));
	int* aux = (int *)calloc(num_elems, sizeof(int));
	int curr_table_y = 0;
	Symbol **mapping = generate_mapping_for_truth_table(rule->symbol_list);

	// generate & resolve truth permutations
	generate_truth_permutations(
		table,
		num_elems,
		&curr_table_y,
		0,
		aux
	);

	int *permutation_results = resolve_truth_permutations(
		mapping,
		table,
		curr_table_y,
		rule->symbol_list
	);

	// filter the truth table to only extract relevant rules
	int *table_indices_to_keep = filter_tt_for_resolve_for_rule(
		table,
		rhs_symbols_res,
		rule->symbol_list,
		mapping,
		permutation_results,
		num_elems
	);

	apply_filters(
		table,
		permutation_results,
		table_indices_to_keep,
		total_rows
	);

	printf("[resolve_for_rule] map\n");
	for (size_t i = 0; i < total_rows; i++)
	{
		for (size_t j = 0; j < num_elems; j++)
			printf("%d ", table[i][j]);
		printf(" = [%d]\n", permutation_results[i]);
	}
	printf("[resolve_for_rule] map end\n");

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
	
	// TODO free all stuff here
	free(rule_str);

	return permutation_results;
}

int *resolve_for_inner(Rulegraph *rg, Symbol* inner_symbols, FtMap *cache)
{
	return 0;
}
