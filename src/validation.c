#include "ft_macros.h"
#include "rules.h"

int is_upper(char *str)
{
	for (int i = 0; str[i]; i++)
	{
		if (str[i] < 'A' || str[i] > 'Z')
			return 0;
	}
	
	return 1;
}

int is_upper_and_nl(char *str)
{
	for (int i = 0; str[i]; i++)
	{
		if (i == (strlen(str) - 1))
		{
			if (str[i] == '\n')
				break;
		}

		if (str[i] < 'A' || str[i] > 'Z')
			return 0;
	}
	
	return 1;
}

int assign_resolver(char* rule, char **resolver, char **resolver_pos)
{
	if (*resolver_pos = strstr(rule, IMPL_RESOLVER))
		*resolver = IMPL_RESOLVER;
	else if (*resolver_pos = strstr(rule, IFF_RESOLVER))
		*resolver = IFF_RESOLVER;

	// check no resolver
	if (!*resolver_pos)
	{
		EPRINTF("[assign_resolver] No resolve found at rule %s\n", rule);
		return 1;
	}

	// check for duplicate resolver
	if (strstr(*resolver_pos + strlen(*resolver), IMPL_RESOLVER) || strstr(*resolver_pos + strlen(*resolver), IFF_RESOLVER))
	{
		char *test = strstr(*resolver_pos, IMPL_RESOLVER);
		char *test2 = strstr(*resolver_pos, IFF_RESOLVER);

		EPRINTF("[assign_resolver] Multiple resolve found at rule %s or %s\n", test, test2);
		return 1;
	}

	return 0;

}

int check_invalid_operators(Symbol **res, char *line)
{
	int i = -1;
	while (res[++i])
	{
		if (i == 0 || res[i + 1] == 0)
		{
			if (res[i]->type == OPERATOR)
			{
				EPRINTF("[check_invalid_operators], first or last operator placement, got %s\n", line);
				return -1;
			}
		}
		Symbol *curr = res[i];
		if (curr->type == INNER)
		{
			if (check_invalid_operators(curr->inner_symbols, line) == -1)
				return -1;
		}
		if ((curr->type == OPERATOR && res[i-1]->type == OPERATOR) || (curr->type == OPERATOR && res[i+1]->type == OPERATOR) )
		{
			EPRINTF("[check_invalid_operators], back to back operator placement, got %s\n", line);
			return -1;
		}
	}
	return 0;
}