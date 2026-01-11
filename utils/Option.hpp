#pragma once

#include <stdexcept>
#include <string>

template <typename T>

class Option{
	protected:
		T		value;
		bool	none;
		bool	checked;

	private:
		Option(){}
		Option(Option &other){}
		Option& operator=(Option &other){}

	public:
		Option(T &value): value(value), none(false), checked(false){}
		Option(bool none): none(true), checked(false){}
		T	safe_unwrap(void){
			if (checked == false) throw std::invalid_argument("Tried to safe_unwrap without checking for none");

			if	(none == true) throw std::runtime_error("Unwraped option that contanied NONE");
			else return value;
		}
		T	unwrap(void){
			if (checked == true && none == false) return this->value;

			if	(none == true) throw std::runtime_error("Unwraped option that contanied NONE");
			else return value;
		}
		bool	check(void){
			this->checked = true;
			return (this->none);
		}
		T	unwrap_or(T &other){
			if (checked == true && none == false) return this->value;

			if	(none == true) return (other);
			else return value;
		}
		T	expect(const std::string &error_mesage){
			if (checked == true && none == false) return this->value;

			if	(none == true) throw std::runtime_error(error_mesage);
			else return value;
		}	
		~Option(){}
};
