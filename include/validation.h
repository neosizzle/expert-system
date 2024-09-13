#ifndef __VALIDATION__H__
#define __VALIDATION__H__

// validate query and facts
int is_upper(char *str);
int is_upper_and_nl(char *str);

// validate rules
int assign_resolver(char* rule, char **resolver, char **resolver_pos);
int check_invalid_operators(Symbol **res, char *line);

#endif  //!__VALIDATION__H__