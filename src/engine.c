#include "rules.h"
#include "ft_macros.h"
#include "ft_map.h"
#include "engine_utils.h"
#include "colors.h"
#include "shell.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

int *resolve_for_rule(Rulegraph *rg, Rule *rule, char *facts, FtMap *cache, char *original_key, Rule **rule_ignore_list, int level, char *indent);
Symbol **get_inner_symbols(Symbol **list, int *indices);
int *resolve_for_inner(
	Rulegraph *rg,
	Symbol *inner_symbol,
	char *facts,
	FtMap *cache,
	char *original_key,
	Rule **rule_ignore_list,
	int level, char *indent);

// given a symbol, look for rules which implies / iffs that symbol and writes to res
void locate_conditional_rule(Rulegraph *rg, char *input_symbol, Rule **res, Rule **ignore_list)
{
	Rule **rules = rg->all_rules_vertices;

	int i = -1;
	int j = -1;
	while (rules[++i])
	{
		Rule *rule = rules[i];
		Rule *rule_cmp = NULL;

		// check ignore list
		int ignore = 0;
		for (size_t j = 0; ignore_list[j]; j++)
		{
			if (ignore_list[j] == rule)
				ignore = 1;
		}
		if (ignore)
			continue;

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

int *resolve_for_symbol(
	Rulegraph *rg,
	Symbol *symbol,
	char *facts,
	FtMap *cache,
	char *original_key,
	Rule **rule_ignore_list,
	int level,
	char *indent)
{
	DBG(indent, "[resolve_for_symbol] [%s] Entry for %s\n", symbol->str_repr, symbol->str_repr);

	Rule **rules_to_resolve = (Rule **)calloc(MAX_VALUES, sizeof(Rule *)); // shh.. hard allocate here
	int *res = (int *)malloc(MAX_VALUES * sizeof(int));					   // shh.. hard allocate here
	memset(res, -1, MAX_VALUES);

	// check facts first
	if (symbol->type == FACT)
	{
		free_rule_list(rules_to_resolve);
		DBG(indent, "[resolve_for_symbol] [%s] %s is found in facts, is_negated? %d\n", symbol->str_repr, symbol->str_repr, symbol->is_negated);
		res[0] = 1;
		if (symbol->is_negated)
			res[0] = 0;
		return res;
	}

	char *symbol_key = symbol->str_repr;
	if (symbol->is_negated)
		symbol_key += 1;

	// if we already have some ignoring rules and our symbol_key is the same as original key,
	// we return a predefined [0,1] amgiguity
	if (rule_ignore_list[0] != 0 && !strcmp(symbol_key, original_key))
	{
		DBG(indent, "[resolve_for_symbol] [%s] %s reentry, resolving to [0, 1]\n", symbol_key, symbol->str_repr);
		free_rule_list(rules_to_resolve);
		memset(res, -1, MAX_VALUES);
		res[0] = 0;
		res[1] = 1;
		return res;
	}

	// check cache. If found, return here
	int *cache_found = query_map(cache, symbol);
	if (cache_found)
	{
		DBG(indent, "[resolve_for_symbol] [%s] %s found in cache: ", symbol_key, symbol->str_repr);
#ifdef __DEBUG__
		print_list_endl(cache_found);
#endif //__DEBUG__
		free_rule_list(rules_to_resolve);
		free(res);

		return cache_found;
	}

	// search for all rules where rhs has symbol
	locate_conditional_rule(rg, symbol_key, rules_to_resolve, rule_ignore_list);
	// iterate through all results
	for (size_t i = 0; rules_to_resolve[i]; i++)
	{
		Rule *rule_to_resolve = rules_to_resolve[i];
		char *rule_str = serialize_symbols(rule_to_resolve->symbol_list);

		// add rule to ignore list before resolving
		add_ignore_list(rule_ignore_list, rule_to_resolve);

		// resolve that rule
		DBG(indent, "[resolve_for_symbol] [%s] resolving rule %s\n", symbol_key, rule_str);
		char *tmp_indent = expand_indent_new(indent);
		int *curr_rule_res = resolve_for_rule(rg, rule_to_resolve, facts, cache, original_key, rule_ignore_list, level, tmp_indent);
		free(tmp_indent);
		DBG(indent, "[resolve_for_symbol] [%s] rule %s resolved to: ", symbol_key, rule_str);
#ifdef __DEBUG__
		print_list_endl(curr_rule_res);
#endif //_

		// iterate through all possible results for that rule
		int j = -1;
		// just a marker here to tell the filter below wether or not
		// the current LHS has results will yield more than 1 result
		while (curr_rule_res[++j] != -1)
		{
			int curr_lhs_res = curr_rule_res[j];
			int **rhs_symbols_res = (int **)calloc(MAX_VALUES, sizeof(int *));	  // shh.. hard allocate here
			int *inner_symbols_indices = (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
			memset(inner_symbols_indices, -1, MAX_VALUES);
			int inner_symbols_indices_idx = -1;

			DBG(indent, "[resolve_for_symbol] [%s] resolving RHS if rule %s is %d\n", symbol_key, rule_str, curr_lhs_res);

			Symbol **rhs_symbols = 0;
			if (rule_to_resolve->resolve_type == IMPLIES)
				rhs_symbols = rule_to_resolve->implies->symbol_list;
			else
				rhs_symbols = rule_to_resolve->iff->symbol_list;

			// check if any inner symbols in rhs. If yes, handle them first
			Symbol **inner_symbols = get_inner_symbols(rhs_symbols, inner_symbols_indices);
			for (size_t k = 0; inner_symbols[k]; k++)
			{
				Symbol *curr_inner_symbol = inner_symbols[k];
				int *inner_res = resolve_for_inner(rg, curr_inner_symbol, facts, cache, original_key, rule_ignore_list, level, indent);
				int idx_for_inner_symbol = inner_symbols_indices[k];

				rhs_symbols_res[idx_for_inner_symbol] = inner_res;
			}
			free(inner_symbols_indices);
			free_symbol_list(inner_symbols);

			// iterate through rhs symbols
			for (size_t k = 0; rhs_symbols[k]; k++)
			{
				Symbol *curr_rhs_symbol = rhs_symbols[k];

				if (curr_rhs_symbol->type == INNER || curr_rhs_symbol->type == OPERATOR)
				{
					if (curr_rhs_symbol->type == OPERATOR)
						rhs_symbols_res[k] = (int *)-1;
					continue;
				}

				// call resolve_for_symbol() for current symbol and save them in rhs_symbols_res
				// howver, if the current symbol is the symbol that we are tring to solve, put [0, 1, -1] as a placeholder
				DBG(indent, "[resolve_for_symbol] [%s] resolving RHS symbol %s\n", symbol_key, curr_rhs_symbol->str_repr);
				char *tmp_indent = expand_indent_new(indent);
				int *curr_symbol_res = resolve_for_symbol(rg, curr_rhs_symbol, facts, cache, original_key, rule_ignore_list, level, tmp_indent);
				free(tmp_indent);
				DBG(indent, "[resolve_for_symbol] [%s] RHS symbol %s resolved to: ", symbol_key, curr_rhs_symbol->str_repr);
#ifdef __DEBUG__
				print_list_endl(curr_symbol_res);
#endif //_

				// if current symbol is negated, apply negation on the results
				// if (symbol->is_negated)
				// {
				// 	for (size_t l = 0; curr_symbol_res[l] > -1; l++)
				// 	{
				// 		if (curr_symbol_res[l] == 1)
				// 			curr_symbol_res[l] = 0;
				// 		else
				// 			curr_symbol_res[l] = 1;
				// 	}
				// }
				rhs_symbols_res[k] = curr_symbol_res;
			}

			// generate inputs for truth table
			int num_elems = unique_symbols(rhs_symbols);
			int total_rows = pow(2, num_elems);
			int **table = (int **)calloc(total_rows + 1, sizeof(int *));
			for (size_t i = 0; i < total_rows; i++)
				table[i] = (int *)calloc(num_elems + 1, sizeof(int));
			int *aux = (int *)calloc(num_elems, sizeof(int));
			int curr_table_y = 0;
			Symbol **mapping = generate_mapping_for_truth_table(rhs_symbols);

			// generate & resolve truth permutations
			generate_truth_permutations(
				table,
				num_elems,
				&curr_table_y,
				0,
				aux);

			int *permutation_results = resolve_truth_permutations(
				mapping,
				table,
				curr_table_y,
				rhs_symbols);

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

			DBG(indent, "[resolve_for_symbol] [%s] tt unfiltered \n", symbol_key);
			for (size_t i = 0; table[i]; i++)
			{
				for (size_t j = 0; j < num_elems; j++)
					DBG(indent, "%d ", table[i][j]);
				DBG(indent, " = [%d]\n", permutation_results[i]);
			}

			apply_filters(
				table,
				permutation_results,
				table_indices_to_keep,
				total_rows);

			DBG(indent, "[resolve_for_symbol] [%s] tt filtered\n", symbol_key);
			for (size_t i = 0; table[i]; i++)
			{
				for (size_t j = 0; j < num_elems; j++)
					DBG(indent, "%d ", table[i][j]);
				DBG(indent, " = [%d]\n", permutation_results[i]);
			}

			// store results of computation in cache
			store_results_in_cache(
				mapping,
				table,
				cache,
				indent
				);

			for (size_t i = 0; table[i]; i++)
				free(table[i]);
			free(table);
			free(aux);
			free(mapping);
			free(permutation_results);
			free(table_indices_to_keep);

			// TODO free all stuff here
			for (size_t i = 0; rhs_symbols_res[i]; i++)
			{
				if (rhs_symbols_res[i] == (int *)-1)
					continue;
				free(rhs_symbols_res[i]);
			}

			free(rhs_symbols_res);
		}

		free(curr_rule_res);
		free(rule_str);

		// remove rule from ignorelist
		remove_ignore_list(rule_ignore_list, rule_to_resolve, indent);
	}

	// if we get no rules to resolve, means it is false by default
	if (!rules_to_resolve[0])
		res[0] = 0;

	// get cache and copy and return
	cache_found = query_map(cache, symbol);
	if (cache_found)
	{
		DBG(indent, "[resolve_for_symbol] [%s] return %s found in cache: ", symbol_key, symbol->str_repr);
#ifdef __DEBUG__
		print_list_endl(cache_found);
#endif //_

		if (list_len_neg_1(cache_found) > 1)
		{
			WARN(debug_indent,  "[resolve_for_symbol] [%s] WARN: resolved ambigious result, assuming false\n", symbol_key)
			memset(cache_found, -1, MAX_VALUES * sizeof(int));
			cache_found[0] = 0;
		}

		memcpy(res, cache_found, MAX_VALUES * sizeof(int));
		free(rules_to_resolve);
		free(cache_found);
		return res;
	}
	DBG(indent, "[resolve_for_symbol] [%s] We did all that but cache is still empty, resolving [0], is_negated: %d\n", symbol_key, symbol->is_negated);
	if (symbol->is_negated)
		res[0] = 1;
	else
		res[0] = 0;

	free(rules_to_resolve);
	return res;
}

int *resolve_for_rule(
	Rulegraph *rg,
	Rule *rule,
	char *facts,
	FtMap *cache,
	char *original_key,
	Rule **rule_ignore_list,
	int level,
	char *indent)
{
	int **rhs_symbols_res = (int **)calloc(MAX_VALUES, sizeof(int *));	  // shh.. hard allocate here
	int *inner_symbols_indices = (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(inner_symbols_indices, -1, MAX_VALUES);
	int inner_symbols_indices_idx = -1;
	char *rule_str = serialize_symbols(rule->symbol_list);

	DBG(indent, "[resolve_for_rule] Entry for %s\n", rule_str);

	// check if any inner symbols in rhs. If yes, handle them first
	Symbol **inner_symbols = get_inner_symbols(rule->symbol_list, inner_symbols_indices);
	for (size_t k = 0; inner_symbols[k]; k++)
	{
		Symbol *curr_inner_symbol = inner_symbols[k];
		int *inner_res = resolve_for_inner(rg, curr_inner_symbol, facts, cache, original_key, rule_ignore_list, level, indent);
		int idx_for_inner_symbol = inner_symbols_indices[k];

		rhs_symbols_res[idx_for_inner_symbol] = inner_res;
	}
	free(inner_symbols_indices);
	free(inner_symbols);

	// iterate symbols in rules
	for (size_t i = 0; rule->symbol_list[i]; i++)
	{
		Symbol *curr_symbol = rule->symbol_list[i];

		if (curr_symbol->type == INNER || curr_symbol->type == OPERATOR)
		{
			if (curr_symbol->type == OPERATOR)
				rhs_symbols_res[i] = (int *)-1;
			continue;
		}

		// TODO circular check
		DBG(indent, "[resolve_for_rule] resolving symbol %s\n", curr_symbol->str_repr);
		char *symbol_key = curr_symbol->str_repr;
		char *tmp_indent = expand_indent_new(indent);
		int *curr_symbol_res = resolve_for_symbol(rg, curr_symbol, facts, cache, original_key, rule_ignore_list, level, tmp_indent);
		free(tmp_indent);
		DBG(indent, "[resolve_for_rule] symbol %s resolved to: ", curr_symbol->str_repr);
#ifdef __DEBUG__
		print_list_endl(curr_symbol_res);
#endif //_

		// if current symbol is negated, apply negation on the results
		// if (curr_symbol->is_negated)
		// {
		// 	for (size_t l = 0; curr_symbol_res[l] > -1; l++)
		// 	{
		// 		if (curr_symbol_res[l] == 1)
		// 			curr_symbol_res[l] = 0;
		// 		else
		// 			curr_symbol_res[l] = 1;
		// 	}
		// }
		rhs_symbols_res[i] = curr_symbol_res;
	}

	// generate inputs for truth table
	int num_elems = unique_symbols(rule->symbol_list);
	int total_rows = pow(2, num_elems);
	int **table = (int **)calloc(total_rows + 1, sizeof(int *));
	for (size_t i = 0; i < total_rows; i++)
		table[i] = (int *)calloc(num_elems + 1, sizeof(int));
	int *aux = (int *)calloc(num_elems, sizeof(int));
	int curr_table_y = 0;
	Symbol **mapping = generate_mapping_for_truth_table(rule->symbol_list);

	// generate & resolve truth permutations
	generate_truth_permutations(
		table,
		num_elems,
		&curr_table_y,
		0,
		aux);

	int *permutation_results = resolve_truth_permutations(
		mapping,
		table,
		curr_table_y,
		rule->symbol_list);

	// filter the truth table to only extract relevant rules
	int *table_indices_to_keep = filter_tt_for_resolve_for_rule(
		table,
		rhs_symbols_res,
		rule->symbol_list,
		mapping,
		permutation_results,
		num_elems);

	DBG(indent, "[resolve_for_rule] tt unfiltered\n");
	for (size_t i = 0; table[i]; i++)
	{
		// int num =
		for (size_t j = 0; j < num_elems; j++)
		{
			int num = table[i][j];
			DBG(indent, "%d ", num)
		}
		DBG(indent, " = [%d]\n", permutation_results[i])
	}


	apply_filters(
		table,
		permutation_results,
		table_indices_to_keep,
		total_rows);

	DBG(indent, "[resolve_for_rule] tt filtered\n");
	for (size_t i = 0; table[i]; i++)
	{
		// int num =
		for (size_t j = 0; j < num_elems; j++)
		{
			int num = table[i][j];
			DBG(indent, "%d ", num)
		}
		DBG(indent, " = [%d]\n", permutation_results[i])
	}

	// return either true, false, or true / false
	res_deduper(permutation_results);

	// TODO free all stuff here
	for (size_t i = 0; rhs_symbols_res[i]; i++)
	{
		if (rhs_symbols_res[i] == (int *)-1)
			continue;
		free(rhs_symbols_res[i]);
	}
	free(rhs_symbols_res);

	for (size_t i = 0; table[i]; i++)
		free(table[i]);
	free(table);

	free(aux);
	free(mapping);
	free(table_indices_to_keep);
	free(rule_str);

	return permutation_results;
}

int *resolve_for_inner(
	Rulegraph *rg,
	Symbol *inner_symbol,
	char *facts,
	FtMap *cache,
	char *original_key,
	Rule **rule_ignore_list,
	int level,
	char *indent)
{
	int **symbols_res = (int **)calloc(MAX_VALUES, sizeof(int *)); // shh.. hard allocate here
	Symbol **symbols = inner_symbol->inner_symbols;

	int *inner_symbols_indices = (int *)malloc(MAX_VALUES * sizeof(int)); // shh.. hard allocate here
	memset(inner_symbols_indices, -1, MAX_VALUES);
	Symbol **_inner_symbols = get_inner_symbols(symbols, inner_symbols_indices);

	for (size_t k = 0; _inner_symbols[k]; k++)
	{
		Symbol *curr_inner_symbol = _inner_symbols[k];
		int *inner_res = resolve_for_inner(rg, curr_inner_symbol, facts, cache, original_key, rule_ignore_list, level, indent);
		int idx_for_inner_symbol = inner_symbols_indices[k];

		symbols_res[idx_for_inner_symbol] = inner_res;
	}
	free_symbol_list(_inner_symbols);
	free(inner_symbols_indices);

	// iterate through rhs symbols
	for (size_t k = 0; symbols[k]; k++)
	{
		Symbol *curr_rhs_symbol = symbols[k];

		if (curr_rhs_symbol->type == INNER || curr_rhs_symbol->type == OPERATOR)
		{
			if (curr_rhs_symbol->type == OPERATOR)
				symbols_res[k] = (int *)-1;
			continue;
		}

		char *symbol_key = curr_rhs_symbol->str_repr;
		if (curr_rhs_symbol->is_negated)
			symbol_key += 1;

		// call resolve_for_symbol() for current symbol and save them in symbols_res
		// howver, if the current symbol is the symbol that we are tring to solve, put [0, 1, -1] as a placeholder
		DBG(indent, "[resolve_for_inner] [%s] resolving RHS symbol %s\n", symbol_key, curr_rhs_symbol->str_repr);
		char *tmp_indent = expand_indent_new(indent);
		int *curr_symbol_res = resolve_for_symbol(rg, curr_rhs_symbol, facts, cache, original_key, rule_ignore_list, level, tmp_indent);
		free(tmp_indent);
		DBG(indent, "[resolve_for_inner] [%s] RHS symbol %s resolved to: ", symbol_key, curr_rhs_symbol->str_repr);
#ifdef __DEBUG__
		print_list_endl(curr_symbol_res);
#endif //_

		symbols_res[k] = curr_symbol_res;
	}

	// generate inputs for truth table
	int num_elems = unique_symbols(symbols);
	int total_rows = pow(2, num_elems);
	int **table = (int **)calloc(total_rows + 1, sizeof(int *));
	for (size_t i = 0; i < total_rows; i++)
		table[i] = (int *)calloc(num_elems + 1, sizeof(int));
	int *aux = (int *)calloc(num_elems, sizeof(int));
	int curr_table_y = 0;
	Symbol **mapping = generate_mapping_for_truth_table(symbols);

	// generate & resolve truth permutations
	generate_truth_permutations(
		table,
		num_elems,
		&curr_table_y,
		0,
		aux);

	int *permutation_results = resolve_truth_permutations(
		mapping,
		table,
		curr_table_y,
		symbols);

	// filter the truth table to only extract relevant rules
	int *table_indices_to_keep = filter_tt_for_resolve_for_inner(
		table,
		symbols_res,
		symbols,
		mapping,
		permutation_results,
		num_elems);

	apply_filters(
		table,
		permutation_results,
		table_indices_to_keep,
		total_rows);

	// return either true, false, or true / false
	res_deduper(permutation_results);

	DBG(indent, "[resolve_for_inner] [-] resolved to: ");
#ifdef __DEBUG__
	print_list_endl(permutation_results);
#endif //_

	for (size_t i = 0; table[i]; i++)
		free(table[i]);
	free(table);

	for (size_t i = 0; symbols_res[i]; i++)
	{
		if (symbols_res[i] == (int *)-1)
			continue;
		free(symbols_res[i]);
	}

	free(symbols_res);

	free(aux);
	free(mapping);
	free(table_indices_to_keep);

	return permutation_results;
}

void resolve_query(Rulegraph *rule_graph, char *facts_list, char *query_list)
{
	// init cache
	FtMap *cache = ft_map_new(69);

	for (size_t query_idx = 0; query_list[query_idx]; query_idx++)
	{
		char query_str[2] = {query_list[query_idx], 0};
		Symbol *query_symbol = generate_symbol_from(query_str, 0, 0);
		char *debug_indent = calloc(INDENT_SIZE, 1);

		// check symbol for fact
		char *symbol_key = query_symbol->str_repr;
		if (query_symbol->is_negated)
			symbol_key += 1;
		if (strstr(facts_list, symbol_key))
			query_symbol->type = FACT;

		Rule **ignore_list = calloc(1024, sizeof(Rule *));
		int level = 0;
		int *res = resolve_for_symbol(
			rule_graph,
			query_symbol,
			facts_list,
			cache,
			query_str,
			ignore_list,
			level,
			debug_indent);

		// return either true, false, or true / false
		res_deduper(res);

		// if ambigious, assume false
		if (list_len_neg_1(res) > 1)
			printf(GRN "[resolve_query] %s is %d" RESET "\n", query_str, 0);
		else
			printf(GRN "[resolve_query] %s is %d" RESET "\n", query_str, res[0]);

		free(debug_indent);
		free_symbol(query_symbol);
		free(res);
		free(ignore_list);
	}

	free_ft_map(cache);
}