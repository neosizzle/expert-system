# expert-system

A simple expert system with an inference engine that executes propositional logic using backward chaining. Come with modifyable knowledge bases.

## Setup
```
make && ./expertsystem  <input file with knowledge base> 
```

Example:
```
make && ./expertsystem  example-input1.txt 
```

Replace `make` with `make debug` for more verbose debug logs

## Algorithm
Each symbol e.g. `A`, `B`, `!A`, `+`, `|`, `( A + B | C ^ D)` will be represented in the following structure

```
typedef struct Symbol
{
	SymbolType type; // Variable, fact, operator or inner?
	Operator operator;	// only relevant when type is operator, +, ^, | 
	struct Symbol** inner_symbols; // only relevant when type is inner, stores symbols in parenthesis
	int is_negated; // is the symbol prefixed with '!'?
	char* str_repr; // the string representation of the symbol
} Symbol;
```

Symbols being formed together like `A + B => C` represents a Rule, which is represented by the following structure

```
typedef struct Rule
{
	Symbol** symbol_list; // list of symbol structs in this rule
	ResolveType resolve_type; // is the rule being resolved with => instead of <=>?
	struct Rule* implies; // only relavant if rule have =>, points to RHS of rule
	struct Rule* iff; // only relevant of rule have <=>, points to RHS of rule
} Rule;
```

A bunch of rules combined together, we can represent them as a directed graph for easy query
```
typedef struct Rulegraph 
{
	Rule** all_rules_vertices;
	int vertex_count;
} Rulegraph;
```
The algorithm to resolve a symbol goes like this:
1. Check if a symbol is a fact. If it is, return **true** if non negated and **false** if negated
2. There are some rules and symbols we have to ignore, we need to return an ambigious result for this one (explained later). Return **true, false**
3. If the symbol we are trying to look for is found in the cache, return that value from cache
4. Locate all the rules where this symbol exists as the result
5. Iterate all the found rules
	- Add the current rule to the ignore list, just incase somehow there is a circular dependency, we will return an ambigious result like in step 2
	- Resolve that rule by iterating through all its symbols, resolving each one of the, run the boolean logic and obtain either **true**, **false** or **true, false**
	- Iterate all possible results for the said rule
	- Iterate through all symbols of RHS of said rule to get their values based on knowledge base
	- Generate a truth table based on the RHS of the current rule 
	- Filter the truth table so it contains the current iterated result in 5a. and contains the symbol values at 5d (only for the symbols that we are NOT trying to query right now)
	- Find the possible results of the symbol that we are querying in the filtered truth table and do a cache lookup
	- If the cache does not contain the symbol or contain an ambigious result for the symbol, overwrite the cache
6. Return the query symbol from cache

## Demo