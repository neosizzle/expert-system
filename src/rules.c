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

void free_symbol(Symbol *symbol)
{
	if (symbol->inner_symbols)
	{
		int i = -1;
		while(symbol->inner_symbols[++i])
			free(symbol->inner_symbols[i]);
	}
	free(symbol->str_repr);
	free(symbol);
}

void free_symbol_list(Symbol **symbols)
{
	int i = -1;
	while(symbols[++i])
		free_symbol(symbols[i]);
}