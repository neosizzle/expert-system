#include "ft_macros.h"
#include "rules.h"
#include "get_next_line.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#define  ALPHA_COUNT  26

int find_matching_rp(char *line) {
	int res;
	int parent_stack;

	parent_stack = 0;
	res = -1;
	while (line[++res]) {
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

Symbol** parse_expression(char *line) {
	Symbol** all_inner_symbols[8]; // shh.. dont tell bocal i hard allocate here
	Symbol** res = (Symbol**)calloc(16, sizeof(Symbol*)); // shh.. dont tell bocal i hard allocate here also

	int inner_symbols_idx = 0;

	// do we find any '(' in the current line?
	int i = strcspn(line, "(");

	// we have '(', so we need to process the things inside of the '(' first
	while (i < strlen(line)) {
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
			if (pstack == 0) {
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
		if (pstack == 0) {
			res[res_idx] = generate_symbol_from(curr_token, 0, 0);
			res_idx += 1;
		}

		curr_token = strtok(NULL, " ");
	}
	free(linedup);
	return res;
}

int parse_facts_or_query(char* line, char* symbol_str_list) {
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
			EPRINTF("Facts are too much, more than %d\n", ALPHA_COUNT)
			return 1;
		}
	}

	printf("fact list is now %s\n", symbol_str_list);
	return 0;
}

void parse_rule(char* line, Rulegraph* rule_graph) {
	char *rule = strtok(line, "#");
	char lhs[128] = {0};
	char rhs[128] = {0};

	// check if rule has implication or iff
	char *resolver = 0;
	char *resolver_pos = 0;
	if (resolver_pos = strstr(rule, " => "))
		resolver = " => ";
	else if (resolver_pos = strstr(rule, " <=> "))
		resolver = " <=> ";
	else
	{
		EPRINTF("No resolve found at rule %s\n", rule);
		return 1;
	}	

	// clean the rule and split into 2 sides
	if (rule[strlen(rule) - 1] == '\n')
		rule[strlen(rule) - 1] = 0;
	strncpy(lhs, rule, resolver_pos - rule);
	strncpy(rhs, resolver_pos + strlen(resolver), (strlen(rule) - strlen(lhs) - strlen(resolver)));
	
	// get the expressions for both sides
	// TODO for rhs, we search previous rules for the same symbols
	// Symbol **lhs_symbols = parse_expression(lhs);
	// Symbol **rhs_symbols = parse_expression(rhs);

	// TODO construct / fetch rule struct and add to rule graph

	printf("rule %s with %s (%s)\n", lhs, rhs, rule);
	// printf("resolver (%s)\n", resolver);

}

void parse_input_file(int fd, Rulegraph* rule_graph, char* query_list) // take in array of rules
{
	char *facts_list = calloc(ALPHA_COUNT, 1);
	Rule* rules = (Rule*)malloc(sizeof(Rule) * 32); // shh... dont tell evaluator that I cap the rules size

	for (char* line = get_next_line(fd); line != 0; free(line), line = get_next_line(fd))
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

		// free(line);
		// line = get_next_line(fd);
	}

	// TODO construct rule graph

	free(facts_list);
}

void test() {
	printf("From test\n");
	char* line = "A + (B + (C + D) + E + (F + G) + (F2 + G2 + (F2))) + H";
	// char* line = "A + ((B + C) + (D + E)) + (G + F)";
	// char *line = "(!A + B) + C";
	// char* line = "()";
	Symbol** symbol_list = parse_expression(line);
	char *symbol_str = serialize_symbols(symbol_list);
	printf("%s\n", symbol_str);
	free(symbol_str);

	// Symbol* s1 = generate_symbol_from("A", 0, 0);
	// Symbol* s2 = generate_symbol_from("+", 0, 0);
	// Symbol* s3 = generate_symbol_from("!B", 0, 0);

	// free_symbol(s1);

	// Symbol** symbol_list = (Symbol**)calloc(16, sizeof(Symbol*));
	// symbol_list[0] = s1;
	// symbol_list[1] = s2;
	// symbol_list[2] = s3;

	free_symbol_list(symbol_list);
	exit(0);
}

int main(int argc, char *argv[])
{
	if (argc != 2) {
		EPRINTF("Usage: %s [input_file]\n", argv[0]);
		return 1;
	}
	
	// open file
	char *input_filename = argv[1];
	int fd = open(input_filename, O_RDONLY);
	if (fd < 0) {
		DIE_OS(fd, "Failed to open file %s\n", input_filename);
	}

	// start parsing and get list of rules, query and facts
	Rulegraph *rule_graph = (Rulegraph *)malloc(sizeof(Rulegraph)); 
	char *query_list = calloc(ALPHA_COUNT, 1);
	parse_input_file(fd, rule_graph, query_list);

	// TODO update the symbols here w/ facts here because we are cool and lazy

	// TODO run expert system with said rules and query

	// busy spin
	while (1)
	{
		// prompt for fact change
		char *prompt = get_next_line(0);
		printf("prompt: %s\n", prompt);

		// exit prompt
		if (!strcmp(prompt, "exit\n"))
		{
			free(prompt);
			break;
		}

		// apply changes and run expert system again
		free(prompt);
	}
	

	free(rule_graph);
	free(query_list);
	printf("OK\n");
	return 0;
}