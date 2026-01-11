#include "StrSlice.hpp"

StrSlice::StrSlice(){}

StrSlice::StrSlice(char *start, size_t len): str(start), len(len){}

StrSlice::StrSlice(char *from, char until): str(from){
	size_t	i = 0;

	while(str[i]){
		if (str[i] == until){
			this->len = i;
			return ;
		}
		i++;
	}
	this->len = 0;
}

StrSlice &StrSlice::operator=(StrSlice &other){
	this->str = other.str;
	this->len = other.len;
	return(*this);
}

bool	StrSlice::operator==(StrSlice &other){
	if (other.len != this->len)
		return (false);
	for (size_t i = 0; i < this->len; i ++){
		if (this->at(i) != other.at(i))
			return (false);
	}
	return (true);
}

char	StrSlice::operator[](size_t position) const{
	if (position < len)
		return (str[position]);
	return (0);
}

char	StrSlice::at(size_t position) const{
	return str[position];
}

size_t	StrSlice::getLen(void)const{return (this->len);}

std::iostream &operator<<(std::iostream &os, StrSlice &str){
	for (size_t i = 0; i < str.getLen(); i ++)
		os << str[i];
	return (os);
}
