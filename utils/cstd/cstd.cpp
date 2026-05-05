#include "cstd.hpp"

void *ft_memcpy(void *dest, const void *src, size_t n) {
	t_uc	*tmp1;
	t_uc	*tmp2;

	if (!n)
		return (dest);
	if (!src || !dest)
		return (NULL);
	tmp1 = (t_uc *)dest;
	tmp2 = (t_uc *)src;
	for (size_t i = 0; i < n; ++i) {
		tmp1[i] = tmp2[i];
	}
	return (dest);
}

void	*ft_memmove(void *dest, const void *src, size_t n)
{
	unsigned char	*ptr1;
	unsigned char	*ptr2;

	if (!n)
		return (dest);
	if (!src || !dest)
		return (NULL);
	ptr1 = (unsigned char *)dest;
	ptr2 = (unsigned char *)src;
	if (ptr1 < ptr2)
		while (n--)
			*ptr1++ = *ptr2++;
	else
	{
		ptr1 = ptr1 + (n - 1);
		ptr2 = ptr2 + (n - 1);
		while (n--)
			*ptr1-- = *ptr2--;
	}
	return (dest);
}

void	*ft_memset(void *s, int c, size_t n)
{
	size_t	i;
	t_uc	*tmp;

	tmp = (t_uc *)s;
	i = 0;
	while (i < n)
		tmp[i++] = c;
	return (s);
}

void	*ft_strcat(void *dest, const void *src)
{
	size_t	d_len;
	size_t	i;
	t_uc	*_dest = (t_uc *)dest;
	t_uc	*_src = (t_uc *)src;

	d_len = ft_strlen(dest);
	i = -1;
	while (_src[++i])
		_dest[d_len + i] = _src[i];
	_dest[d_len + i] = '\0';
	return (dest);
}

size_t	ft_strlen(const void *_s)
{
	size_t	i;
	t_uc	*s = (t_uc *)_s;

	i = 0;
	while (s[i])
		i++;
	return (i);
}

int	ft_strcmp(const void *_s1, const void *_s2)
{
	size_t	i;
	t_uc	*s1 = (t_uc *)_s1;
	t_uc	*s2 = (t_uc *)_s2;

	i = -1;
	while (s1[++i] && s2[i] && s1[i] == s2[i])
		;
	return (s1[i] - s2[i]);
}

char	*ft_strcpy(char *dest, const char *src)
{
	size_t	i;

	i = -1;
	while (src[++i] != '\0')
		dest[i] = src[i];
	dest[i] = '\0';
	return (dest);
}

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;
	t_uc	*s_1;
	t_uc	*s_2;

	i = 0;
	s_1 = (t_uc *)s1;
	s_2 = (t_uc *)s2;
	if (!n)
		return (0);
	while (s1[i] && s_1[i] == s_2[i] && i < n - 1)
		i++;
	return ((s_1[i] - s_2[i]));
}

char	*ft_strncpy(char *dest, const char *src, unsigned int n)
{
	unsigned int i;

	i = 0;
	while (i < n && src[i] != '\0') {
		dest[i] = src[i];
		++i;
	}
	while (i < n) {
		dest[i++] = '\0';
	}
	return (dest);
}

char	*ft_strrchr(const char *s, int c)
{
	size_t len;

	len = ft_strlen(s);
	for (size_t i = len + 1; i > 0; ) {
		--i;
		if (s[i] == (char)c)
			return ((char *)(s + i));
		if (i == 0)
			break;
	}
	return (NULL);
}

