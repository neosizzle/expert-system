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



void parse_line(char *line) {
	printf("line is %s\n", line);
	int found_lp = strcspn(line, "(");
	printf("found_lp @ %d\n", found_lp);
	if (found_lp < strlen(line)) {
		parse_line(line + found_lp + 1);
		// int end_border = strcspn(line, ")");
		// char *local_context = strndup(line, end_border);
		// printf("local context is %s\n", local_context);
		// return;
	}
	int end_border = strcspn(line, ")");
	char *local_context = strndup(line, end_border);
	printf("local context is %s\n", local_context);
}

void test() {
	printf("From test\n");
	char* line = "A + (B + (C + D) + E) + F";
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