#ifndef __RULES__H__
#define __RULES__H__

#define  MAX_VERTEX_COUNT  32

typedef enum SymbolType
{
	VARIABLE,
	FACT,
	OPERATOR,
	INNER,
} SymbolType;

typedef enum Operator
{
	AND,
	OR,
	XOR,
	NOP,
} Operator;

typedef enum ResolveType
{
	IMPLIES,
	IFF,
	NO_RESOLVE
} ResolveType;

typedef struct Symbol
{
	SymbolType type;
	Operator operator;
	struct Symbol **inner_symbols;
	int is_negated;
	char *str_repr;
} Symbol;

typedef struct Rule
{
	Symbol **symbol_list;
	ResolveType resolve_type;
	struct Rule *implies;
	struct Rule *iff;
} Rule;

typedef struct Rulegraph
{
	Rule **all_rules_vertices;
	int vertex_count;
} Rulegraph;

Symbol *generate_symbol_from(char *str, int is_inner, Symbol **inner_symbols);
void free_symbol(Symbol *symbol);
void free_symbol_list(Symbol **symbols);
void free_rule_list(Rule **rules);
char *serialize_symbols(Symbol **symbols);

Rule *search_for_rule(Rulegraph *rg, char *input_symbols, int is_lhs);
void free_rulegraph(Rulegraph *rg);
Rulegraph *generate_default_rulegraph();
Rule *generate_rule_from(Symbol **symbol_list);

int update_rule_graph_with_facts(Rulegraph *rule_graph, char *facts);

void print_rulegraph(Rulegraph *rule_graph);
void print_adjacency_list(Rulegraph *rule_graph);

#endif //!__RULES__H__