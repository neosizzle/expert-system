#include "ft_macros.h"
#include "rules.h"
#include "get_next_line.h"
#include "engine.h"
#include "ft_map.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#define ALPHA_COUNT 26
#define IMPL_RESOLVER " => "
#define IFF_RESOLVER " <=> "

int find_matching_rp(char *line)
{
	int res;
	int parent_stack;

	parent_stack = 0;
	res = -1;
	while (line[++res])
	{
		if (line[res] == '(')
			parent_stack += 1;
		if (line[res] == ')')
		{
			parent_stack -= 1;
			if (parent_stack == -1)
				return res;
		}
	}
	return -1;
}

Symbol **parse_expression(char *line)
{
	Symbol **all_inner_symbols[8];							// shh.. dont tell bocal i hard allocate here
	Symbol **res = (Symbol **)calloc(16, sizeof(Symbol *)); // shh.. dont tell bocal i hard allocate here also

	int inner_symbols_idx = 0;

	// do we find any '(' in the current line?
	int i = strcspn(line, "(");

	// we have '(', so we need to process the things inside of the '(' first
	while (i < strlen(line))
	{
		// i will always point to the '(', so we will find the matching ')' here.
		int matching_rp = find_matching_rp(line + i + 1); // offset + 0 index?

		// extract the subscring and recurse to get the symbols inside the parenthesis
		char *inner = strndup(line + i + 1, matching_rp);
		all_inner_symbols[inner_symbols_idx++] = parse_expression(inner);
		free(inner);

		i += matching_rp + 2; // offset + 0 index

		// no more '('? break.
		int next_lp = strcspn(line + i, "(");
		if (!(next_lp < strlen(line + i)))
			break;
		i += next_lp;
	}

	char *linedup = strdup(line);
	char *curr_token = strtok(linedup, " ");
	int pstack = 0;
	int res_idx = 0;
	inner_symbols_idx = 0;

	// re-iterate through all tokens now, not just the '('s
	while (curr_token)
	{
		// if we found a '(', replace the entire section with an
		// inner symbol with symbol lists
		if (strcspn(curr_token, "(") < strlen(curr_token))
		{
			// first '(' found, find symbol list to replace
			if (pstack == 0)
			{
				res[res_idx] = generate_symbol_from(curr_token, 1, all_inner_symbols[inner_symbols_idx]);
				res_idx += 1;
				inner_symbols_idx += 1;
			}

			for (size_t i = 0; i < strlen(curr_token); i++)
			{
				if (curr_token[i] == '(')
					pstack += 1;
			}
		}

		if (strcspn(curr_token, ")") < strlen(curr_token))
		{
			for (size_t i = 0; i < strlen(curr_token); i++)
			{
				if (curr_token[i] == ')')
					pstack -= 1;
			}
			curr_token = strtok(NULL, " ");
			continue;
		}

		// real symbol which is not in parenthesis now, parse it normally
		if (pstack == 0)
		{
			res[res_idx] = generate_symbol_from(curr_token, 0, 0);
			res_idx += 1;
		}

		curr_token = strtok(NULL, " ");
	}
	free(linedup);
	return res;
}

int parse_facts_or_query(char *line, char *symbol_str_list)
{
	int i = 0;
	char *facts = strtok(line, "#");
	while (++i)
	{
		if (facts[i] == '\n' || facts[i] == 0 || facts[i] == ' ')
			break;

		int j = -1;
		while (j++ < ALPHA_COUNT)
		{
			if (symbol_str_list[j] == 0)
			{
				symbol_str_list[j] = facts[i];
				break;
			}
		}
		if (j == ALPHA_COUNT)
		{
			EPRINTF("Facts/ query are too much, more than %d\n", ALPHA_COUNT)
			return 1;
		}
	}

	return 0;
}

void parse_rule(char *line, Rulegraph *rule_graph)
{
	char *rule = strtok(line, "#");
	char lhs[128] = {0};
	char rhs[128] = {0};

	// check if rule has implication or iff
	char *resolver = 0;
	char *resolver_pos = 0;
	if (resolver_pos = strstr(rule, IMPL_RESOLVER))
		resolver = IMPL_RESOLVER;
	else if (resolver_pos = strstr(rule, IFF_RESOLVER))
		resolver = IFF_RESOLVER;
	else
	{
		EPRINTF("No resolve found at rule %s\n", rule);
		return;
	}

	// clean the rule and split into 2 sides
	if (rule[strlen(rule) - 1] == '\n')
		rule[strlen(rule) - 1] = 0;
	strncpy(lhs, rule, resolver_pos - rule);
	strncpy(rhs, resolver_pos + strlen(resolver), (strlen(rule) - strlen(lhs) - strlen(resolver)));

	// get the expressions for both sides
	// TODO for rhs, we search previous rules for the same symbols
	Symbol **lhs_symbols = parse_expression(lhs);
	Symbol **rhs_symbols = parse_expression(rhs);
	char *lhs_symbols_str = serialize_symbols(lhs_symbols);
	char *rhs_symbols_str = serialize_symbols(rhs_symbols);

	// resolve rhs rule
	// search rhs rule
	// if not found
	// construct new rule and add in rulegrapg and increm vertex count
	// return new rule address
	// if found
	// return for resolver assignation later
	Rule *rhs_rule = search_for_rule(rule_graph, rhs_symbols_str, 0);
	if (!rhs_rule)
	{
		rhs_rule = generate_rule_from(rhs_symbols);
		rule_graph->all_rules_vertices[rule_graph->vertex_count++] = rhs_rule;
	}
	else
		free_symbol_list(rhs_symbols);

	// resolve lhs rule
	// search lhs rule
	// if not found, construct new rule with rhs resolver, add rulegrapg and increm vertex count
	// if found, check for existing resolver, and add resolver if old resolver does not exist
	Rule *lhs_rule = search_for_rule(rule_graph, lhs_symbols_str, 1);
	if (!lhs_rule)
	{
		lhs_rule = generate_rule_from(lhs_symbols);
		if (!strcmp(resolver, IFF_RESOLVER))
		{
			lhs_rule->resolve_type = IFF;
			lhs_rule->iff = rhs_rule;
		}
		else
		{
			lhs_rule->resolve_type = IMPLIES;
			lhs_rule->implies = rhs_rule;
		}
		rule_graph->all_rules_vertices[rule_graph->vertex_count++] = lhs_rule;
	}
	else
	{
		if (lhs_rule->resolve_type != NO_RESOLVE)
		{
			EPRINTF("Resolve already found at rule %s\n", lhs_symbols_str);
			return;
		}

		if (!strcmp(resolver, IFF_RESOLVER))
		{
			lhs_rule->resolve_type = IFF;
			lhs_rule->iff = rhs_rule;
		}
		else
		{
			lhs_rule->resolve_type = IMPLIES;
			lhs_rule->implies = rhs_rule;
		}
	}

	// printf("rule %s with %s @ %s\n", lhs, rhs, rule);
	free(lhs_symbols_str);
	free(rhs_symbols_str);
}

void parse_input_file(int fd, Rulegraph *rule_graph, char *query_list, char *facts_list) // take in array of rules
{
	for (char *line = get_next_line(fd); line != 0; free(line), line = get_next_line(fd))
	{
		// if line starts with # or blank line, continue
		if (line[0] == '#' || line[0] == '\n')
			continue;

		// if line starts with =, parse facts
		else if (line[0] == '=')
			parse_facts_or_query(line, facts_list);

		// if line starts with ?, parse query
		else if (line[0] == '?')
			parse_facts_or_query(line, query_list);

		// else, parse rule
		else
			parse_rule(line, rule_graph);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		EPRINTF("Usage: %s [input_file]\n", argv[0]);
		return 1;
	}

	// open file
	char *input_filename = argv[1];
	int fd = open(input_filename, O_RDONLY);
	if (fd < 0)
	{
		DIE_OS(fd, "Failed to open file %s\n", input_filename);
	}

	// start parsing and get list of rules, query and facts
	Rulegraph *rule_graph = generate_default_rulegraph();
	char *query_list = calloc(ALPHA_COUNT, 1);
	char *facts_list = calloc(ALPHA_COUNT, 1);

	parse_input_file(fd, rule_graph, query_list, facts_list);

	// update the symbols here w/ facts here because we are cool and lazy
	update_rule_graph_with_facts(rule_graph, facts_list);
	print_rulegraph(rule_graph);

	// TODO run expert system with said rules and query
	// TODO test int *filter_tt_for_resolve_for_symbol
	resolve_for_symbol(rule_graph, "C", facts_list);

	// busy spin
	while (1)
	{
		// prompt for fact change
		write(1, "prompt: ", 8);
		char *prompt = get_next_line(0);

		if (strlen(prompt) < ALPHA_COUNT)
		{
			printf("updating facts: %s\n", prompt);
			memset(facts_list, ALPHA_COUNT, 0);
			memcpy(facts_list, prompt, strlen(prompt));
			update_rule_graph_with_facts(rule_graph, facts_list);
		}

		// exit prompt
		if (!strcmp(prompt, "exit\n"))
		{
			free(prompt);
			break;
		}

		// apply changes and run expert system again
		free(prompt);
	}

	free_rulegraph(rule_graph);
	free(facts_list);
	free(query_list);
	printf("OK\n");
	return 0;
}