#include "rules.h"
#include <stdlib.h>
#include <string.h>

Symbol *generate_symbol_from(char* str, int is_inner, Symbol** inner_symbols){
	Symbol *res = (Symbol *)malloc(sizeof(Symbol));
	res->result = RESULT_INIT;
	res->is_negated = 0;
	if (str[0] == '!')
		res->is_negated = 1;
		
	if (is_inner) {
		res->type = INNER;
		res->operator = NOP;
		res->inner_symbols = inner_symbols;
		res->str_repr = strdup("<Inner symbols>");
	}
	else {
		// check for operator
		if (strcspn(str, "+^|") < strlen(str))
		{
			res->type = OPERATOR;
			if (strcspn(str, "+") < strlen(str))
				res->operator = AND;
			else if (strcspn(str, "|") < strlen(str))
				res->operator = OR;
			else
				res->operator = XOR;	
		}
		else
		{
			res->type = VARIABLE;
			res->operator = NOP;
		}

		res->inner_symbols = NULL;
		res->str_repr = strdup(str);
	}
	return res;
}

void serialize_symbols_inner(Symbol* inner_symbol, char* res)
{
	strcat(res, "( ");
	for (size_t i = 0; inner_symbol->inner_symbols[i]; i++)
	{
		Symbol* symbol = inner_symbol->inner_symbols[i];
		if (symbol->type == INNER)
			serialize_symbols_inner(symbol, res);
		else
			strcat(res, symbol->str_repr);
		strcat(res, " ");
	}
	strcat(res, ")");
}

char *serialize_symbols(Symbol** symbols)
{
	char* res = (char*)calloc(256, sizeof(char*)); // shh.. dont tell bocal i hard allocate here also
	for (size_t i = 0; symbols[i]; i++)
	{
		Symbol* symbol = symbols[i];
		if (symbol->type == INNER)
			serialize_symbols_inner(symbol, res);
		else
			strcat(res, symbol->str_repr);
		strcat(res, " ");
		// printf("%s\n", symbol->str_repr);
	}
	
	return res;
}

void free_symbol(Symbol *symbol)
{
	if (symbol->inner_symbols)
		free_symbol_list(symbol->inner_symbols);
	free(symbol->str_repr);
	free(symbol);
}

void free_symbol_list(Symbol **symbols)
{
	int i = -1;
	while(symbols[++i])
		free_symbol(symbols[i]);
	free(symbols);
}

void free_rule(Rule* rule)
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

void free_rule_list(Symbol **rules)
{
	int i = -1;
	while(rules[++i])
		free_rule(rules[i]);
	free(rules);
}
