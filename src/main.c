#include "ft_macros.h"
#include "rules.h"
#include "get_next_line.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#define  ALPHA_COUNT  26

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
}

void parse_input_file(int fd, Rulegraph* rule_graph, char* query_list) // take in array of rules
{
	char *facts_list = calloc(ALPHA_COUNT, 1);
	Rule* rules = (Rule*)malloc(sizeof(Rule) * 1024); // shh... dont tell evaluator that I cap the rules size

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
	free(facts_list);
}

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

void parse_line(char *line) {
	Symbol **all_inner_symbols = (Symbol **)malloc(sizeof(Symbol*) * 32); // shh.. dont tell bocal i hard allocate here

	// do we find any '(' in the current line?
	int i = strcspn(line, "(");
	if (i == strlen(line)) {
		// if we dont, means we wont have any inner symbols. free the inner symbols list
		free(all_inner_symbols);
		all_inner_symbols = NULL;
	}

	// we have '(', so we need to process the things inside of the '(' first
	while (i < strlen(line)) {
		// i will always point to the '(', so we will find the matching ')' here.
		int matching_rp = find_matching_rp(line + i + 1); // offset + 0 index?

		// extract the subscring and recurse to get the symbols inside the parenthesis
		char *inner = strndup(line + i + 1, matching_rp);
		parse_line(inner);
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
	while (curr_token)
	{
		if (strcspn(curr_token, "(") < strlen(curr_token))
		{
			// first '(' found, find symbol list to replace
			if (pstack == 0) {
				printf("gotta blast\n");
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

		if (pstack == 0) {
			printf("currtoken final %s\n", curr_token);
			// 
		}

		curr_token = strtok(NULL, " ");
	}
	printf("==========================\n");
	free(linedup);
}

void test() {
	printf("From test\n");
	char* line = "A + (B + (C + D) + E + (F + G) + (F2 + G2 + (F2))) + H";
	// char* line = "A + ((B + C) + (D + E)) + (G + F)";
	// char *line = "(A + B) + C";
	parse_line(line);
	exit(0);
}

int main(int argc, char *argv[])
{
	test();
	if (argc != 2) {
		EPRINTF("Usage: %s [input_file]\n", argv[0]);
		return 1;
	}
	
	// open file
	char *input_filename = argv[1];
	int fd = open(input_filename, O_RDONLY);
	if (fd < 0) {
		DIE(fd, "Failed to open file %s\n", input_filename);
	}

	// start parsing and get list of rules, and the query
	Rulegraph *rule_graph = (Rulegraph *)malloc(sizeof(Rulegraph)); 
	char *query_list = calloc(ALPHA_COUNT, 1);
	parse_input_file(fd, rule_graph, query_list);

	// run expert system with said rules and query

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