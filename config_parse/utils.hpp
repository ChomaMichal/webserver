#pragma once
#include <fstream>
void getline_stripspace(std::ifstream &infile, std::string &line);
void getline_stripspace(std::ifstream &infile, std::string &line,
                        std::string delim);
void get_redirection_pair(std::string &line, std::pair<int, std::string> &pair);
