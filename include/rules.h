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
} ResolveType;

typedef struct Symbol
{
	SymbolType type;
	Operator operator;
	struct Symbol* inner_symbols;
	int is_negated;
	int (*to_string)(struct Symbol symbol);
	Result result;
} Symbol;

typedef struct Rule
{
	Symbol* symbol_list;
	Result confirmed_result;
	Result *possible_results;
	ResolveType resolve_type;
	struct Rule* implies;
	struct Rule* iff;
} Rule;

typedef struct Rulegraph 
{
	Rule* all_rules_vertices;
	int vertex_count;
} Rulegraph;

#endif  //!__RULES__H__