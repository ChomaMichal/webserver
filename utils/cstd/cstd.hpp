#pragma once

typedef unsigned char t_uc;

#include <cstdio>

void	*ft_memcpy(void *dest, const void *src, size_t n);
void	*ft_memmove(void *dest, const void *src, size_t n);
void	*ft_memset(void *s, int c, size_t n);
void	*ft_strcat(void *dest, const void *src);
size_t	ft_strlen(const void *s);
int		ft_strcmp(const void *_s1, const void *_s2);
char	*ft_strcpy(char *dest, const char *src);
int		ft_strncmp(const char *s1, const char *s2, size_t n);
char	*ft_strncpy(char *dest, const char *src, unsigned int n);
char	*ft_strrchr(const char *s, int c);
