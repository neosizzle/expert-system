#include "ft_macros.h"
#include "rules.h"
#include "get_next_line.h"
#include "engine.h"
#include "ft_map.h"
#include "shell.h"
#include "validation.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

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

		// no matching ')'
		if (matching_rp < 0)
		{
			free_symbol_list(res);
			EPRINTF("Missing ')' at %s\n", line);
			return 0;
		}		

		// extract the subscring and recurse to get the symbols inside the parenthesis
		char *inner = strndup(line + i + 1, matching_rp);
		Symbol **new_expression = parse_expression(inner);
		if (!new_expression)
		{
			free_symbol_list(res);
			return 0;
		}
		all_inner_symbols[inner_symbols_idx++] = new_expression;
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
				Symbol *new_symbol =  generate_symbol_from(curr_token, 1, all_inner_symbols[inner_symbols_idx]);
				
				// parsing error
				if (!new_symbol)
				{
					EPRINTF("[parse_expression], invalid parenthesis, got %s\n", line);
					free(linedup);
					free_symbol_list(res);
					return 0;
				}

				res[res_idx] = new_symbol;
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
			Symbol *new_symbol =  generate_symbol_from(curr_token, 0, 0);
				
			// parsing error
			if (!new_symbol)
			{
				EPRINTF("[parse_expression], invalid parenthesis, got %s\n", line);
				free(linedup);
				free_symbol_list(res);
				return 0;
			}


			res[res_idx] = new_symbol;
			res_idx += 1;
		}

		curr_token = strtok(NULL, " ");
	}

	// check expressions are in order - no stray operators
	if (check_invalid_operators(res, line))
	{
		free(linedup);
		free_symbol_list(res);
		return 0;
	}
	
	free(linedup);
	return res;
}

int parse_facts_or_query(char *line, char *symbol_str_list)
{
	int i = 0;
	char *facts = strtok(line, "#");
	if (!is_upper_and_nl(facts + 1))
	{
		EPRINTF("[parse_facts_or_query] Must be uppercase, got %s\n", facts + 1);
		return 1;
	}
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

int parse_rule(char *line, Rulegraph *rule_graph)
{
	char *rule = strtok(line, "#");
	char lhs[128] = {0};
	char rhs[128] = {0};

	// check if rule has implication or iff
	char *resolver = 0;
	char *resolver_pos = 0;
	int assign_res = assign_resolver(rule, &resolver, &resolver_pos);
	if (assign_res)
		return 1;

	// clean the rule and split into 2 sides
	if (rule[strlen(rule) - 1] == '\n')
		rule[strlen(rule) - 1] = 0;
	strncpy(lhs, rule, resolver_pos - rule);
	strncpy(rhs, resolver_pos + strlen(resolver), (strlen(rule) - strlen(lhs) - strlen(resolver)));

	// get the expressions for both sides
	Symbol **lhs_symbols = parse_expression(lhs);
	Symbol **rhs_symbols = parse_expression(rhs);
	
	// paarsing error check
	if (!lhs_symbols || !rhs_symbols)
	{
		if (lhs_symbols)
			free_symbol_list(lhs_symbols);
		if (rhs_symbols)
			free_symbol_list(rhs_symbols);
		return 1;
	}

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
			free_symbol_list(lhs_symbols);
			free(lhs_symbols_str);
			free(rhs_symbols_str);
			EPRINTF("Resolve already found at rule %s\n", lhs_symbols_str);
			return 1;
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

		// free newly generated symbols
		free_symbol_list(lhs_symbols);
	}

	// printf("rule %s with %s @ %s\n", lhs, rhs, rule);
	free(lhs_symbols_str);
	free(rhs_symbols_str);
	return 0;
}

int parse_input_file(int fd, Rulegraph *rule_graph, char *query_list, char *facts_list) // take in array of rules
{
	for (char *line = get_next_line(fd); line != 0; free(line), line = get_next_line(fd))
	{
		// if line starts with # or blank line, continue
		if (line[0] == '#' || line[0] == '\n')
			continue;

		// if line starts with =, parse facts
		else if (line[0] == '=')
		{
			if (parse_facts_or_query(line, facts_list))
				return 1;
		}

		// if line starts with ?, parse query
		else if (line[0] == '?')
		{
			if (parse_facts_or_query(line, query_list))
				return 1;
		}

		// else, parse rule
		else
		{
			if (parse_rule(line, rule_graph))			
				return 1;
		}
	}
	return 0;
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

	if (parse_input_file(fd, rule_graph, query_list, facts_list))
	{
		free_rulegraph(rule_graph);
		free(facts_list);
		free(query_list);
		return 1;
	}

	// update the symbols here w/ facts here because we are cool and lazy
	update_rule_graph_with_facts(rule_graph, facts_list);
	print_banner();
	print_rulegraph(rule_graph);
	print_help();

	#ifdef __DEBUG__
        printf("__DEBUG__ DEFINED\n");
	#else
        printf("__DEBUG__ not defined\n");
    #endif

	// busy spin
	// while (1)
	// {
	// 	// prompt for fact change
	// 	write(1, "# ", 2);
	// 	char *prompt = get_next_line(0);
	// 	prompt[strlen(prompt) - 1] = 0;

	// 	// exit prompt
	// 	if (!strcmp(prompt, "exit"))
	// 	{
	// 		free(prompt);
	// 		break;
	// 	}

	// 	// clear screen
	// 	if (!strcmp(prompt, "clear"))
	// 	{
    // 		system("clear");
	// 		free(prompt);
	// 		continue;
	// 	}

	// 	// help
	// 	if (!strcmp(prompt, "help"))
	// 	{
    // 		print_help();
	// 		free(prompt);
	// 		continue;
	// 	}


	// 	// change fact
	// 	if (!strcmp(prompt, "fact"))
	// 	{
	// 		printf("\n");
	// 		while (1)
	// 		{
	// 			write(1, "fact# ", 6);
	// 			char *fact_prompt = get_next_line(0);
	// 			fact_prompt[strlen(fact_prompt) - 1] = 0;
	// 			if (strlen(fact_prompt) < ALPHA_COUNT && is_upper(fact_prompt))
	// 			{
	// 				memset(facts_list, 0, ALPHA_COUNT);
	// 				memcpy(facts_list, fact_prompt, strlen(fact_prompt));
	// 				update_rule_graph_with_facts(rule_graph, facts_list);
	// 				printf("fact updated %s\n", facts_list);
	// 				free(fact_prompt);
	// 				break;
	// 			}
	// 			else {
	// 				printf("Fact invalid \n");
	// 			}

	// 			free(fact_prompt);
	// 		}
	// 		free(prompt);
	// 		continue;
	// 	}

	// 	// change query
	// 	if (!strcmp(prompt, "query"))
	// 	{
	// 		printf("\n");
	// 		while (1)
	// 		{
	// 			write(1, "query# ", 7);
	// 			char *query_prompt = get_next_line(0);
	// 			query_prompt[strlen(query_prompt) - 1] = 0;
	// 			if (strlen(query_prompt) < ALPHA_COUNT && is_upper(query_prompt))
	// 			{
	// 				memset(query_list, 0, ALPHA_COUNT);
	// 				memcpy(query_list, query_prompt, strlen(query_prompt));
	// 				update_rule_graph_with_facts(rule_graph, query_list);
	// 				printf("query updated %s\n", query_list);
	// 				free(query_prompt);
	// 				break;
	// 			}
	// 			else {
	// 				printf("Query invalid \n");
	// 			}

	// 			free(query_prompt);
	// 		}
	// 		free(prompt);
	// 		continue;
	// 	}

	// 	// ls
	// 	if (!strcmp(prompt, "ls"))
	// 	{
	// 		print_rulegraph(rule_graph);
	// 		printf("\nQuery: %s\n", query_list);
	// 		printf("Facts: %s\n", facts_list);
	// 		free(prompt);
	// 		continue;
	// 	}

	// 	// runs query
	// 	if (!strcmp(prompt, "run"))
	// 	{
	// 		resolve_query(rule_graph, facts_list, query_list);
	// 		free(prompt);
	// 		continue;
	// 	}

	// 	// any other command
	// 	printf("Command %s not found. type 'help' to list commands\n", prompt);

	// 	free(prompt);
	// }

	free_rulegraph(rule_graph);
	free(facts_list);
	free(query_list);
	printf("OK\n");
	return 0;
}