#include "rules.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

# define MAX_POSSIBLE_RESULTS 16

Symbol *generate_symbol_from(char *str, int is_inner, Symbol **inner_symbols)
{
	Symbol *res = (Symbol *)malloc(sizeof(Symbol));
	
	res->possible_results = (int *)calloc(MAX_POSSIBLE_RESULTS, sizeof(int));// shh.. hard allocate here
	res->is_negated = 0;
	if (str[0] == '!')
		res->is_negated = 1;

	if (is_inner)
	{
		res->type = INNER;
		res->operator= NOP;
		res->inner_symbols = inner_symbols;
		res->str_repr = strdup("<Inner symbols>");
	}
	else
	{
		// check for operator
		if (strcspn(str, "+^|") < strlen(str))
		{
			res->type = OPERATOR;
			if (strcspn(str, "+") < strlen(str))
				res->operator= AND;
			else if (strcspn(str, "|") < strlen(str))
				res->operator= OR;
			else
				res->operator= XOR;
		}
		else
		{
			res->type = VARIABLE;
			res->operator= NOP;
		}

		res->inner_symbols = NULL;
		res->str_repr = strdup(str);
	}
	return res;
}

void serialize_symbols_inner(Symbol *inner_symbol, char *res)
{
	strcat(res, "( ");
	for (size_t i = 0; inner_symbol->inner_symbols[i]; i++)
	{
		Symbol *symbol = inner_symbol->inner_symbols[i];
		if (symbol->type == INNER)
			serialize_symbols_inner(symbol, res);
		else
			strcat(res, symbol->str_repr);
		strcat(res, " ");
	}
	strcat(res, ")");
}

char *serialize_symbols(Symbol **symbols)
{
	char *res = (char *)calloc(256, sizeof(char *)); // shh.. dont tell bocal i hard allocate here also
	for (size_t i = 0; symbols[i]; i++)
	{
		Symbol *symbol = symbols[i];
		if (symbol->type == INNER)
			serialize_symbols_inner(symbol, res);
		else
			strcat(res, symbol->str_repr);
		strcat(res, " ");
	}

	return res;
}

void free_symbol(Symbol *symbol)
{
	if (symbol->inner_symbols)
		free_symbol_list(symbol->inner_symbols); 
	if (symbol->possible_results)
		free(symbol->possible_results);
	free(symbol->str_repr);
	free(symbol);
}

void free_symbol_list(Symbol **symbols)
{
	int i = -1;
	while (symbols[++i])
		free_symbol(symbols[i]);
	free(symbols);
}

void free_rule(Rule *rule)
{
	if (rule->symbol_list)
		free_symbol_list(rule->symbol_list);

	// we trust free_rule_list to free these already - HAVE FAITH
	// if (rule->implies && rule->implies->ref_cnt == 1)
	// 	free_rule(rule->implies);
	// if (rule->iff && rule->implies->ref_cnt == 1)
	// 	free_rule(rule->iff);

	free(rule);
}

void free_rule_list(Rule **rules)
{
	int i = -1;
	while (rules[++i])
		free_rule(rules[i]);
	free(rules);
}

void free_rulegraph(Rulegraph *rg)
{
	if (rg->all_rules_vertices)
		free_rule_list(rg->all_rules_vertices);
	free(rg);
}

Rulegraph *generate_default_rulegraph()
{
	Rulegraph *res = (Rulegraph *)calloc(1, sizeof(Rulegraph));
	res->all_rules_vertices = (Rule **)calloc(32, sizeof(Rule *)); // hard allocate
	res->vertex_count = 0;
	return res;
}

Rule *search_for_rule(Rulegraph *rg, char *input_symbols, int is_lhs)
{
	Rule **rules = rg->all_rules_vertices;
	int i = -1;
	while (rules[++i])
	{
		Rule *rule = NULL;
		if (is_lhs)
		{
			if (rules[i]->implies)
				rule = rules[i]->implies;
			else
				rule = rules[i]->iff;
		}
		else
			rule = rules[i];
		char *symbols_str = NULL;
		if (rule)
			symbols_str = serialize_symbols(rule->symbol_list);
		else
			symbols_str = strdup("");
		if (!strcmp(input_symbols, symbols_str))
		{
			free(symbols_str);
			return rule;
		}
		free(symbols_str);
	}
	return NULL;
}

Rule *generate_rule_from(Symbol **symbol_list)
{
	Rule *res = (Rule *)calloc(1, sizeof(Rule));
	res->symbol_list = symbol_list;
	res->confirmed_result = 0;
	res->iff = NULL;
	res->implies = NULL;
	res->resolve_type = NO_RESOLVE;
	return res;
}

int update_symbol_with_facts(Symbol *symbol, char *facts)
{
	if (symbol->type == OPERATOR)
		return 0;

	if (symbol->type == VARIABLE || symbol->type == FACT)
	{
		int char_idx = 0;
		bzero(symbol->possible_results, sizeof(int) * MAX_POSSIBLE_RESULTS);
		if (symbol->is_negated)
			char_idx = 1;
		if (strcspn(facts, &(symbol->str_repr[char_idx])) != strlen(facts))
		{
			symbol->possible_results[0] = FT_TRUE;
			symbol->type = FACT;
		}
		else
			symbol->type = VARIABLE;

		return 0;
	}

	int i = -1;
	while (symbol->inner_symbols[++i])
	{
		Symbol *inner_symbol = symbol->inner_symbols[i];
		update_symbol_with_facts(inner_symbol, facts);
	}

	return 0;
}

int update_rule_graph_with_facts(Rulegraph *rule_graph, char *facts)
{
	int i = -1;
	while (++i < rule_graph->vertex_count)
	{
		Rule *rule = rule_graph->all_rules_vertices[i];

		int j = -1;
		while (rule->symbol_list[++j])
		{
			Symbol *symbol = rule->symbol_list[j];
			if (symbol->type == OPERATOR)
				continue;
			update_symbol_with_facts(symbol, facts);
		}
	}
	return 0;
}

void print_rulegraph(Rulegraph *rule_graph)
{
	for (size_t i = 0; rule_graph->all_rules_vertices[i]; i++)
	{
		Rule *rule = rule_graph->all_rules_vertices[i];
		Symbol **lhs_symbols = rule->symbol_list;
		char *lhs_symbols_str = serialize_symbols(lhs_symbols);

		char *rhs_symbols_str = 0;
		char *resolve = 0;

		if (rule->resolve_type == NO_RESOLVE)
		{
			free(lhs_symbols_str);
			continue;
		}
		if (rule->resolve_type == IMPLIES)
		{
			rhs_symbols_str = serialize_symbols(rule->implies->symbol_list);
			resolve = " => ";
		}
		if (rule->resolve_type == IFF)
		{
			rhs_symbols_str = serialize_symbols(rule->iff->symbol_list);
			resolve = " <=> ";
		}
		printf("%s%s%s\n", lhs_symbols_str, resolve, rhs_symbols_str);
		free(lhs_symbols_str);
		free(rhs_symbols_str);
	}
}