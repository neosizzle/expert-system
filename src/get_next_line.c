# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

#define BUFFER_SIZE 1024

//helpers
//calculates and gets the string length
size_t	ft_strlen(const char *s)
{
	size_t	res;

	res = 0;
	while (s[res])
		res++;
	return (res);
}

//joins s1 and s2 together
//allocates new memory
char	*ft_strcat(char *s1, char *s2)
{
	char	*res;
	size_t	i;
	size_t	j;

	res = (char *)malloc(sizeof(char) *(ft_strlen(s1) + ft_strlen(s2) + 1));
	if (!res)
		return (0);
	i = 0;
	j = 0;
	while (s1[i])
	{
		res[i + j] = s1[i];
		i++;
	}
	while (s2[j])
	{
		res[i + j] = s2[j];
		j++;
	}
	res[i + j] = '\0';
	return (res);
}

//finds a chr in a string, returns pointer to that char if found
char	*ft_strchr(const char *s, int c)
{
	size_t	i;
	size_t	len;

	i = 0;
	len = ft_strlen(s);
	while (i < len && s[i])
	{
		if (s[i] == c)
			return ((char *)(s + i));
		s++;
	}
	return (0);
}

//allocates mem and generates a substring based on the 
//satrting point and length of the og string
char	*ft_substr(const char *str, size_t start, size_t len)
{
	char	*res;
	size_t	i;

	if (!str)
		return (0);
	res = (char *)malloc(sizeof(char) * (len + 1));
	if (!res)
		return (0);
	i = 0;
	while (start < len)
		res[i++] = str[start++];
	res[i] = 0;
	return (res);
}

//allocates mem with size and fills it with zero
char	*ft_bzero(size_t size)
{
	char	*res;
	size_t	i;

	res = (char *)malloc(sizeof(char) * (size + 1));
	if (!res)
		return (0);
	i = 0;
	while (i < size + 1)
	{
		*(char *)(res + i) = 0;
		i++;
	}
	return (res);
}

//function to free a pointer and points it to null byte
static void	ft_freestr(char **str)
{
	if (str)
	{
		free(*str);
		*str = 0;
	}
}

//helpr func to allocate mem and duplicate a string
static char	*ft_strdup(const char *str)
{
	size_t	i;
	char	*res;

	i = 0;
	res = (char *) malloc(sizeof(char) * (ft_strlen(str) + 1));
	if (!res)
		return (0);
	while (str[i])
	{
		res[i] = str[i];
		i++;
	}
	res[i] = 0;
	return (res);
}

//this function reads from a fd
//sets the number of bytes read to a pointer
//return the number of bytes read
static int	read_buff(int fd, char **buff, int *bytes_read)
{
	int	res;

	res = read(fd, *buff, BUFFER_SIZE);
	*bytes_read = res;
	return (res);
}

//function to extract a string containing newline
//it will allocate the memory for the result
//the result will contain the string that ends with \n
//if its end of file, it will contain a null termed str
//besides trimming the result, the remainder of that trim
//will get stored in str for next call
//e.g 
//12345\n2234
//res = 12345\n
//str = 2234
static char	*extract_line(char **str)
{
	size_t	i;
	char	*res;
	char	*temp;

	i = 0;
	while ((*str)[i] && (*str)[i] != '\n')
		i++;
	if ((*str)[i])
	{
		res = ft_substr(*str, 0, i + 1);
		temp = ft_strdup(*str + i + 1);
		ft_freestr(str);
		if (temp[0] != '\0')
			*str = temp;
		else
			ft_freestr(&temp);
	}
	else
	{
		res = ft_strdup(*str);
		ft_freestr(str);
	}
	return (res);
}

//gnl main func
//the general idea is to:
//1. read the fd for buff size with while its not EOF
//2. check the buff size if got any new lines
//	if yes, stop reading and trim the buffer and return
//	if no, continue reading 
//3. return null for err handling or EOF reached
char	*get_next_line(int fd)
{
	static char	*res;
	char		*buff;
	char		*temp;
	int			bytes_read;

	if (fd < 0 || fd > 1024 || BUFFER_SIZE < 1)
		return (0);
	buff = (char *) malloc(sizeof(char) * (BUFFER_SIZE + 1));
	if (!buff)
		return (0);
	while (read_buff(fd, &buff, &bytes_read) > 0)
	{
		buff[bytes_read] = 0;
		if (!res)
			res = ft_bzero(0);
		temp = ft_strcat(res, buff);
		ft_freestr(&res);
		res = temp;
		if (ft_strchr(buff, '\n'))
			break ;
	}
	ft_freestr(&buff);
	if (bytes_read < 0 || (bytes_read == 0 && !res))
		return (NULL);
	return (extract_line(&res));
}