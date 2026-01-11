#pragma once
#include <string>
#include <cstddef>
#include <iostream>

class StrSlice {
	private:
		char	*str;
		size_t	len;
		StrSlice();
	
	public:
		//doesn't check for valid len of slice
		StrSlice(char *start, size_t len);
		//creates slice form until it find character until 
		//when character isn't found len == 0
		StrSlice(char *from, char until);
		StrSlice(StrSlice &other);
		~StrSlice();
		StrSlice	&operator=(StrSlice &other);
		//checks if both strings contain the same characters
		bool		operator==(StrSlice &other);
		//checks for index bound returns 0 in case of out of bound
		char		operator[](size_t position) const;
		//doesn't check for index bound
		char		at(size_t position) const;
		size_t		getLen(void) const;

};

std::iostream &operator<<(std::iostream &os, StrSlice &str);
