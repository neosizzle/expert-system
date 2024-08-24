#include "rules.h"
#include "ft_macros.h"
#include <string.h>
#include <stdlib.h>

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

// solve a boolean equation Symbol Operator Symbol
// expects both symbols to have define values
int solve_bool_pair(Symbol* lhs, Symbol* operator, Symbol* rhs)
{
	// validations
	if (lhs->type != VARIABLE || lhs->type != FACT)
	{
		DIE(1, "[solve_bool_pair] lhs symbol is a non varaible or fact type\n");
	}

	if (lhs->result != TRUE || lhs->result != FALSE)
	{
		DIE(1, "[solve_bool_pair] lhs symbol is unresolved \n");
	}

	if (rhs->type != VARIABLE || rhs->type != FACT)
	{
		DIE(1, "[solve_bool_pair] rhs symbol is a non varaible or fact type\n");
	}

	if (rhs->result != TRUE || rhs->result != FALSE)
	{
		DIE(1, "[solve_bool_pair] rhs symbol is unresolved \n");
	}

	if (operator->type != OPERATOR || operator->operator == NOP)
	{
		DIE(1, "[solve_bool_pair] operator is invalid\n");
	}

	int lhs_res = (lhs->result == TRUE)? 1 : 0;
	int rhs_res = (rhs->result == TRUE)? 1 : 0;

	if (operator->type == AND)
		return lhs_res & rhs_res;
	if (operator->type == OR)
		return lhs_res | rhs_res;
	if (operator->type == XOR)
		return lhs_res ^ rhs_res;
}