#ifndef __RULES__H__
#define __RULES__H__

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

typedef enum Result
{
	RESULT_INIT,
	TRUE,
	FALSE,
	UNDEFINED
} Result;

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
	struct Symbol** inner_symbols;
	int is_negated;
	char* str_repr;
	Result result;
} Symbol;

typedef struct Rule
{
	Symbol** symbol_list;
	Result confirmed_result;
	// Result *possible_results;
	int ref_cnt; // used for freeing
	ResolveType resolve_type;
	struct Rule* implies;
	struct Rule* iff;
} Rule;

typedef struct Rulegraph 
{
	Rule** all_rules_vertices;
	int vertex_count;
} Rulegraph;

Symbol *generate_symbol_from(char* str, int is_inner, Symbol** inner_symbols);
void free_symbol(Symbol *symbol);
void free_symbol_list(Symbol **symbols);
char *serialize_symbols(Symbol** symbols);

Rule* search_for_rule(Rulegraph* rg, char* input_symbols, int is_lhs);
void free_rulegraph(Rulegraph *rg);
Rulegraph *generate_default_rulegraph();
Rule *generate_rule_from(Symbol** symbol_list);

#endif  //!__RULES__H__